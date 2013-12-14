// Copyright John McLaughlin
// have fun with it, and credit me.
// sending me a note you used this, would be awesome. Would be great to know people actually use this!
// ##################################################
// splats the Bouncemap
// requires:
//		- Camera GBuffer
//		- BounceMap SRVs
//		- Light View Projection Matrix
// ##################################################


#ifndef SPLATBOUNCEMAP_HLSL
#define SPLATBOUNCEMAP_HLSL

#include "ShaderConstants.hlsl"

// sampler
SamplerState splatSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = BORDER;
	AddressV = BORDER;
	AddressW = BORDER;
	BorderColor = 0x000000;
};

tbuffer textures
{
	//Gbuffer: specular, albedo, transmittance (rgb: transmittance, a: eta_material), normal
	Texture2D<float4>	specularMap		: register(t4);
	Texture2D<float4>	diffuseMap		: register(t5);
	Texture2D<float4>	transmittanceMap		: register(t6);
	Texture2D<float4>	normalMap		: register(t7);
	Texture2D<float4>	wsPositionMap	: register(t8);
	Texture2D<float4>	powerDistanceMap: register(t9);
	Texture2D<float4>	directionDensity: register(t10);
};

cbuffer splatConstants : register(b5)
{
	float4x4 lightViewProjection;
};

// uses Vertex and Geometry shader from TextureToScreen.hlsl

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD1;
};

// ### Pixel Shader ###
float4 SplatBounceMapPS(VS_OUTPUT input) : SV_Target
{
	float4 result = float4(0,0,0,1);

	float4 wsPosition		 = wsPositionMap.Sample(defaultSampler, input.Tex);
	//
	float4 lightViewSpacePos = mul(wsPosition, lightViewProjection);
	float4 powerDistance	 = powerDistanceMap.Sample(defaultSampler, lightViewSpacePos.xy);

	// object is behind lighted area (in shadow)
	if(lightViewSpacePos.z >= powerDistance.w)
		return float4(0, 0, 0, 1);

	float2 texCoordLight = float2(	saturate(lightViewSpacePos.x/lightViewSpacePos.w),
									saturate(lightViewSpacePos.y/lightViewSpacePos.w));
	result = float4(powerDistanceMap.Sample(defaultSampler, texCoordLight).rgb, 1.0);
	//result = float4(
					//saturate(lightViewSpacePos.x/lightViewSpacePos.w),
					//saturate(lightViewSpacePos.y/lightViewSpacePos.w), 
					//0, 1.0);

	return result;
}

#endif // SPLATBOUNCEMAP_HLSL
