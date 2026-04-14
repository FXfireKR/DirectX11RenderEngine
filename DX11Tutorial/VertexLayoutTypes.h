#pragma once
#include <vector>
#include <cassert>
#include <functional>

#include <d3d11.h>
#include <dxgiformat.h>

enum class VERTEX_SEMANTIC
{
    COLOR,
    NORMAL,
    POSITION,
    POSITIONT,
    PSIZE,
    TANGENT,
    TEXCOORD,
    BINORMAL,
    BLENDINDICES,
    BLENDWEIGHT,
    // PSIZE
    // TESSFACTOR
    // VFACE
    // VPOS
    // DEPTH
};

constexpr const char* GetVertexSemanticString(VERTEX_SEMANTIC eVertexSemantic_)
{
    switch (eVertexSemantic_)
    {
    case VERTEX_SEMANTIC::COLOR: return "COLOR";
    case VERTEX_SEMANTIC::NORMAL: return "NORMAL";
    case VERTEX_SEMANTIC::POSITION: return "POSITION";
    case VERTEX_SEMANTIC::POSITIONT: return "POSITIONT";
    case VERTEX_SEMANTIC::PSIZE: return "PSIZE";
    case VERTEX_SEMANTIC::TANGENT: return "TANGENT";
    case VERTEX_SEMANTIC::TEXCOORD: return "TEXCOORD";
    case VERTEX_SEMANTIC::BINORMAL: return "BINORMAL";
    case VERTEX_SEMANTIC::BLENDINDICES: return "BLENDINDICES";
    case VERTEX_SEMANTIC::BLENDWEIGHT: return "BLENDWEIGHT";
    }
    assert(false && "Invalid VERTEX_SEMANTIC");
    return "";
}

struct VertexElementDesc
{
    VERTEX_SEMANTIC eSemantic;
    uint32_t uSemanticIndex = 0;
    DXGI_FORMAT dxgiFormat;
    uint32_t uOffset;
    uint32_t uInputSlot = 0;
    D3D11_INPUT_CLASSIFICATION eInputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    uint32_t uInstanceDataStepRate = 0;

    // initailizer-helper
    VertexElementDesc(VERTEX_SEMANTIC eSemantic_, DXGI_FORMAT dxgiFormat_
        , uint32_t uOffset_ = D3D11_APPEND_ALIGNED_ELEMENT, uint32_t uSemanticIndex_ = 0
        , uint32_t uInputSlot_ = 0, D3D11_INPUT_CLASSIFICATION eInputSlotClass_ = D3D11_INPUT_PER_VERTEX_DATA
        , uint32_t uInstanceDataStepRate_ = 0
    )
        : eSemantic(eSemantic_)
        , uSemanticIndex(uSemanticIndex_)
        , dxgiFormat(dxgiFormat_)
        , uOffset(uOffset_)
        , uInputSlot(uInputSlot_)
        , eInputSlotClass(eInputSlotClass_)
        , uInstanceDataStepRate(uInstanceDataStepRate_)
    {
    }

    bool operator==(const VertexElementDesc& rhs_) const
    {
        return eSemantic == rhs_.eSemantic &&
            uSemanticIndex == rhs_.uSemanticIndex &&
            dxgiFormat == rhs_.dxgiFormat &&
            uOffset == rhs_.uOffset &&
            uInputSlot == rhs_.uInputSlot &&
            eInputSlotClass == rhs_.eInputSlotClass &&
            uInstanceDataStepRate == rhs_.uInstanceDataStepRate;
    }
};

struct VertexLayoutDesc
{
    std::vector<VertexElementDesc> vecElements;
    uint32_t uStride = 0;

    bool operator==(const VertexLayoutDesc& rhs_) const {
        if (uStride != rhs_.uStride) return false;
        if (vecElements.size() != rhs_.vecElements.size()) return false;

        for (size_t i = 0; i < vecElements.size(); ++i) {
            const auto& lhs = vecElements[i];
            const auto& rhs = rhs_.vecElements[i];
            if (lhs != rhs) return false;
        }
        return true;
    }
};

struct VertexLayoutDescHash
{
    size_t operator()(const VertexLayoutDesc& desc) const
    {
        size_t seed = 0;
        auto hash_combine = [&seed](size_t v) {
            seed ^= v + 0x9e3779b97f4a7c15ull + (seed << 6) + (seed >> 2);
            };

        hash_combine(std::hash<uint32_t>{}(desc.uStride));
        hash_combine(std::hash<size_t>{}(desc.vecElements.size()));

        for (const auto& e : desc.vecElements)
        {
            hash_combine(std::hash<uint32_t>{}(static_cast<uint32_t>(e.eSemantic)));
            hash_combine(std::hash<uint32_t>{}(e.uSemanticIndex));
            hash_combine(std::hash<uint32_t>{}(static_cast<uint32_t>(e.dxgiFormat)));
            hash_combine(std::hash<uint32_t>{}(e.uOffset));
            hash_combine(std::hash<uint32_t>{}(e.uInputSlot));
            hash_combine(std::hash<uint32_t>{}(static_cast<uint32_t>(e.eInputSlotClass)));
            hash_combine(std::hash<uint32_t>{}(e.uInstanceDataStepRate));
        }
        return seed;
    }
};