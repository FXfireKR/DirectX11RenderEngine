#pragma once

/*
*	[ Structure ]

	Util
	├─ Block
	│  ├─ BlockAccessor.hpp
	│  ├─ BlockRaycastTypes.h
	│  ├─ BlockRaycastUtil.h
	│  ├─ BlockStateTypes.h
	│  └─ BlockStateParseUtil.h
	│
	├─ Core
	│  ├─ CoreTypes.h
	│  ├─ EncodingUtil.h
	│  ├─ HashUtil.h
	│  ├─ ObjectTypes.h
	│  └─ StringUtil.h
	│
	├─ Minecraft
	│  ├─ MinecraftCore.h
	│  ├─ VoxelTypes.h
	│  ├─ ChunkRenderTypes.h
	│  ├─ MCModelTypes.h
	│  ├─ MCModelUtil.h
	│  ├─ MCModelUtil.cpp
	│  ├─ MCResourceUtil.h
	│  └─ MCResourceUtil.cpp
	│
	├─ Render
	│  ├─ RenderTypes.h
	│  ├─ ShaderTypes.h
	│  ├─ VertexLayoutTypes.h
	│  └─ VertexTypes.h
	│
	└─ World
		├─ ChunkMath.h
		└─ ChunkTypes.h
*/

// 전역에 걸쳐서 쓸 가치가 있는 util들
#include "HashUtil.h"
#include "MathUtil.h"	
#include "EncodingUtil.h"
#include "StringUtil.h"
#include "DebugUtil.h"
#include "IFileWrapper.h"
#include "CRapidJsonParseWrapper.h"