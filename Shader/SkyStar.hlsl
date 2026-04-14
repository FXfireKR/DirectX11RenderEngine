
struct VS_INPUT
{
    float3 position : POSITION;
    float2 uv       : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD0;
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

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

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
    float4 tex = texture0.Sample(sampler0, input.uv);

    float skyLuma = dot(skyColor.rgb, float3(0.299f, 0.587f, 0.114f));
    float visibility = saturate((0.22f - skyLuma) / 0.18f);

    float alpha = tex.a * visibility;
    float3 rgb = tex.rgb;

    return float4(rgb, alpha);
}