// Optix Types
#pragma once

// Number of ray types
#define RAY_TYPE_COUNT 2

// Enumeration of ray types
enum ERayType
{
	RayType_DirectDiffuse = 0,	// computes direct diffuse illumination
	RayType_PhotonRay = 1,		// a photon
};

// Number of entry types
#define ENTRY_TYPE_COUNT 3
// Enumeration of entry types
enum EEntryType
{
	EntryType_OptixViewerCamera = 0,
	EntryType_StaticCamera = 1,
	EntryType_PhotonEmitter = 2
};

// Maximum number of PhotonRecords a photon can drop.
#define DIFFUSE_PHOTONS_PER_RAY 3
// Maximum number of indirections-1 per photon.
#define MAX_PHOTON_DEPTH 4

#define RefractIdx_Vacuum 1.0f
#define RefractIdx_Air 1.000292f
#define RefractIdx_Ice 1.31f
#define RefractIdx_Water 1.33f
#define RefractIdx_FusedQuartz 1.46f
#define RefractIdx_Glass 2.0f
#define RefractIdx_Diamond 2.42f

// Data that is carried by each photon.
struct PhotonPRD
{
	public:
		optix::float3 power;		// energy of the photon (later converted to color)
		optix::uint2  sample;		// random seed
		optix::uint   numDeposits;	// Number of photons emitted.
		optix::uint   rayDepth;	    // Number of ray indirections.
		float		  travelDistance;
		float		  etaIncoming; // comes from texcoord.z in the initial bounce
};

struct PhotonRecord
{
	optix::float3	position;
	optix::float3	normal;      
	optix::float3	incomingPower;
	optix::float3	incomingDirection;
	float			travelDistance;
};

struct PhotonInitialBounce
{
	optix::float4 position;
	optix::float4 texCoord; // will also hold etaOutgoing in texcoord.z
	optix::float4 power;	
	optix::float3 direction;
	float  travelDistance;
	optix::float4 normal; // don't need normal as input!		
};

/* 
Optix mag das nicht. 
struct PhotonInitialBounce
{
optix::float4 position;
optix::float2 texCoord; -> float4 findet optix toll.
optix::float4 power;	
optix::float3 direction;
float  pathDensity;
optix::float4 normal; // don't need normal as input!		
};

*/