// Copyright John McLaughlin
// have fun with it, and credit me.
// sending me a note you used this, would be awesome. Would be great to know people actually use this!

#ifndef SHADERCONSTANTS_HLSL
#define SHADERCONSTANTS_HLSL

#define PI 3.141592653589793238

//cornell
//#define MIN_RADIUS 20.0f
//#define MAX_RADIUS 55.0f
#define	MIN_RADIUS 40.0f
#define MAX_RADIUS 75.0f
//#define MIN_RADIUS 3.0f
//#define MAX_RADIUS 3.0f
#define MIN_DISTANCE 150.0f
#define MAX_DISTANCE 600.0f

//boxcity
#define MIN_RADIUS 35.0f
#define MAX_RADIUS 80.0f
//#define MIN_RADIUS 5.0f
//#define MAX_RADIUS 8.0f
#define MIN_DISTANCE 50.0f
#define MAX_DISTANCE 400.0f
////
#define ambientColor float3(0.1,0.1,0.1)

#define luminanceMax 40.0f
#define GAMMA (1.1f/2.2f)

//#define luminanceMax 150.0f
//#define GAMMA (1.8f/2.2f)
//
#define DepthBias 8.0f
#define	kernelSize 2
#define shadowRange 0.32f

// Constants for use in all shaders
// camera, and other general stuff

cbuffer changesCamera: register(b0)
{
	float4x4 World; // object to world (of light?)
	float4x4 View; 
	float4x4 WorldViewProj;
	float3 cameraPosition;
	float refractiveIndexETA;
}

cbuffer changesLightCamera: register(b1)
{
	float4x4 lightWorld; // object to world (of light?)
	float4x4 lightView; 
	float4x4 lightWorldViewProj;
	float lightRefractiveIndexETA;
}

cbuffer changesPerLight : register(b4)
{
	float4x4 lightViewProj;
	float4x4 shadowToScreen;
	float4	lightPower;
	float4	lightPositionAngle;
	float4	lightDirectionDistance;
}

// sampler
SamplerState defaultSampler : register(s0)
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
	AddressW = Wrap;
};

Texture2D randomMap : register(t9);

/** Fresnel reflection coefficient by Schlick's approximation.*/
float3 ComputeFresnel(in float3 F0, in float cos_i) {
    return F0 + (float3(1.0f, 1.0f, 1.0f) - F0) * pow(1.0 - cos_i, 5);
}

//random function
float rand(float2 co){
    return saturate(frac(sin(dot(co.xy ,float2(12.9898f,78.233f))) * 43758.5453f));
}

void GetTangents(in float3 Normal, out float3 Tangent, out float3 BiTangent)
{
		        // Choose another vector not perpendicular
        Tangent = (abs(Normal.x) < 0.9f) ? float3(1, 0, 0) : float3(0, 1, 0);
        
        // Remove the part that is parallel to Z
        Tangent = Tangent - Normal * dot(Normal, Tangent);
        Tangent = normalize(Tangent);
    
        BiTangent = cross(Normal, Tangent);
		//test
	//	BiTangent = normalize(BiTangent);
}

float3 CosHemi(float3 normal, float randomE1, float randomE2)
{
    
	// Jensen's method (ported from G3D::Vector3)
    const float sin_theta = sqrt(1.0f - randomE1);
    const float cos_theta = sqrt(randomE1);
    const float phi = ( PI * 2 ) * randomE2;

    float x = cos(phi) * sin_theta;
    float y = sin(phi) * sin_theta;
    float z = cos_theta;

	float3 tangent = float3(1,0,0);
	float3 biTangent = float3(0,1,0);
	GetTangents(normal, tangent, biTangent);
	
	return (x * tangent + y * biTangent + z * normal);
};

float3 CosPowHemi(float3 reflected, float glossIndex, float randomE1, float randomE2)
{
    

	//void Random::cosPowHemi(const float k, float& x, float& y, float& z) {

    const float cos_theta = pow(abs(randomE1), 1.0f / (glossIndex + 1.0f));
    const float sin_theta = sqrt(1.0f - (cos_theta * cos_theta));
    const float phi = ( PI * 2 ) * randomE2;


    float x = cos(phi) * sin_theta;
    float y = sin(phi) * sin_theta;
    float z = cos_theta;

	float3 tangent = float3(1,0,0);
	float3 biTangent = float3(0,1,0);
	GetTangents(reflected, tangent, biTangent);
	
	     

	return (x * tangent + y * biTangent + z * reflected);
};


#endif // SHADERCONSTANTS_HLSL
