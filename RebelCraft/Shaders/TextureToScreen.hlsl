// a simple shader for rendering textures on the screen
// whatevs.

#ifndef TEXTURE_TO_SCREEN_HLSL
#define TEXTURE_TO_SCREEN_HLSL

#include "ShaderConstants.hlsl"
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D gDisplayTex : register(t0);


cbuffer cbConstant : register(b2)
{
	float4 destRect;
};

struct VS_INPUT
{
	float3 Pos          : POSITION;       
	float2 Tex          : TEXCOORD0;      
};

struct GSPS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// DepthStates
//--------------------------------------------------------------------------------------
DepthStencilState DisableDepth
{
	DepthEnable = FALSE;
	DepthWriteMask = ALL;
	DepthFunc = LESS_EQUAL;
};

BlendState NoBlending
{
	AlphaToCoverageEnable = FALSE;
	BlendEnable[0] = FALSE;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
GSPS_INPUT TextureToScreenVS( VS_INPUT input )
{
	GSPS_INPUT output = (GSPS_INPUT)0;
	
	output.Pos = float4(input.Pos + float3( destRect.xy, 0), 1);
	output.Tex = input.Tex;
	
	return output;
}


//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
[maxvertexcount(4)]
void TextureToScreenGS( point GSPS_INPUT input[1], inout TriangleStream<GSPS_INPUT> TriStream )
{
	GSPS_INPUT output;
	
	//TODO compute and output a quad from given position and drawSize
	//upper left
	output.Pos = input[0].Pos;
	output.Tex = input[0].Tex;
	TriStream.Append( output );

	//upper right
	output.Pos = input[0].Pos + float4(destRect.z, 0, 0, 0);
	output.Tex = input[0].Tex + float2(1.0, 0);
	TriStream.Append( output );

	//lower left
	output.Pos = input[0].Pos + float4(0, destRect.w, 0, 0);
	output.Tex = input[0].Tex + float2(0.0, 1.0);
	TriStream.Append( output );

	//lower right
	output.Pos = input[0].Pos + float4(destRect.zw, 0, 0);
	output.Tex = input[0].Tex + float2(1.0, 1.0);
	TriStream.Append( output );

	TriStream.RestartStrip();
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 TextureToScreenPS( GSPS_INPUT input) : SV_Target
{

	//return float4(1, 0, 0, 1);
	//TODO distribute values equally (histogram)
	float3 color = gDisplayTex.Sample(defaultSampler, input.Tex).rgb;
	if(length(color) > 3.0f)
		color /= 1000.0f; 
	return float4(color, 1.0);
}

#endif // TEXTURE_TO_SCREEN_HLSL