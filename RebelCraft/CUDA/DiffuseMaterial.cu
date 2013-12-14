#include <optix_world.h>
#include <optixu/optixu_math_namespace.h>
#include "../OptixTypes.h"
#include "helpers.h"
//#include "random.h"

using namespace optix;

// Application input.
rtDeclareVariable(float3, Diffuse, , );
rtDeclareVariable(float3, LightPosition, , ) = {278.0f, 530.7f, 279.5f};
rtDeclareVariable(rtObject,      top_object, , );
rtDeclareVariable(float,         scene_epsilon, , ) = 0.01f;

// Intrinsic input.
rtDeclareVariable(float3,		hit_data,	  rtPayload, );

rtDeclareVariable(optix::Ray,	ray,          rtCurrentRay, );
rtDeclareVariable(float,		t_hit,        rtIntersectionDistance, );
rtDeclareVariable(uint2,		launch_index, rtLaunchIndex, );

// Input from Intersection program.
rtDeclareVariable(float3, shading_normal,		attribute shading_normal, );
rtDeclareVariable(float3, geometric_normal,		attribute geometric_normal, );


// Returns the hit color (does simple diffuse shading)
RT_PROGRAM void DirectDiffuse_closest_hit()
{
	float3 world_shading_normal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );
	float3 world_geometric_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );
	float3 ffnormal     = faceforward( world_shading_normal, -ray.direction, world_geometric_normal );
	float3 hit_point    = ray.origin + t_hit*ray.direction;

	float3 L = LightPosition - hit_point;
	float3 N = normalize(ffnormal);

	hit_data = dot(N,normalize(L)) * Diffuse;
}
/*
rtDeclareVariable(PhotonPRD,		hit_record,	  rtPayload, );
rtBuffer<PhotonRecord, 1>           PhotonMap;
//rtDeclareVariable(uint,		lin_launch_index, rtLaunchIndex, );

RT_PROGRAM void PhotonRay_closest_hit()
{
	// Check if this is a light source
	if (Diffuse.x == 0 && Diffuse.y == 0 && Diffuse.z == 0) return;

	float3 world_shading_normal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );
	float3 world_geometric_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );
	float3 ffnormal     = faceforward( world_shading_normal, -ray.direction, world_geometric_normal );

	float3 hit_point = ray.origin + t_hit*ray.direction;

	// ------	

//	uint2   seed     = photon_rnd_seeds[lin_launch_index];		
//	photon_rnd_seeds[lin_launch_index] = make_uint2(lcg2(seed.x), lcg2(seed.y));
//	float roulette = rnd(seed.x);
//	if( hit_record.ray_depth > 0 ) 	


	uint2&   seed     = hit_record.sample; //photon_rnd_seeds[lin_launch_index];		
//	photon_rnd_seeds[lin_launch_index] = make_uint2(lcg2(seed.x), lcg2(seed.y));
	float roulette = rnd_from_uint2(seed).x;
	
	PhotonRecord& rec = PhotonMap[hit_record.ray_index * DIFFUSE_PHOTONS_PER_RAY + hit_record.num_deposits];
	rec.position = hit_point;
	rec.normal = world_geometric_normal; //ffnormal;
	rec.ray_dir = ray.direction;
	rec.energy = hit_record.energy;
	hit_record.num_deposits++;

	if (roulette < 0.2) // Kill photon.		
		return;	

	hit_record.ray_depth++;
	if ( hit_record.num_deposits >= DIFFUSE_PHOTONS_PER_RAY || hit_record.ray_depth >= MAX_PHOTON_DEPTH)
		return;

	//hit_record.energy = Diffuse * hit_record.energy;
    hit_record.energy = Diffuse * hit_record.energy * 3.0f / (Diffuse.x + Diffuse.y + Diffuse.z); 
    float3 U, V, W;
    createONB(ffnormal, U, V, W);
	float3 new_ray_dir;
    sampleUnitHemisphere(rnd_from_uint2(seed), U, V, W, new_ray_dir);

	optix::Ray new_ray( hit_point, new_ray_dir, RayType_PhotonRay, scene_epsilon );
	rtTrace(top_object, new_ray, hit_record);
}
*/