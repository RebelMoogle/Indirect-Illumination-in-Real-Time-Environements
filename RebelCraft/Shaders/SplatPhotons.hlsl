// Copyright John McLaughlin
// have fun with it, and credit me.
// sending me a note you used this, would be awesome. Would be great to know people actually use this!
// ##################################################
// Filename: SplatPhotons.hlsl
//
// splats photons from a given photon buffer in the specified format
// requires:
//		- Camera GBuffer
//		- BounceMap SRVs
//		- Light View Projection Matrix
// ##################################################


#ifndef SPLATPHOTONS_HLSL
#define SPLATPHOTONS_HLSL

#include "ShaderConstants.hlsl"

tbuffer textures
{
	//Gbuffer: specular, albedo, transmittance (rgb: transmittance, a: eta_material), normal
	Texture2D<float4>	specularMap		: register(t4);
	Texture2D<float4>	diffuseMap		: register(t5);
	Texture2D<float4>	transmittanceMap		: register(t6);
	Texture2D<float4>	normalMap		: register(t7);
	Texture2D<float4>	wsPositionMap	: register(t8);
}

// uses Vertex and Geometry shader from TextureToScreen.hlsl

struct PHOTON_SPLAT
{
	float4 position				: SV_POSITION;
	float3 power				: POWER;
	float  travelDistance		: DISTANCE;
	float3 direction			: DIRECTION;
	float3 normal				: NORMAL;
	float4 photonPosition		: PHOTONPOSITION;
	float4 clippingPosition		: CLIPPINGPOSITION;
	float4 worldPosition		: WORLDPOSITION;
};

struct SplatVertexInput
{
	float3	position	: POSITION;
	float3	normal		: NORMAL;      
	float3	power		: POWER;
	float3	direction	: DIRECTION;
	float	travelDistance : DISTANCE;
};

float3 PhongShade(float3 position, float3 normal, float3 kDiffuse, float3 kSpecular, float specularExponent, float3 power, float3 viewDir, float3 lightDir)
{
	float3 outgoingRadiance = float3(0.0f, 0.0f, 0.0f);

	float3	halfLightView = normalize(-viewDir - lightDir);
	float	cosThetaH = saturate(dot(normal, halfLightView));
	float	cosThetaI = saturate(dot(normal, -lightDir));
	outgoingRadiance = (kDiffuse + kSpecular * pow(cosThetaH, specularExponent)) * power * cosThetaI;

	return outgoingRadiance;
}

// ### Vertex Shader ###

PHOTON_SPLAT SplatVertexShader(SplatVertexInput input)
{
	PHOTON_SPLAT output = (PHOTON_SPLAT)0;

	output.position = float4(input.position,1);
	output.power = input.power;
	output.direction = input.direction;
	output.normal = mul(input.normal.xyz, (float3x3)World);
	output.photonPosition = float4(input.position,1.0f);
	output.travelDistance = input.travelDistance;

	return output;
};


[maxvertexcount(12)]
void SplatGeometryShader(point PHOTON_SPLAT input[1], inout TriangleStream<PHOTON_SPLAT> outputStream)
{
	
	if(length(input[0].power.rgb) <= 0.1 || length(input[0].direction.xyz) < 1)
		return;
	//view direction
	float3 viewDir = normalize(input[0].position.xyz - cameraPosition);

	if(dot(viewDir, input[0].normal.xyz) > 0.5f)
		return;

	//// DEBUG TEST
	//if(input[0].travelDistance > 600.0f)
	 //return;

	// init with zero values
	PHOTON_SPLAT output = (PHOTON_SPLAT)0;
	output.position		= float4(0,0,0,1);
	output.power		= float3(0,0,0);
	output.direction	= float3(0,1,0);
	output.travelDistance	= 0.0;
	output.normal	= float3(0,1,0);

	// ## create splats around photon
	// radius dependent on path density
	float radius = smoothstep(MIN_DISTANCE, MAX_DISTANCE, input[0].travelDistance) * (MAX_RADIUS - MIN_RADIUS) + MIN_RADIUS;
	//float radius = smoothstep(MIN_DISTANCE, MAX_DISTANCE, MAX_DISTANCE * 0.5) * (MAX_RADIUS - MIN_RADIUS) + MIN_RADIUS;


	//float ellipsoidRadius = radius * ELLIPSOID_FACTOR;
	// save distance/ radius in vertices
	float3 normal = normalize(input[0].normal.xyz);
	// compute orthogonal vectors to create the quad with
	float3 orthoA = cross(normal, float3(0,1,0));
	if(abs(orthoA.x) < 0.001f && abs(orthoA.y) < 0.001f && abs(orthoA.z) < 0.001f)
		orthoA = cross(normal, float3(1,0,0));
	orthoA = normalize(orthoA);
	float3 orthoB = cross(orthoA, normal);


	//stays same for every vertex on quad
	output.power = input[0].power; // * (1 - smoothstep(MIN_DISTANCE, MAX_DISTANCE, input[0].travelDistance) );//input[0].power;

	output.direction = input[0].direction;
	output.travelDistance = radius;
	output.normal = input[0].normal;
	output.photonPosition = input[0].photonPosition;

	//check if photon is visible
	{
		float4 clipPos = mul(input[0].photonPosition, WorldViewProj);
		float2 texCoord = ((clipPos.xy/ clipPos.w) * 0.5f) + 0.5f; // screen coords are [-1, 1]
		texCoord.y = 1.0f - texCoord.y;
		float3 wsPosition = wsPositionMap.SampleLevel(defaultSampler, texCoord, 0).xyz;

		// the distance to the world position is greate than our radius, the photon should not be drawn!
		if(distance(wsPosition, input[0].photonPosition.xyz) >= radius)
			return;
	}

	////MID
	//output.position = mul(input[0].position, WorldViewProj);
	//PHOTON_SPLAT mid = output;
	
	//todo scale with near and farplane (nearer = smaller)
		
	//## TOP FACE
	float3 facePosition = input[0].position.xyz + output.normal * radius;
	PHOTON_SPLAT TopUpperLeft	= (PHOTON_SPLAT)0;
	PHOTON_SPLAT TopUpperRight	= (PHOTON_SPLAT)0;
	PHOTON_SPLAT TopLowerLeft	= (PHOTON_SPLAT)0;
	PHOTON_SPLAT TopLowerRight	= (PHOTON_SPLAT)0;

	//UPPER LEFT (-RX, RY)
	output.position = float4(facePosition + (orthoA * (-radius)) + (orthoB * (radius)), 1);
	output.worldPosition = output.position;	
	output.position = mul(output.position, WorldViewProj);
	output.clippingPosition = output.position;
	TopUpperLeft = output;

	// UPPER RIGHT ( RX, RY)
	output.position = float4(facePosition + (orthoA * (radius)) + (orthoB * (radius)), 1);
	output.worldPosition = output.position;
	output.position = mul(output.position, WorldViewProj);
	output.clippingPosition = output.position;
	TopUpperRight = output;

	
	//Lower LEFT (-RX, -RY)
	output.position = float4(facePosition + (orthoA * (-radius)) + (orthoB * (-radius)), 1);
	output.worldPosition = output.position;
	output.position = mul(output.position, WorldViewProj);
	output.clippingPosition = output.position;
	TopLowerLeft = output;

	// Lower RIGHT (RX, -RY)
	output.position = float4(facePosition + (orthoA * (radius)) + (orthoB * (-radius)), 1);
	output.worldPosition = output.position;
	output.position = mul(output.position, WorldViewProj);
	output.clippingPosition = output.position;
	TopLowerRight = output;
	
	if(dot(output.normal, viewDir) <0 )
	{
		outputStream.Append(TopLowerLeft);
		outputStream.Append(TopUpperLeft);
		outputStream.Append(TopLowerRight);
		outputStream.Append(TopUpperRight);
		outputStream.RestartStrip();
	}

	//## BOTTOM FACE
	//output.normal = -input[0].normal;
	facePosition = input[0].position.xyz - output.normal * radius;
	PHOTON_SPLAT BottomUpperLeft	= (PHOTON_SPLAT)0;
	PHOTON_SPLAT BottomUpperRight	= (PHOTON_SPLAT)0;
	PHOTON_SPLAT BottomLowerLeft	= (PHOTON_SPLAT)0;
	PHOTON_SPLAT BottomLowerRight	= (PHOTON_SPLAT)0;

	//Upper LEFT (RX, RY)
	output.position = float4(facePosition + (orthoA * (radius)) + (orthoB * (radius)), 1);
	output.worldPosition = output.position;	
	output.position = mul(output.position, WorldViewProj);
	output.clippingPosition = output.position;
	BottomUpperLeft = output;

	// Upper RIGHT ( -RX, RY)
	output.position = float4(facePosition + (orthoA * (-radius)) + (orthoB * (radius)), 1);
	output.worldPosition = output.position;
	output.position = mul(output.position, WorldViewProj);
	output.clippingPosition = output.position;
	BottomUpperRight = output;

	//Lower LEFT (RX, -RY)
	output.position = float4(facePosition + (orthoA * (radius)) + (orthoB * (-radius)), 1);
	output.worldPosition = output.position;
	output.position = mul(output.position, WorldViewProj);
	output.clippingPosition = output.position;
	BottomLowerLeft = output;

	// lower RIGHT (-RX, -RY)
	output.position = float4(facePosition + (orthoA * (-radius)) + (orthoB * (-radius)), 1);
	output.worldPosition = output.position;
	output.position = mul(output.position, WorldViewProj);
	output.clippingPosition = output.position;
	BottomLowerRight = output;

	if(dot(-output.normal, viewDir) <0 )
	{
		outputStream.Append(BottomUpperRight);
		outputStream.Append(BottomLowerRight);
		outputStream.Append(BottomUpperLeft);
		outputStream.Append(BottomLowerLeft);
		outputStream.RestartStrip();
	}

	//## LEFT FACE
	if(dot(-orthoA, viewDir) <0 )
	{
		outputStream.Append(BottomUpperRight);
		outputStream.Append(TopUpperLeft);
		outputStream.Append(BottomLowerRight);
		outputStream.Append(TopLowerLeft);
		outputStream.RestartStrip();
	}

	//## RIGHT FACE	
	if(dot(orthoA, viewDir) <0 )
	{
		outputStream.Append(BottomLowerLeft);
		outputStream.Append(TopLowerRight);
		outputStream.Append(BottomUpperLeft);
		outputStream.Append(TopUpperRight);
		outputStream.RestartStrip();
	}

	//## FRONT FACE	
	if(dot(-orthoB, viewDir) < 0 )	
	{
		outputStream.Append(BottomLowerRight);
		outputStream.Append(TopLowerLeft);
		outputStream.Append(BottomLowerLeft);
		outputStream.Append(TopLowerRight);
		outputStream.RestartStrip();
	}

	

	//## BACK FACE	
	if(dot(orthoB, viewDir) < 0 )	
	{
		outputStream.Append(BottomUpperLeft);
		outputStream.Append(TopUpperRight);
		outputStream.Append(BottomUpperRight);
		outputStream.Append(TopUpperLeft);
		outputStream.RestartStrip();
	}

	
	// ### END ###
}


float4 SplatPixelShader( PHOTON_SPLAT input) : SV_Target0
{
	float4 result = float4(0,0,0,1);
	
	//return float4(abs(input.direction.rgb), 1.0f);

	float2 texCoord = ((input.clippingPosition.xy/ input.clippingPosition.w) * 0.5f) + 0.5f; // screen coords are [-1, 1]
	texCoord.y = 1.0f - texCoord.y;
	float3 surfaceNormal =	normalMap.SampleLevel(defaultSampler, texCoord, 0).xyz;//input.normal.rgb;
	float3 wsPosition = wsPositionMap.SampleLevel(defaultSampler, texCoord, 0).xyz;
	float4 diffuse = diffuseMap.SampleLevel(defaultSampler, texCoord, 0);
	float3 photonNormal = input.normal.xyz;
	//result = float4(surfaceNormal, 1);

	// falloff
	float3 photonToWorld = wsPosition - input.photonPosition.xyz;
	float distanceToCenter = length(photonToWorld);//input.worldPosition.xyz);
	if(distanceToCenter >= input.travelDistance) //travelDistance is now Radius from GS
		discard;

	distanceToCenter = smoothstep(input.travelDistance, 0.0f, distanceToCenter ); 

	////test
	////result.rgb = input.power.rgb * distanceToCenter;
	//result.rgb = abs(input.direction.rgb) * distanceToCenter;
	//return float4(abs(result.rgb), 1.0f);

    // Specular exponent. 
    float4 specular = specularMap.SampleLevel(defaultSampler, texCoord, 0).rgba;
    //float3 adjustedPower = (input.power / (PI * input.pathDensity * input.pathDensity)) * distanceToCenter;
	float3 adjustedPower = input.power * distanceToCenter;
	float3 viewDir = normalize(wsPosition - cameraPosition);

	if(length(specular) >= 0.1)
	{
		float3 kDiffuse = diffuse.rgb / PI;
		float3 kSpecular = ((specular.a + 8) / 8*PI) * specular.rgb;

		result.rgb = PhongShade( wsPosition, surfaceNormal, kDiffuse, kSpecular,  specular.a, adjustedPower, viewDir, input.direction )* 5.0f;
	}
	else
		result.rgb = adjustedPower * (diffuse.rgb / ((diffuse.r + diffuse.g + diffuse.b)/3));

		//return float4(adjustedPower / 100.0f, 1.0f);
		result = float4(result.rgb * saturate(dot(-photonNormal, viewDir)), 1.0f);//(1 - smoothstep(0, input.pathDensity ,distanceToCenter)) * 0.01f ); //+ 0.004f);// float4(adjustedPower * cosInAngle, 1);
   
   //result *= 0.5;
	
	 //// fourth component of specular is specular exponent
    //if (specular.a != 0.0) {
        //float3 outgoingDirection = normalize(cameraPosition - wsPosition);
        //float3 vectorInOut = normalize(input.direction.xyz + outgoingDirection);
                //
        //// outgoingDirection*normal and vectorInOut*normal must both be greater than zero because
        //// the surface is an eye and light front-face, so no max(0, ...) needed
        //// on this dot product...however, when we remove it the result has errors
        //float gloss = pow(abs(dot(surfaceNormal, vectorInOut)), specular.a * 127.0 + 1.0);
        //result.rgb = float4(adjustedPower.rgb * gloss,1);
		////result = float4(specular.a,0,0,1);
    //}


	//DEBUG
	//result = float4(abs(photonNormal),1);
	return result;
}


#endif // SPLATBOUNCEMAP_HLSL
