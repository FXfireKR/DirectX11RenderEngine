
struct VS_INPUT
{
    float3 position : POSITION;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
};

cbuffer CBFrame : register(b0)
{
    float4x4 viewMatrix;
    float4x4 projMatrix;
    //float4x4 lightViewMatrix;
};

cbuffer CB_Object : register(b1)
{
    float4x4 worldMatrix;
    float4 gColor;
};

// IA - VS - RS - PS - OM
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    float4 position = float4(input.position.xyz, 1.0f);

    float4 worldPos = mul(position, worldMatrix);
    float4 viewPos = mul(worldPos, viewMatrix);
    float4 projPos = mul(viewPos, projMatrix);

    output.position = projPos;
    return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
    return float4(1.0f,0.0f,0.0f,1.0f);
}