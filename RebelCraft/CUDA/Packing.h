#ifndef __PACKING__
#define __PACKING__

#include <optixu/optixu_math_namespace.h>
using namespace optix;

__device__ float4 R8G8B8A8_UNORM_to_float4(uint packedInput)
{
	float4 unpackedOutput;
	unpackedOutput.x = (float)  (packedInput      & 0x000000ff) / 255;
	unpackedOutput.y = (float) ((packedInput>> 8) & 0x000000ff) / 255;
	unpackedOutput.z = (float) ((packedInput>>16) & 0x000000ff) / 255;
	unpackedOutput.w = (float) ((packedInput>>24) & 0x000000ff) / 255;
	unpackedOutput.x = unpackedOutput.x * 2 - 1;
	unpackedOutput.y = unpackedOutput.y * 2 - 1;
	unpackedOutput.z = unpackedOutput.z * 2 - 1;	
	return unpackedOutput;
}

// Quantize float4 to 8 bit per component
__device__ uint float4_to_R8G8B8A8_UNORM(float4 unpackedInput)
{
	uint packedOutput;	
	unpackedInput.x = unpackedInput.x * 0.5f + 0.5f;
	unpackedInput.y = unpackedInput.y * 0.5f + 0.5f;
	unpackedInput.z = unpackedInput.z * 0.5f + 0.5f;	
	unpackedInput.x = min(max(unpackedInput.x,0.0f), 1.0f);	// NaN gets set to 0.
	unpackedInput.y = min(max(unpackedInput.y,0.0f), 1.0f);	// NaN gets set to 0.
	unpackedInput.z = min(max(unpackedInput.z,0.0f), 1.0f);	// NaN gets set to 0.
	unpackedInput.w = min(max(unpackedInput.w,0.0f), 1.0f);	// NaN gets set to 0.
	unpackedInput.x = unpackedInput.x * 255.0f + 0.5f;
	unpackedInput.y = unpackedInput.y * 255.0f + 0.5f;
	unpackedInput.z = unpackedInput.z * 255.0f + 0.5f;
	unpackedInput.w = unpackedInput.w * 255.0f + 0.5f;
	
	unpackedInput = floor(unpackedInput);
	packedOutput =	(((uint)unpackedInput.x)      |
					(((uint)unpackedInput.y)<< 8) |
					(((uint)unpackedInput.z)<<16) |
					(((uint)unpackedInput.w)<<24) );
	return packedOutput;
}

#endif