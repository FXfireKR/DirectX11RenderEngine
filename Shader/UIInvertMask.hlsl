
struct VS_INPUT
{
    float3 position : POSITION;
    float2 uv       : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD;
};

cbuffer CB_Frame : register(b0)
{
    float4x4 viewMatrix;
    float4x4 projMatrix;

    float4 lightDirWs;
    float4 lightColorIntensity;
    float4 ambientColor;
    float4 skyColor;

    float4x4 lightViewProj;
    float4 shadowParams;
};

cbuffer CB_Object : register(b1)
{
    float4x4 worldMatrix;
};

Texture2D gTex : register(t0);
SamplerState gSamp : register(s0);

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;

    float4 pos = float4(input.position, 1.0f);
    float4 worldPos = mul(pos, worldMatrix);
    float4 viewPos = mul(worldPos, viewMatrix);
    float4 projPos = mul(viewPos, projMatrix);

    output.position = projPos;
    output.uv = input.uv;
    return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
    float4 tex = gTex.Sample(gSamp, input.uv);

    // alpha만 반전 마스크로 사용
    float m = tex.a;

    // 너무 약한 가장자리는 버리고 싶으면 이 줄 사용
    // clip(m - 0.05f);

    return float4(m, m, m, m);
}