
struct VS_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

cbuffer CBFrame : register(b0)
{
    float4x4 viewMatrix;
    float4x4 projMatrix;
    
    float4 lightDirWs;
    float4 lightColorIntensity;
    float4 ambientColor;

    float4 skyColor;
};

cbuffer CBObject : register(b1)
{
    float4x4 worldMatrix;
};

// IA - VS - RS - PS - OM
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    float4 position = float4(input.position, 1.0f);

    float4 worldPos = mul(position, worldMatrix);
    float4 viewPos = mul(worldPos, viewMatrix);
    float4 projPos = mul(viewPos, projMatrix);

    output.position = projPos;
    output.uv = input.uv;

    return output;
}

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

float4 PS(VS_OUTPUT input) : SV_Target
{
    float4 tex = texture0.Sample(sampler0, input.uv);
    float luma = dot(tex.rgb, float3(0.299f, 0.587f, 0.114f));
    float mask = smoothstep(0.0f, 0.05f, luma);

    float3 outRGB = lerp(skyColor.rgb, tex.rgb, mask);
    return float4(outRGB.rgb, 1.0f);
}