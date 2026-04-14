#pragma once

#include <cmath>
#include <limits>
#include <DirectXMath.h>

#include "BlockRaycastTypes.h"
#include "IBlockAccessor.hpp"

namespace BlockRaycastUtil
{
	static float SafeInv(float v)
	{
		const float eps = 1e-8f; // custom epsilon

		if (std::fabs(v) < eps) 
			return std::numeric_limits<float>::infinity();

		return 1.0f / v;
	}

	inline float NextBoundary(int cell, int step)
	{
		if (step > 0) return static_cast<float>(cell + 1);
		if (step < 0) return static_cast<float>(cell);
		return std::numeric_limits<float>::infinity();
	}

	inline bool RaycastVoxelDDA(const IBlockAccessor& world
		, const DirectX::XMFLOAT3& origin
		, const DirectX::XMFLOAT3& dirNorm
		, float maxDist
		, const BlockRaycastOptions& options
		, BlockHitResult& outResult)
	{
		constexpr float INF = std::numeric_limits<float>::infinity();

		outResult = {};

		DirectX::XMINT3 coord =
		{
			static_cast<int>(std::floor(origin.x)),
			static_cast<int>(std::floor(origin.y)),
			static_cast<int>(std::floor(origin.z))
		};

		const DirectX::XMINT3 step =
		{
			(dirNorm.x > 0.f) ? 1 : ((dirNorm.x < 0.f) ? -1 : 0),
			(dirNorm.y > 0.f) ? 1 : ((dirNorm.y < 0.f) ? -1 : 0),
			(dirNorm.z > 0.f) ? 1 : ((dirNorm.z < 0.f) ? -1 : 0)
		};

		const DirectX::XMFLOAT3 invDiv =
		{
			SafeInv(dirNorm.x),
			SafeInv(dirNorm.y),
			SafeInv(dirNorm.z)
		};

		DirectX::XMFLOAT3 tMax =
		{
			(step.x == 0) ? INF : (NextBoundary(coord.x, step.x) - origin.x) * invDiv.x,
			(step.y == 0) ? INF : (NextBoundary(coord.y, step.y) - origin.y) * invDiv.y,
			(step.z == 0) ? INF : (NextBoundary(coord.z, step.z) - origin.z) * invDiv.z
		};

		DirectX::XMFLOAT3 tDelta =
		{
			(step.x == 0) ? INF : std::fabs(static_cast<float>(step.x) * invDiv.x),
			(step.y == 0) ? INF : std::fabs(static_cast<float>(step.y) * invDiv.y),
			(step.z == 0) ? INF : std::fabs(static_cast<float>(step.z) * invDiv.z)
		};

		DirectX::XMINT3 prevCell = coord;

		// 시작 셀 검사
		{
			const BlockCell startCell = world.GetBlock(coord.x, coord.y, coord.z);
			if (true == world.CanRaycastHit(startCell))
			{
				outResult.bHit = true;
				outResult.block = coord;
				outResult.prev = coord;
				outResult.normal = { 0, 0, 0 };
				outResult.t = 0.f;
				outResult.cell = startCell;

				return true;
			}
		}

		float t = 0.f;

		while (t <= maxDist)
		{
			prevCell = coord;

			if (tMax.x <= tMax.y && tMax.x <= tMax.z)
			{
				coord.x += step.x;
				t = tMax.x;
				tMax.x += tDelta.x;
				outResult.normal = { -step.x, 0, 0 };
			}
			else if (tMax.y <= tMax.z)
			{
				coord.y += step.y;
				t = tMax.y;
				tMax.y += tDelta.y;
				outResult.normal = { 0, -step.y, 0 };
			}
			else
			{
				coord.z += step.z;
				t = tMax.z;
				tMax.z += tDelta.z;
				outResult.normal = { 0, 0, -step.z };
			}

			if (t > maxDist) break;

			const BlockCell cell = world.GetBlock(coord.x, coord.y, coord.z);
			if (true == world.CanRaycastHit(cell))
			{
				outResult.bHit = true;
				outResult.block = coord;
				outResult.prev = prevCell;
				outResult.t = 0.f;
				outResult.cell = cell;

				return true;
			}
		}

		outResult.bHit = false;
		return false;
	}
}