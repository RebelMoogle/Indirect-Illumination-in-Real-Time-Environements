// a simple shader for rendering textures on the screen
// whatevs.

#ifndef OMNI_MATERIAL_HLSL
#define OMNI_MATERIAL_HLSL

#include "ShaderConstants.hlsl"

cbuffer cbOmniMaterial: register(b2)
{
	float4 diffuseColor;
	float4 specularColor;
	float4 transmissiveColor;
}
struct VS_INPUT
{
	float3 Pos          : POSITION;       
	float3 Normal          : NORMAL0;      
};

struct GSPS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 wsPos : WORLDPOSITION;
	float3 Normal : NORMAL0;     
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
GSPS_INPUT OmniMaterialVS( VS_INPUT input )
{
	GSPS_INPUT output = (GSPS_INPUT)0;
	
	output.wsPos = float4(input.Pos, 1);
	//test
	output.Pos = mul(output.wsPos, WorldViewProj);
	output.Normal = input.Normal; //mul(input.Normal, (float3x3)normWorld);
	
	return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 OmniMaterialPS( GSPS_INPUT input) : SV_Target
{
	float3 lightPosition = float3(278.0f, 530.7f, 279.5f);
	//return float4(input.Normal, 1);
	//TODO distribute values equally (histogram)
	
	return dot(input.Normal, normalize(lightPosition-input.wsPos.xyz)) * diffuseColor;
	//return float4(1.0, 0, 0, 1.0);
}




#endif // RENDERVERTEX_HLSL