// a simple shader for rendering textures on the screen
// whatevs.

#ifndef TONEMAPPING_HLSL
#define TONEMAPPING_HLSL

#include "ShaderConstants.hlsl"
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D gDisplayTex : register(t0);

struct VS_INPUT
{
	float3 Pos          : POSITION;       
	float2 Tex			: TEXCOORD;      
};

struct PS_INPUT
{
	float4 Pos	: SV_POSITION;
	float2 Tex	: TEXCOORD; 
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT ToneMappingVS( VS_INPUT input )
{
	PS_INPUT output = (PS_INPUT)0;
	
	output.Pos = float4(input.Pos, 1);
	output.Tex = input.Tex;
	
	return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 ToneMappingPS( PS_INPUT input) : SV_Target
{
	// generate mipmaps, use lowest level for luminance value
	// adjust tonemapping values accordingly
	float4 result = float4(gDisplayTex.SampleLevel(defaultSampler, input.Tex, 0).rgb, 1);	

	float luminance = 0.3f * result.r + 0.6f * result.g + 0.1f * result.b;

	float mappedLuminance = clamp(0,1,luminance / luminanceMax);
	mappedLuminance = pow(mappedLuminance, GAMMA);

	result.rgb = (mappedLuminance/luminance) * result.rgb;
		
	return result;
}

#endif // TEXTURE_TO_SCREEN_HLSL