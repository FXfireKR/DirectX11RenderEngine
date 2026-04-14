
struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD;
    float4 color    : COLOR;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 normalWS : TEXCOORD1;
    float2 uv       : TEXCOORD0;
    float4 color    : COLOR;
    float4 shadowPos: TEXCOORD2;
};

cbuffer CBFrame : register(b0)
{
    float4x4 viewMatrix;
    float4x4 projMatrix;
    
    float4 lightDirWs; // xyz = direction to light
    float4 lightColorIntensity; // rgb = light color, a = intensity
    float4 ambientColor; // rgb = ambient
    float4 skyColor;

    float4x4 lightViewProj;
    float4 shadowParams;
};

cbuffer CBObject : register(b1)
{
    float4x4 worldMatrix;
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
    output.uv = input.uv;
    output.color = input.color;

    float3 normalWS = mul(input.normal, (float3x3)worldMatrix);
    output.normalWS = normalize(normalWS);

    output.shadowPos = mul(worldPos, lightViewProj);

    return output;
}

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

Texture2D shadowMap : register(t1);
SamplerComparisonState shadowSampler : register(s1);

float ComputeShadowFactor(float4 shadowPos, float3 N, float3 L)
{
    if (shadowPos.w <= 0.00001f)
        return 1.0f;

    float3 ndc = shadowPos.xyz / shadowPos.w;

    float2 shadowUV;
    shadowUV.x = ndc.x * 0.5f + 0.5f;
    shadowUV.y = -ndc.y * 0.5f + 0.5f;

    if (shadowUV.x < 0.0f || shadowUV.x > 1.0f ||
        shadowUV.y < 0.0f || shadowUV.y > 1.0f ||
        ndc.z < 0.0f || ndc.z > 1.0f)
    {
        return 1.0f;
    }

    float baseBias = shadowParams.x;
    float slopeFactor = 1.0f - saturate(dot(N, L));
    float bias = baseBias * (1.0f + 4.0f * slopeFactor);

    uint w, h;
    shadowMap.GetDimensions(w, h);
    float2 texelSize = 1.0f / float2(w, h);

    float compareDepth = ndc.z - bias;

    float lit = 0.0f;

    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            float2 offset = float2(x, y) * texelSize;
            lit += shadowMap.SampleCmpLevelZero(shadowSampler, shadowUV + offset, compareDepth);
        }
    }

    lit /= 9.0f;

    // 0이면 완전 그림자, 1이면 완전 lit
    return lerp(shadowParams.y, 1.0f, lit);
}

float4 PS(VS_OUTPUT input) : SV_Target
{
    float4 tex0Color = texture0.Sample(sampler0, input.uv);

    float3 tint = input.color.rgb;
    float blockLight01 = saturate(input.color.a);

    float3 albedo = tex0Color.rgb * tint;
    float alpha = tex0Color.a;
    clip(alpha - 0.5f);

    float3 N = normalize(input.normalWS);
    float3 L = normalize(lightDirWs.xyz);

    float NdotL = saturate(dot(N, L));

    float3 ambient = ambientColor.rgb;
    float3 direct = lightColorIntensity.rgb * (NdotL * lightColorIntensity.a);

    float shadowFactor = ComputeShadowFactor(input.shadowPos, N, L);
    float3 shadowedDirect = direct * shadowFactor;

    // torch / block light
    // 곡선을 살짝 세워서 중간 레벨도 체감되게
    float localL = saturate(pow(blockLight01, 0.80f));

    // 따뜻한 계열 local light
    float3 localLight = float3(1.00f, 0.92f, 0.82f) * (localL * 1.20f);

    // 낮에는 태양광이 우세, 밤에는 local light가 우세
    float3 lighting = ambient + max(shadowedDirect, localLight);

    return float4(albedo * lighting, alpha);
}



// debug out
// float4 PS(VS_OUTPUT input) : SV_Target
// {
//      return float4(1.0f, 1.0f, 1.0f, 1.0f);
// }