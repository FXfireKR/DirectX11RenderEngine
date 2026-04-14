#pragma once

#include <DirectXMath.h>
#include "ChunkTypes.h"

struct BlockHitResult
{
	bool bHit = false;

	DirectX::XMINT3 block{};	// 맞은 블럭 좌표
	DirectX::XMINT3 prev{};		// 바로 직전 좌표
	DirectX::XMINT3 normal{};	// 맞은 면 노멀

	float t = 0.f;		// ray param / 대략적 거리
	BlockCell cell{};	// 맞은 셀 정보
};