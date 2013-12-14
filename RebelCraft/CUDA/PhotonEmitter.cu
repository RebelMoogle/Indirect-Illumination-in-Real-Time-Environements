#include <optix_world.h>
#include "helpers.h"
#include "..\\random.h"
#include "..\\OptixTypes.h"

using namespace optix;

// Application input.
rtDeclareVariable(float,			scene_epsilon, , );
rtDeclareVariable(rtObject,			top_object, , );
rtBuffer<uint2, 1>					randomSeeds;
rtBuffer<PhotonInitialBounce, 1>	photonInitialBuffer;
rtBuffer<PhotonRecord, 1>			photonResultBuffer;


// Intrinsic input.
rtDeclareVariable(uint, launch_index, rtLaunchIndex, );


RT_PROGRAM void photon_emitter()
{
	PhotonInitialBounce initialPhoton = photonInitialBuffer[launch_index];
	// Fetch random seeds.
	uint2  seed = make_uint2(initialPhoton.texCoord.x, initialPhoton.texCoord.y);//randomSeeds[launch_index];

	//	 Check if this photon has power
	//if ((initialPhoton.power.x + initialPhoton.power.y + initialPhoton.power.z)/3 < 0.01) return;

	float3 ray_origin = xyz(initialPhoton.position);
	float3 ray_direction = initialPhoton.direction;	
	
	optix::Ray ray(ray_origin, ray_direction, RayType_PhotonRay, scene_epsilon );

	// TODO: clear photon records before splatting!	
	// Initialize our photons
	/*for(unsigned int i = 0; i < DIFFUSE_PHOTONS_PER_RAY; ++i) {
		//prd.sample[i] = rnd_from_uint2(photon_rnd_seeds[launch_index]);
		photonResultBuffer[i+(launch_index) * DIFFUSE_PHOTONS_PER_RAY].incomingPower = make_float3(0.0f);
	}*/

	PhotonPRD prd;

	prd.power = xyz(initialPhoton.power);
	prd.sample = seed;
	prd.numDeposits = 0;
	prd.rayDepth = 0;
	//prd.pathDensity = initialPhoton.pathDensity;
	prd.etaIncoming = initialPhoton.texCoord.z;
	prd.travelDistance = initialPhoton.travelDistance;
	rtTrace( top_object, ray, prd );

	//// ### DEBUG ##

	//PhotonRecord dbgOutput;

	//dbgOutput.position = xyz(initialPhoton.position);
	//dbgOutput.normal = xyz(initialPhoton.normal);      // Pack this into 4 bytes
	//dbgOutput.incomingPower = xyz(initialPhoton.power);
	//dbgOutput.incomingDirection= initialPhoton.direction;
	//dbgOutput.travelDistance = initialPhoton.travelDistance;

	//photonResultBuffer[(launch_index) * DIFFUSE_PHOTONS_PER_RAY] = dbgOutput; 
}
