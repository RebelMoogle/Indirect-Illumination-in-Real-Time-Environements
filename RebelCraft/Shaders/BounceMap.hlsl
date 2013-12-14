// Copyright John McLaughlin
// have fun with it, and credit me.
// sending me a note you used this, would be awesome. Would be great to know people actually use this!

#ifndef BOUNCEMAP_HLSL
#define BOUNCEMAP_HLSL

#include "ShaderConstants.hlsl"

// set all the GBuffer maps (as input or still available)
// use constant buffers! ( different ones for changing (gbuffer) and constant (bouncemap mask, etc)
// create a method that just renders the bouncemap when calculating the G-Buffer?, can be called from the other shader

//#### buffers and constants pixel shader ####
// (world space) surface point (get from depth map)
// (world space) incoming light vector w_i ( points towards light)

tbuffer textures
{
	//Gbuffer: specular, albedo, transmittance (rgb: transmittance, a: eta_material), normal
	Texture2D<float4>	specularMap		: register(t4);
	Texture2D<float4>	diffuseMap		: register(t5);
	Texture2D<float4>	transmittanceMap: register(t6);
	Texture2D<float4>	normalMap		: register(t7);
	Texture2D<float4>	wsPositionMap	: register(t8);
}

cbuffer bounceInfo : register(b3)
{
	float killPercentage;
}

// buffer for output maps 
struct BounceMap
{
	// rgb: power; a: Distance scaled by cosine correction factor
	float4 powerDistance : SV_Target0;

	// rgb: outgoing direction vector
	// a: path density
	float4 directionDensity : SV_Target1;
};

//### in- and output structs
struct VS_INPUT
{
	float3 Pos          : POSITION;       
	float2 Tex          : TEXCOORD0;      
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

struct GS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

struct PHOTON
{
	float4 position				: SV_POSITION;
	float4 texCoord				: TEXCOORD;
	float4 outgoingPower		: POWER;
	float3 outgoingDirection	: DIRECTION;
	float  travelDistance		: DISTANCE;
	float4 surfaceNormal		: NORMAL;
};

//### Scatter Function ###
//	in  random, normal, incoming vector, Incident Power, Lambertian, Fresnel (specular power), Transmited power, eta_material, spec_EXPonent, 
//	out outgoing vector, outgoing power, eta_outgoing, pathDensity

void Scatter(	in float3 random, in float3 normal, in float3 incomingDir, in float3 powerIncoming, in float3 powerReflective, in float3 powerSpecular, in float3 powerTransmit, in float etaMaterial, in float specularExponent, 
				out float3 outgoingDir, out float3 powerOutgoing, out float etaOutgoing)
{
	// assume all lights in air
	// set this per light, since lights could be in water or glass. (it'd be more fun this way. :) )

	// scatter options:
	// lambertian scatter (cmp random to mean of Lambertian Power)
	// glossy or mirror specular (cmp random to mean of specular Power)
	// refraction (Power Transmissive, incoming vector and eta_material)
	// incoming eta: lightETA
	float lightETA = 1.0f;
	bool done = false;
	outgoingDir = float3(0,0,0);
	powerOutgoing = float3(0,0,0);
	etaOutgoing = 0.0f;

	float powerReflectiveMean = (powerReflective.r + powerReflective.g + powerReflective.b) / 3;
	float roulette = random.r - (powerReflectiveMean + 0.3f);
	//random.r = random.r - powerReflectiveMean;

	// DIFFUSE scatter
	//[branch]if(random.r <= 0.0f) // < 0.0f) // is the same as below but DOESN'T work....
	[branch]if(roulette <= 0.0f) // < 0.0f)
	{
		powerOutgoing = powerIncoming * (powerReflective / powerReflectiveMean);
		//get the a diffuse reflection by using a normal hemisphere reflection ("random")
		outgoingDir = CosHemi(normal, random.g, random.b);

		// This is a low-density path because
		// it scattered very diffusely
		//pathDensity = powerReflectiveMean * 0.01;
		etaOutgoing = lightETA; // lightETA is usually 1.0, is the incoming refractive index.
		return;
		
	}
	
	//outgoingDir = float3(1,0,0);
	//return;

	//SPECULAR
	float powerSpecularMean = (powerSpecular.r + powerSpecular.g + powerSpecular.b) / 3;
	roulette = roulette - powerSpecularMean;
	[branch]if(roulette <= 0.0f)
	{
		if (specularExponent < 1.0) 
		{
			// Glossy specular
			powerOutgoing = powerIncoming * powerSpecular / powerSpecularMean;
			outgoingDir = reflect(-incomingDir, normal);//cosHemi(normal, random.g, random.b);

			float gloss = specularExponent * 127.0 + 1.0; //TODO: get rid of magic numbers
			//get the a specular reflection by using a squared hemisphere reflection around the reflected vector ("random")
			outgoingDir = CosPowHemi(outgoingDir, gloss, random.g, random.b);

			////got no real random, ain't gonna change anyway.
			//float gloss = specularExponent * 127.0 + 1.0; //TODO: get rid of magic numbers
			//float3 combinedDir;
			//do {
				//outgoingDir = cosHemi(normal, random.g, random.b);
				//combinedDir = normalize(incomingDir + outgoingDir);
			//} while (((random.r+random.g+random.b) / 3) > pow(dot(combinedDir, normal), gloss));
	
			// Medium density
			//pathDensity = powerSpecularMean * 0.1;

		} 
		else 
		{
			// Mirror specular
			powerOutgoing = powerIncoming * powerSpecular  / powerSpecularMean;

			// might be opposite?
			outgoingDir = reflect(-incomingDir, normal);

			// High density
			//pathDensity = powerSpecularMean;
		}
		etaOutgoing = lightETA;
		return;
	}

	//TEST no transmissive yet!
	return;

	// TRANSMISSIVE
	float powerTransmitMean = (powerTransmit.x + powerTransmit.y + powerTransmit.z) / 3;
	roulette -= powerTransmitMean;
	// Must be the case that this was transmissive
	[branch]if(roulette <= 0.0f) 
	{         
		powerOutgoing = powerIncoming * powerTransmit / powerTransmitMean;
		//might also be opposite
		outgoingDir = refract(-incomingDir, normal, lightETA / etaMaterial);

		// outgoingDir is zero on total internal refraction
		if (dot(outgoingDir, outgoingDir) != 0.0) 
		{
			return;
		} 
		else 
		{
			//pathDensity = powerTransmitMean;
			etaOutgoing = etaMaterial;
			return;
		}
	}

	//// DEFAULT: DIFFUSE:
	//powerOutgoing = powerIncoming * (powerReflective / powerReflectiveMean);
	////get the a diffuse reflection by using a normal hemisphere reflection ("random")
	//outgoingDir = CosHemi(normal, random.g, random.b);
//
	//// This is a low-density path because
	//// it scattered very diffusely
	////pathDensity = powerReflectiveMean * 0.01;
	//etaOutgoing = lightETA; // lightETA is usually 1.0, is the incoming refractive index.
	return;
}

/////////// ############ Photon Buffer Shaders ############# ///////////////
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
GS_INPUT BounceBufferVS( VS_INPUT input )
{
	GS_INPUT output = (GS_INPUT)0;
	
	output.Pos = float4(input.Pos, 1);
	output.Tex = input.Tex;
	
	return output;
}


//### Geometry shader ###

[maxvertexcount(1)]
void BounceBufferGS(point GS_INPUT input[1], inout PointStream<PHOTON> outputStream)
{

	
	int2 dimensions;
	wsPositionMap.GetDimensions(dimensions.x, dimensions.y);
	float3 position = wsPositionMap.SampleLevel(defaultSampler, input[0].Tex, 0);//.Load(int3(dimensions.x * input[0].Tex.x, dimensions.y * input[0].Tex.y, 0)).rgb;

	// check if in spotlight
	float3 lightToWSPosition = normalize(position - lightPositionAngle.xyz);
	float lightToPosAngle = dot(lightDirectionDistance.xyz, lightToWSPosition);
	// wsPosition is outside of spotlightCone
	if(lightPositionAngle.w > 0.0f && cos(lightPositionAngle.w * 0.5f) > lightToPosAngle)
		return;

	PHOTON output;
	output.outgoingPower = float4(0,0,0,1);
	output.texCoord = float4(input[0].Tex, 0, 1);
	output.position = float4(0,0,0,1);
	output.surfaceNormal = float4(0,0,0,1);
	output.outgoingDirection = float3(0,0,0);
	output.travelDistance = 1.0;

	//normal
	//normalMap.GetDimensions(dimensions.x, dimensions.y);
	float3 normal = normalMap.SampleLevel(defaultSampler, input[0].Tex, 0);//.Load(int3(dimensions.x * input[0].Tex.x, dimensions.y * input[0].Tex.y, 0)).rgb;  // normalMap.SampleLevel(defaultSampler, input[0].Tex, 0).rgb;
	normal = normalize(normal);
	// check if hitting background??
	if(dot(normal, normal) < 0.5) // dot(n, n) gives |n|^2
	{
		return;
	}
	// need diffuse to calculate
	float3 powerReflective	= diffuseMap.SampleLevel(defaultSampler, input[0].Tex, 0).rgb;//Load(int3(dimensions.x * input[0].Tex.x, dimensions.y * input[0].Tex.y, 0)).rgb;
	float4 specular			= specularMap.SampleLevel(defaultSampler, input[0].Tex, 0);//.Load(int3(dimensions.x * input[0].Tex.x, dimensions.y * input[0].Tex.y, 0));
	//what is F0?
	float3 F0 = specular.rgb;
	float specularExponent = specular.a;

	float4 transmit = transmittanceMap.SampleLevel(defaultSampler, input[0].Tex, 0);//.Load(int3(dimensions.x * input[0].Tex.x, dimensions.y * input[0].Tex.y, 0));
	float etaMaterial = max(1.0, transmit.a); //at least 1.0

	
	// ######## //

	//NOTE: not sure if correct
	float3 incomingDir = float3(0,0,0);
	float incomingDistance = 0.0f;
	if(lightPositionAngle.w > 0.0f)
	{
		incomingDir = lightPositionAngle.xyz - position.xyz;
		incomingDistance = length(incomingDir);
		incomingDir = normalize(incomingDir);
	}
	else
	{
		// should be normalized already
		incomingDir = normalize(-lightDirectionDistance.xyz);
	}

	
	

	// cosine of angle of incidence
	float cosAngleIn = max(0.0, dot(incomingDir, normal));


	// ### Fresnel reflection coefficient
	// TODO compute fresnel (need specular)
	// apply fresnel
	float3 powerSpecular = ComputeFresnel(F0, cosAngleIn);
	float3 powerTransmit = transmit.rgb; // * (float3(1.0) - F);

	// TODO: create random variable
	//randomMap.GetDimensions(dimensions.x, dimensions.y);
	//float3 random = randomMap.Load(int3(dimensions.x * input[0].Tex.x, dimensions.y * input[0].Tex.y, 0)).rgb; //RANDOM(); //randomMap is a greyscale noise texture. 
	float3 random = float3( rand(input[0].Tex), rand(input[0].Tex.yx), rand(input[0].Tex.xy) );
	//if randomMap is not a greyscale texture just do: sum()/3   ;)

	//kill to fullfill quota
	if(random.r <= killPercentage)
		return;


	//float testrandom = (random.r + random.g + random.b) / 3;
	//output.outgoingPower = float4(random,1);


	float3 powerTotal = powerReflective + powerSpecular.rgb + powerTransmit;

	// Random variable that decides what kind of scattering occurs
		// Multiply by 3.0, instead of dividing right side, to get average.
	// discard immediately if no scatter
	//if(random.r*3 <= (powerTotal.x + powerTotal.y + powerTotal.z)) //no scatter
		//return;
//
	// cosine of the angle between the look vector and w_i (world space incoming light vector)
		
	//float cosAngleLookIn = dot(-incomingDir, lightDirectionDistance.xyz); // should work now: direction wasn't normalized at first

	//incident power, adjust to projective space of bounce map, i.e. decreasing acceptance probability with distance from view axis
	//TODO: do this with a mask!
	float3 powerIncoming = lightPower.rgb; // * cosAngleLookIn;

	float3 outgoingDir = float3(0,0,0);

	float3 powerOutgoing = float3(0,0,0);

	float travelDistance = 1.0;
	float etaOutgoing = 1.0;

	//void Scatter(	in float3 random, in float3 normal, in float3 incomingDir, in float3 powerIncoming, in float3 powerReflective, in float3 powerSpecular, in float3 powerTransmit, in float etaMaterial, in float specularExponent, 
	//			out float3 outgoingDir, out float3 powerOutgoing, out float etaOutgoing, out float pathDensity)
	Scatter(random, normal, incomingDir, powerIncoming, powerReflective, powerSpecular.rgb, powerTransmit, etaMaterial, specularExponent, outgoingDir, powerOutgoing, etaOutgoing); //, pathDensity);
	
	//// write out new photon
	//output.powerDistance.rgb = powerOutgoing;//float3(cosAngleLookIn, 0,0);//powerOutgoing;
	//output.powerDistance.a = incomingDistance * cosAngleLookIn; // * invCos_fov
	//
	//output.directionDensity.rgb = outgoingDir;
	//output.directionDensity.a = etaOutgoing;
	if(length(powerOutgoing) <= 0)
		return;


	output.outgoingPower = float4(powerOutgoing,1);//float4(powerOutgoing,1);
	output.position = float4(position + normal * 0.1,1);
	output.surfaceNormal = float4(normal,0);
	output.outgoingDirection = outgoingDir;
	output.travelDistance = 0.0f; //these photons will not be drawn...//incomingDistance;
	output.texCoord.z =  etaOutgoing;

	//output.outgoingPower = float4(output.texCoord.w / 1000, 0, 0, 1);

	//if(!lightPositionAngle.w < 0.0f)
		//pathDensity * 10.0f;

	//output.surfaceNormal = float4(dimensions.x * input[0].Tex.x / 1024.0f, 0, 0,1);

	//output.surfaceNormal = float4(powerReflective, 1);

	// test
	//output.outgoingDirection = incomingDir;
	//output.outgoingPower = lightPower;
	//output.position = lightPosition;

	outputStream.Append(output);
}


// ### old photon bounce map shaders ###
// renders into RenderTargets instead of stream output


#endif // BOUNCEMAP_HLSL
