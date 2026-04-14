struct VS_INPUT
{
    float4 position : POSITION;
    float4 color : COLOR;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

cbuffer CBFrame : register(b0)
{
    float4x4 viewMatrix;
    float4x4 projMatrix;
};

cbuffer CBObject : register(b1)
{
    float4x4 worldMatrix;
};

// IA - VS - RS - PS - OM
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    float4 worldPos = mul(input.position, worldMatrix);
    float4 viewPos = mul(worldPos, viewMatrix);
    float4 projPos = mul(viewPos, projMatrix);

    output.position = projPos;
    output.color = input.color;
    return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
    return input.color;
}

//// Global Buffer
//cbuffer MatrixBuffer
//{
//    matrix worldMatrix;
//    matrix viewMatrix;
//    matrix projectionMatrix;
//};

//cbuffer LightBuffer
//{
//    float4 ambientColor;
//    float4 diffuseColor;
//    float3 lightDirection;
//    float specularPower;
//    float4 specularColor;
//};

//cbuffer CameraBuffer
//{
//    float3 cameraPosition;
//    float padding;
//};

//// Input definition
//struct VertexInputType
//{
//    float4 position : POSITION0;
//    float2 tex : TEXCOORD0;
//    float3 normal : NORMAL0;
//};

//struct PixelInputType
//{
//    float4 position : SV_POSITION;
//    float2 tex : TEXCOORD0;
//    float3 normal : NORMAL0;
//    float3 viewDirection : TEXCOORD1;
//};


//// Vertex Shader
//PixelInputType LightVertexShader(VertexInputType input)
//{
//    PixelInputType output;
//    float4 worldPosition;
	
//    // Change the position vector to be 4 units for proper matrix calculations.
//	input.position.w = 1.0f;
    
//    // Calculate the position of the vertex against the world, view, and projection matrices.
//	output.position = mul(input.position, worldMatrix);
//	output.position = mul(output.position, viewMatrix);
//	output.position = mul(output.position, projectionMatrix);
	
//    output.tex = input.tex;
	
//    // Calculate the normal vector against the world matrix only.
//    output.normal = mul(input.normal, (float3x3) worldMatrix);
    
//    // Normalize the normal vector.
//    output.normal = normalize(output.normal);
    
//    worldPosition = mul(input.position, worldMatrix);
//    output.viewDirection = cameraPosition.xyz - worldPosition.xyz;
//    output.viewDirection = normalize(output.viewDirection);
    
//	return output;
//}

//// Pixel Shader
//Texture2D shaderTexture : register(t0);
//SamplerState samplerType : register(s0);

//float4 LightPixelShader(PixelInputType input) : SV_TARGET
//{
//    float4 textureColor;
//    float3 lightDir;
//    float lightIntensity;
//    float4 color;
//    float3 reflection;
//    float4 specular;
    
//    textureColor = shaderTexture.Sample(samplerType, input.tex);
    
//    color = ambientColor;
    
//    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
//    // invert direction
//    lightDir = -lightDirection;
    
//    // Calculate the amount of light on this pixel.
//    lightIntensity = saturate(dot(input.normal, lightDir));
//    if (lightIntensity > 0.0f)
//    {
//        color += (diffuseColor * lightIntensity);
        
//        // Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
//        color = saturate(color);
        
//        // Calculate the reflection vector based on the light intensity, normal vector, and light direction.
//        reflection = normalize(2.0f * lightIntensity * input.normal - lightDir);
        
//        specular = pow(saturate(dot(reflection, input.viewDirection)), specularPower);
        
//        specular = specular * specularColor;

//    }
    
//    // Multiply the texture pixel and the final diffuse color to get the final pixel color result.
//    color = color * textureColor;
    
//    color = saturate(color + specular);
    
//    color = textureColor; //float4(1.0f, 1.0f, 1.0f, 1.0f);
    
//    return color;
//}