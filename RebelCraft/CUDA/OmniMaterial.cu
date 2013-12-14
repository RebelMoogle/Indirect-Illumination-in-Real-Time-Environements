#include <optix_world.h>
#include <optixu/optixu_math_namespace.h>
#include "../OptixTypes.h"
#include "helpers.h"
#include "../random.h"

using namespace optix;

// Application input.
rtDeclareVariable(float3, Diffuse, , );
rtDeclareVariable(float4, Specular, , );
rtDeclareVariable(float4, Transmissive, , );
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

rtDeclareVariable(PhotonPRD,		hit_prd,	  rtPayload, );
rtBuffer<PhotonRecord, 1>			photonResultBuffer;
rtDeclareVariable(uint,				lin_launch_index, rtLaunchIndex, );

__device__ bool Scatter(	uint2& seed, 
							float roulette,
							float3 normal, 
							float3 incomingDir, 
							float3 powerIncoming, float3 powerReflective, 
							float3 powerSpecular, float3 powerTransmissive,
							float etaMaterial, float specularExponent,
							float3& outgoingDir, float3& powerOutgoing, float& etaOutgoing) //, float& pathDensity)
{
	float ETA = hit_prd.etaIncoming;

	float powerReflectiveMean = (powerReflective.x + powerReflective.y + powerReflective.z) /3;

	float rouletteScatter = roulette - (powerReflectiveMean+0.3f);

	// diffuse scatter
	if( rouletteScatter <= 0.0f)
	{
		// TODO: multiply with material reflection data
		powerOutgoing = powerIncoming * powerReflective / powerReflectiveMean; /// * etaMaterial;
		//get the a diffuse reflection by using a normal hemisphere reflection ("random")

		// create tangents
		float3 U, V, W;
		createONB(normal, U, V, W);
		sampleUnitHemisphere(rnd_from_uint2(seed), U, V, W, outgoingDir);

		// This is a low-density path because
		// it scattered very diffusely
		//pathDensity *= powerReflectiveMean * 0.01;
		etaOutgoing = ETA; // lightETA is usually 1.0, is the incoming refractive index.

		//drop photon
		float3 hit_point = ray.origin + t_hit*ray.direction;
		PhotonRecord& rec = photonResultBuffer[lin_launch_index * DIFFUSE_PHOTONS_PER_RAY + hit_prd.numDeposits];
		rec.position = hit_point;
		rec.normal = normal; //world_geometric_normal; 
		rec.incomingDirection = ray.direction;
		rec.incomingPower = hit_prd.power;
		rec.travelDistance = hit_prd.travelDistance;
		//rec.pathDensity = pathDensity;
		hit_prd.numDeposits++;

		return true;
	}

	float powerSpecularMean = (powerSpecular.x + powerSpecular.y + powerSpecular.z) / 3;
	rouletteScatter = rouletteScatter - powerSpecularMean;
	if (rouletteScatter  <= 0.0) 
	{
		if (specularExponent < 1.0) 
		{
			// Glossy specular
			powerOutgoing = powerIncoming * powerSpecular / powerSpecularMean;
			outgoingDir = reflect(incomingDir, normal);//cosHemi(normal, random.g, random.b);

			float gloss = specularExponent * 127.0 + 1.0; //TODO: get rid of magic numbers
			//get the a specular reflection by using a squared hemisphere reflection around the reflected vector ("random")
			float3 U, V, W;
			createONB(normal, U, V, W);
			outgoingDir = sample_phong_lobe(rnd_from_uint2(seed), gloss, U, V, W);

			//outgoingDir = CosPowHemi(outgoingDir, gloss, random.g, random.b);

			// Medium density
			//pathDensity *= powerSpecularMean * 0.1;

			//drop photon
			float3 hit_point = ray.origin + t_hit*ray.direction;
			PhotonRecord& rec = photonResultBuffer[lin_launch_index * DIFFUSE_PHOTONS_PER_RAY + hit_prd.numDeposits];
			rec.position = hit_point;
			rec.normal = normal; //world_geometric_normal; 
			rec.incomingDirection = ray.direction;
			rec.incomingPower = hit_prd.power;
			//rec.pathDensity = pathDensity;
			rec.travelDistance = hit_prd.travelDistance;
			hit_prd.numDeposits++;

		} 
		else 
		{
			// Mirror specular
			powerOutgoing = powerIncoming * powerSpecular  / powerSpecularMean;

			// might be opposite?
			outgoingDir = reflect(incomingDir, normal);

			// High density
			//pathDensity *= powerSpecularMean;
		}
		etaOutgoing = ETA;

		return true;
	}

	//test, no transmissive yet!
	return false;

	float powerTransmitMean = (powerTransmissive.x + powerTransmissive.y + powerTransmissive.z) / 3;
	rouletteScatter -= powerTransmitMean;
	// Must be the case that this was transmissive
	if (rouletteScatter <= 0.0) 
	{         
		powerOutgoing = powerIncoming * powerTransmissive / powerTransmitMean;
		//might also be opposite
		 bool NoTotalReflection = refract(outgoingDir, incomingDir, normal, ETA / etaMaterial);

		// outgoingDir is zero on total internal refraction
		if (!NoTotalReflection) 
		{
			//pathDensity = powerTransmitMean;
			etaOutgoing = etaMaterial;
			return true;
		} 
		else 
		{
			powerOutgoing = powerIncoming * powerSpecular  / powerSpecularMean;
			outgoingDir = reflect(-incomingDir, normal);
			//pathDensity = powerSpecularMean;
			etaOutgoing = ETA;
			return true;
		}

	}

	return false;
}


RT_PROGRAM void PhotonRay_closest_hit()
{
	//	 Check if this is a light source
//	if (Diffuse.x == 0 && Diffuse.y == 0 && Diffuse.z == 0) return;

	float3 world_shading_normal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );
	float3 world_geometric_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );
	float3 ffnormal     = faceforward( world_shading_normal, -ray.direction, world_geometric_normal );

	float3 hit_point = ray.origin + t_hit*ray.direction;

	// get distance and add to traveldistance
	hit_prd.travelDistance += t_hit;


	// ------	scatter stuff
	float cosAngleIn = max(0.0f, dot(ray.direction, ffnormal));

	bool incident = cosAngleIn < 0;
	float refractionIndex = Transmissive.w;

	/*switch(hit_prd.numDeposits)
	{
	case 0:
		hit_prd.power = make_float3(1,0,0);
		break;
	case 1:
		hit_prd.power = make_float3(0,1,0);
		break;
	case 2:
		hit_prd.power = make_float3(0,0,1);
		break;
	case 3:
		hit_prd.power = make_float3(0,1,1);
		break;
	case 4:
		hit_prd.power = make_float3(1,1,0);
		break;
	case 5:
		hit_prd.power = make_float3(1,0,1);
		break;
	default:
		hit_prd.power = make_float3(1,1,1);
		break;

	}*/

	// instead of etaOutgoing ( should be carried along by photon) // instead of lightEta in scatter
	if(incident)
	{
		refractionIndex = refractionIndex / 1.0f;
	}
	else
	{
		refractionIndex = 1.0f / refractionIndex;
	}
	float3 powerSpecular = fresnel_schlick(cosAngleIn, 5.0f, xyz(Specular), make_float3(1.0f));
	float3 powerTransmissive = xyz(Transmissive);

	uint2&   seed     = hit_prd.sample;
	float roulette = rnd_from_uint2(seed).x;
	
	float3 powerTotal = Diffuse + powerSpecular + powerTransmissive;

	/*if(roulette <= (powerTotal.x + powerTotal.y + powerTotal.z)/3)
	{
		// drop photon
		PhotonRecord& rec = photonResultBuffer[lin_launch_index * DIFFUSE_PHOTONS_PER_RAY + hit_prd.numDeposits];
		rec.position = hit_point;
		rec.normal = world_geometric_normal; //ffnormal;
		rec.incomingDirection = ray.direction;
		rec.incomingPower = hit_prd.power;
		//rec.pathDensity = hit_prd.pathDensity;
		rec.travelDistance = hit_prd.travelDistance;
		hit_prd.numDeposits++;

		return; //Kill it
	}*/

	float3 outgoingDir = make_float3(0,0,0);
	float3 outgoingPower = make_float3(0,0,0);
	float  outgoingETA = 0.0f;
	//float  pathDensity;
	if(!Scatter(seed, roulette, ffnormal, ray.direction, hit_prd.power, Diffuse, powerSpecular, powerTransmissive, Transmissive.w, Specular.w, outgoingDir, outgoingPower, outgoingETA)) //, pathDensity))
	{
		//default diffuse on kill
		// drop photon
			PhotonRecord& rec = photonResultBuffer[lin_launch_index * DIFFUSE_PHOTONS_PER_RAY + hit_prd.numDeposits];
			rec.position = hit_point;
			rec.normal = world_geometric_normal; //ffnormal;
			rec.incomingDirection = ray.direction;
			rec.incomingPower = hit_prd.power;
			//rec.pathDensity = hit_prd.pathDensity;
			rec.travelDistance = hit_prd.travelDistance;
			hit_prd.numDeposits++;;

			return;
	}
	
	hit_prd.rayDepth++;
	if ( hit_prd.numDeposits >= DIFFUSE_PHOTONS_PER_RAY) //|| hit_prd.rayDepth >= MAX_PHOTON_DEPTH || outgoingETA == 0.0f)
		return;

	//hit_prd.energy = Diffuse * hit_prd.energy;
    hit_prd.power = outgoingPower; //may lose saturation 
	//hit_prd.pathDensity = pathDensity;
	hit_prd.etaIncoming = outgoingETA;

	optix::Ray new_ray( hit_point, outgoingDir, RayType_PhotonRay, scene_epsilon );
	rtTrace(top_object, new_ray, hit_prd);
}