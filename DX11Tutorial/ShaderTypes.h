#pragma once

#include <cstdint>
#include <functional>

enum class SHADER_MACRO : uint32_t
{
    USE_INSTANCING = 1 << 0,
    USE_SKINNING = 1 << 1,
    USE_SHADOW = 1 << 2,

    // 여기 추가되면 _ConvertFlagToShaderMacro 함수에도 추가해야함.
};

struct ShaderKey
{
    uint64_t uBaseShaderID = 0;
    uint32_t uMacroFlags = 0;

    bool operator==(const ShaderKey& other_) const 
    {
        return (uBaseShaderID == other_.uBaseShaderID) 
            && (uMacroFlags == other_.uMacroFlags);
    }
};

struct ShaderKeyHash
{
    size_t operator()(const ShaderKey& key_) const 
    {
        size_t h1 = std::hash<uint64_t>{}(key_.uBaseShaderID);
        size_t h2 = std::hash<uint32_t>{}(key_.uMacroFlags);
        return h1 ^ (h2 << 1); // Combine hashes
    }
};