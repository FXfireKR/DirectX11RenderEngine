
struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
    float4 color    : COLOR;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
};

cbuffer CBFrame : register(b0)
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

cbuffer CBObject : register(b1)
{
    float4x4 worldMatrix;
};

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;

    float4 localPos = float4(input.position, 1.0f);
    float4 worldPos = mul(localPos, worldMatrix);
    output.position = mul(worldPos, lightViewProj);

    return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
    return float4(1, 1, 1, 1);
}