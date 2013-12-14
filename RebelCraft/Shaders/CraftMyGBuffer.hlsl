// Copyright John McLaughlin
// have fun with it, and credit me.
// sending me a note you used this, would be awesome. Would be great to know people actually use this!

//############
//	CraftMyGBuffer.hlsl
//	creates a Gbuffer with specular, diffuse, transmittance and normal value
//	depth is the z-value in view space, for easy position reconstruction, if needed
#ifndef CRAFTMYGBUFFER_HLSL
#define CRAFTMYGBUFFER_HLSL
#endif

//### input 

#include "ShaderConstants.hlsl"

//Gbuffer: specular, albedo, transmittance (rgb: transmittance, a: eta_material), normal
// or: calculate yourself, do without GBuffer, no GBuffer computation beforehand?
//tbuffer textures: register(t0)
//{
	Texture2D<float4> diffuseMap: register(t0);
	Texture2D<float4> specularMap: register(t1);
	Texture2D<float4> transmissiveMap: register(t2);
	Texture2D<float4> normalMap: register(t3);

//}


cbuffer cbOmniMaterial: register(b2)
{
	float4 diffuseColor;
	float4 specularColor;
	float4 transmissive;
}

//### output structs
struct GBuffer
{
	float4 specular : SV_Target0;
	float4 diffuse : SV_Target1;
	float4 transmittance : SV_Target2;
	float4 normal	: SV_Target3;
	float4 position : SV_Target4;
	//float depth : SV_Target4;
};

//### shader output/input structs

struct VertexInput
{
	float3 position: POSITION;
	float3 normal: NORMAL;
	float2 texCoord: TEXCOORD;
};

struct VertexOutput
{
	float4 position     : SV_Position;
	float4 normal       : normal;
	float2 texCoord     : texCoord;
	float4 worldPosition : WorldPosition;
};

//### Vertex Shader
VertexOutput CraftMyGBufferVS(VertexInput input)
{
	VertexOutput output;
	
	//float4 positionView = mul(float4(input.position, 1.0f), mul(World, View));
	//output.position     = mul(positionView, Projection);
	output.position		= mul(float4(input.position, 1.0f), WorldViewProj);
	//output.position.w	= 1.0f;
	output.normal       = mul(float4(input.normal, 0.0f),  World); //mul(World, View)); // KEINE 1.0f  bei NORMALE
	output.texCoord     = input.texCoord;
	output.worldPosition = mul(float4(input.position, 1.0f),World);//output.position;

	return output;
}

//### Pixel Shader
void CraftMyGBufferPS(VertexOutput input, out GBuffer craftedGBuffer)
{
	//craftedGBuffer.specular = specularMap.Sample(defaultSampler, input.texCoord, int2(0,0));// * specularConstant;
	//craftedGBuffer.diffuse = diffuseMap.Sample(defaultSampler, input.texCoord);// * diffuseConstant;
	craftedGBuffer.specular = specularColor;
	craftedGBuffer.diffuse = diffuseColor;
	craftedGBuffer.transmittance = transmissive;//float3(1,1,1); //transmissiveConstant;//transmissiveMap.sample(defaultSampler, input.texCoord).rgb * transmissiveConstant;b
	//craftedGBuffer.transmittance.a = refractiveIndexETA;
	craftedGBuffer.normal = normalize(input.normal); // or normalMap? combine with normalMap?
	craftedGBuffer.position = float4(input.worldPosition.xyz, 1.0);//input.depthVS / cameraFar;

	////testor
	//craftedGBuffer.specular = float4(1.0,0.0,0.0,1.0);
	//craftedGBuffer.diffuse = float4(0.0,1.0,0.0,1.0);
	//craftedGBuffer.transmittance = float4(0.0,0.0,1.0,1.0);
	//craftedGBuffer.normal = float4(1.0,0.0,1.0,1.0);; // or normalMap? combine with normalMap?
	//craftedGBuffer.depth = 1.0;

	return;
};

//### Vertex Shader for lights
VertexOutput CraftMyLightGBufferVS(VertexInput input)
{
	VertexOutput output;
	
	//float4 positionView = mul(float4(input.position, 1.0f), mul(World, View));
	//output.position     = mul(positionView, Projection);
	output.position		= mul(float4(input.position, 1.0f),  lightWorldViewProj);
	//output.position.w	= 1.0f;
	output.normal       = mul(float4(input.normal, 0.0f),  lightWorld); //mul(World, View));
	output.texCoord     = input.texCoord;
	output.worldPosition = mul(float4(input.position, 1.0f), lightWorld);//output.position;

	return output;
}

//### Pixel Shader
void CraftMyLightGBufferPS(VertexOutput input, out GBuffer craftedGBuffer)
{

	//TODO handle transparent materials
	//if(length(transmissive.xyz) >= 1)
		//discard;

	//craftedGBuffer.specular = specularMap.Sample(defaultSampler, input.texCoord, int2(0,0));// * specularConstant;
	//craftedGBuffer.diffuse = diffuseMap.Sample(defaultSampler, input.texCoord);// * diffuseConstant;
	craftedGBuffer.specular = specularColor;
	craftedGBuffer.diffuse = diffuseColor;
	// refractiveIndex of material is already in transmissive.a
	craftedGBuffer.transmittance = transmissive;//float3(1,1,1); //transmissiveConstant;//transmissiveMap.sample(defaultSampler, input.texCoord).rgb * transmissiveConstant;b
	//craftedGBuffer.transmittance.a = lightRefractiveIndexETA;
	craftedGBuffer.normal = normalize(input.normal); // or normalMap? combine with normalMap?
	craftedGBuffer.position = float4(input.worldPosition.xyz, 1.0);//input.depthVS / cameraFar;

	////testor
	//craftedGBuffer.specular = float4(1.0,0.0,0.0,1.0);
	//craftedGBuffer.diffuse = float4(0.0,1.0,0.0,1.0);
	//craftedGBuffer.transmittance = float4(0.0,0.0,1.0,1.0);
	//craftedGBuffer.normal = float4(1.0,0.0,1.0,1.0);; // or normalMap? combine with normalMap?
	//craftedGBuffer.depth = 1.0;

	return;
};