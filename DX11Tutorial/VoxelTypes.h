#pragma once
#include <cstdint>
#include <limits>

using BLOCK_ID = uint16_t;		// Block Registry에서 부여된 내부 ID
using STATE_INDEX = uint16_t;	// 블록 당 상태 조합수 65535 이하로 가정
using PROPERTY_ID = uint16_t;	// 프로퍼티 값
using VALUE_INDEX = uint8_t;	// property 내부 값 인덱스

using PROP_HASH = uint64_t;
using VALUE_HASH = uint64_t;

using MODEL_ID = uint32_t;

constexpr BLOCK_ID INVALID_BLOCK_ID = UINT16_ERROR;
constexpr STATE_INDEX INVALID_STATE_INDEX = UINT16_ERROR;