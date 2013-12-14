// a simple shader for rendering textures on the screen
// whatevs.

#ifndef RENDERVERTEX_HLSL
#define RENDERVERTEX_HLSL


#include "ShaderConstants.hlsl"

struct VS_INPUT
{
	float3 position          : POSITION;       
	float4 texCoord          : TEXCOORD;      
	//float4 outgoingPower		: POWER;
	//float3 outgoingDirection	: DIRECTION;
	//float  pathDensity			: DENSITY;
	//float4 surfaceNormal		: NORMAL;
};

struct GSPS_INPUT
{
	float4 position				: SV_POSITION;
	float4 texCoord				: TEXCOORD;
	//float4 outgoingPower		: POWER;
	//float3 outgoingDirection	: DIRECTION;
	//float  pathDensity			: DENSITY;
	//float4 surfaceNormal		: NORMAL;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
GSPS_INPUT RenderVertexVS( VS_INPUT input )
{
	GSPS_INPUT output = (GSPS_INPUT)0;
	
	output.position = float4(input.position, 1);
	//test
	//output.Pos = mul(output.Pos, WorldViewProj);
	output.texCoord = input.texCoord;

	//output.outgoingPower = input.outgoingPower;
	//output.outgoingDirection = input.outgoingDirection;
	//output.pathDensity = input.pathDensity;
	//output.surfaceNormal = input.surfaceNormal;

	return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 RenderVertexPS( GSPS_INPUT input) : SV_Target
{
	//return input.surfaceNormal;
//	return float4(1, 0, 0, 1);
	//TODO distribute values equally (histogram)
	return float4(input.texCoord.xy, 0, 1);
	//return float4(1.0, 0, 0, 1.0);
}




#endif // RENDERVERTEX_HLSL