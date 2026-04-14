#pragma once
#include "CWorldTime.h"

struct WorldLightingParams
{
    XMFLOAT3 sunDir = { 0.f, 1.f, 0.f };
    XMFLOAT3 moonDir = { 0.f,-1.f, 0.f };

    XMFLOAT3 sunColor = { 1.0f, 0.97f, 0.92f };
    float sunIntensity = 1.0f;

    XMFLOAT3 ambientColor = { 0.25f, 0.27f, 0.30f };
    float ambientStrength = 1.0f;

    XMFLOAT3 skyColor = { 0.55f, 0.72f, 0.95f };

    bool shadowEnabled = true;
};

class CWorldLight
{
public:
    CWorldLight() = default;
    ~CWorldLight() = default;

    WorldLightingParams Evaluate(const WorldTimeParams& timeParams) const;

private:
    XMFLOAT3 _LerpColor(const XMFLOAT3& a, const XMFLOAT3& b, float t) const;
};