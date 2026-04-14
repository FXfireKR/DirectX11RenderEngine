#pragma once
#include "VoxelTypes.h"

#include <string>
#include <vector>
#include <optional>

enum class BLOCK_RENDER_LAYER : uint8_t
{
	OPAQUE_LAYER = 0,
	CUTOUT_LAYER,
	TRANSLUCENT_LAYER,
	INVISIBLE_LAYER,
};

enum class BLOCK_COLLISION_TYPE : uint8_t
{
	NONE = 0,
	FULL_CUBE,
	SLAB,
	CUSTOM,
};

struct BlockPropertyFlags
{
	bool bAir = false;
	bool bOpaque = false;
	bool bSolid = false;
	bool bFullCube = false;
};

struct BlockRenderDef
{
	BLOCK_RENDER_LAYER layer = BLOCK_RENDER_LAYER::OPAQUE_LAYER;
	bool bAmbientOcclusion = true;
	bool bCullSameBlockFace = false;
	uint8_t lightEmissions = 0;
};

struct BlockCollisionDef
{
	BLOCK_COLLISION_TYPE colType = BLOCK_COLLISION_TYPE::NONE;
};

struct BlockGameplayDef
{
	float fHardness = 0.f;
	float fBlastResistance = 0.f;

	bool bRequiresCorrectTool = false;
	std::string requiredToolTag;
	int requiredToolTier = 0;
};

struct BlockSoundDef
{
	std::string profile;
};

struct BlockDef
{
	BLOCK_ID blockID = INVALID_BLOCK_ID;

	std::string name;
	std::string stateSource;

	BlockPropertyFlags properties;
	BlockRenderDef render;
	BlockCollisionDef collision;

	BlockGameplayDef gameplay;
	BlockSoundDef sound;

	bool bValid = false;
};

struct BlockPropertyFlagsRaw
{
	std::optional<bool> air;
	std::optional<bool> opaque;
	std::optional<bool> solid;
	std::optional<bool> fullCube;
};

struct BlockRenderDefRaw
{
	std::optional<BLOCK_RENDER_LAYER> layer;
	std::optional<bool> ambientOcclusion;
	std::optional<bool> cullSameBlockFace;
	std::optional<int> lightEmissions;
};

struct BlockCollisionDefRaw
{
	std::optional<BLOCK_COLLISION_TYPE> colType;
};

struct BlockGameplayDefRaw
{
	std::optional<float> hardness;
	std::optional<float> blastResistance;

	std::optional<bool> bRequiresCorrectTool;
	std::optional<std::string> requiredToolTag;
	std::optional<int> requiredToolTier;
};

struct BlockSoundDefRaw
{
	std::optional<std::string> profile;
};

struct BlockDefRaw
{
	std::string name;
	std::string parent;
	std::optional<std::string> stateSource;

	BlockPropertyFlagsRaw properties;
	BlockRenderDefRaw render;
	BlockCollisionDefRaw collision;
	BlockGameplayDefRaw gameplay;
	BlockSoundDefRaw sound;

	bool bIsTemplate = false;
	bool bLoaded = false;
};

