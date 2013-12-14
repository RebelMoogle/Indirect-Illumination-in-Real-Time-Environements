

// TODO::::: !!! : !::!°: : !: (Width and Height)
//uint Width = 1024;
//uint Height = 768;

#define WIDTH 1024

ByteAddressBuffer optixOutput : register(t0);

struct VS_INPUT10
{
    float3 Position : POSITION0;
};

struct PS_INPUT10
{
    float4 Position : SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT10 OptixDebugRenderVS(VS_INPUT10 input)
{
    PS_INPUT10 output;    	
    output.Position = float4(input.Position,1);
    return output;
}

float4 R8G8B8A8_UNORM_to_float4(uint packedInput)
{
	precise float4 unpackedOutput;
	unpackedOutput.r = (float)  (packedInput      & 0x000000ff) / 255;
	unpackedOutput.g = (float) ((packedInput>> 8) & 0x000000ff) / 255;
	unpackedOutput.b = (float) ((packedInput>>16) & 0x000000ff) / 255;
	unpackedOutput.a = (float) ((packedInput>>24) & 0x000000ff) / 255;
	unpackedOutput.xyz *= 2;
	unpackedOutput.xyz -= 1;
	return unpackedOutput;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 OptixDebugRenderPS( PS_INPUT10 input) : SV_Target
{
	//return float4(1,1,0,1);
	uint addr = (input.Position.y * WIDTH + input.Position.x - WIDTH / 2u) * 4u;
	return R8G8B8A8_UNORM_to_float4(optixOutput.Load(addr));
}

