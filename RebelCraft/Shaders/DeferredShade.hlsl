// Deferred direct light shading

#ifndef DEFERRED_SHADE_HLSL
#define DEFERRED_SHADE_HLSL

#include "ShaderConstants.hlsl"

tbuffer textures
{
	
	Texture2D<float4>	lightWSPositionMap : register(t3);

	//Gbuffer: specular, albedo, transmittance (rgb: transmittance, a: eta_material), normal
	Texture2D<float4>	specularMap		: register(t4);
	Texture2D<float4>	diffuseMap		: register(t5);
	Texture2D<float4>	transmittanceMap		: register(t6);
	Texture2D<float4>	normalMap		: register(t7);
	Texture2D<float4>	wsPositionMap	: register(t8);
}

SamplerComparisonState shadowSampler : register(s1);

struct VS_INPUT
{
	float3 Pos          : POSITION;       
	float2 TexCoord     : TEXCOORD;      
};

struct GSPS_INPUT
{
	float4 Pos			: SV_POSITION;      
	float2 TexCoord     : TEXCOORD;     
};

float3 PhongShade(float3 position, float3 normal, float3 kDiffuse, float3 kSpecular, float specularExponent, float3 lightDirection)
{
	float3 viewDirection = normalize(cameraPosition - position);
	float3 outgoingRadiance = float3(0.0f, 0.0f, 0.0f);

	float3	halfLightView = normalize(viewDirection - lightDirection);
	float	cosThetaH = saturate(dot(normal, halfLightView));
	float	cosThetaI = saturate(dot(normal, -lightDirection));
	outgoingRadiance = (kDiffuse + kSpecular * pow(cosThetaH, specularExponent)) * lightPower.rgb * cosThetaI;

	return outgoingRadiance;
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
GSPS_INPUT DeferredShadeVS( VS_INPUT input )
{
	GSPS_INPUT output = (GSPS_INPUT)0;
	
	//output.wsPos = float4(input.Pos, 1);
	//test
	//output.Pos = mul(output.wsPos, WorldViewProj);
	//output.Normal = input.Normal; //mul(input.Normal, (float3x3)normWorld);

	output.Pos = float4(input.Pos,1);
	output.TexCoord = input.TexCoord;
	
	return output;
}

// computes a soft shadow
float ComputeShadow(float4 worldPosition)
{
	float4 lightScreenPosition = mul(worldPosition, lightWorldViewProj);
	lightScreenPosition.xyz /= lightScreenPosition.w;

	lightScreenPosition.xy = mad(lightScreenPosition.xy, 0.5, 0.5);
	lightScreenPosition.y = 1 - lightScreenPosition.y;

	int2 lightDimensions;
	lightWSPositionMap.GetDimensions(lightDimensions.x, lightDimensions.y);

	float2 rightTexelSize = float2(1.0f / lightDimensions.x, 0.0f);
	float2 upTexelSize = float2(0.0f, 1.0f / lightDimensions.y);

	float4 rightTexelDepthRatio = mul(float4(rightTexelSize, 0, 1), shadowToScreen);
	float4 upTexelDepthRatio = mul( float4(upTexelSize, 0, 1), shadowToScreen);

	rightTexelDepthRatio /= rightTexelDepthRatio.w;
	upTexelDepthRatio /= upTexelDepthRatio.w;
	
	float2 ddDepth = float2(ddx(lightScreenPosition.z), ddy(lightScreenPosition.z));
	
	float upTexelDepthDelta =  0.0f;//dot(upTexelDepthRatio.xy, ddDepth);
	float rightTexelDepthDelta = 0.0f; //dot(rightTexelDepthRatio.xy, ddDepth);
	
	float fPercentLit = 0;
	int2 gBufferDimensions = int2(0,0);
	wsPositionMap.GetDimensions(gBufferDimensions.x, gBufferDimensions.y);
	float2 nativeTexelSize = float2(1.0f / gBufferDimensions.x, 1.0f / gBufferDimensions.y);
	for( int x = -kernelSize; x <= kernelSize; ++x ) 
	{ 
		for( int y = -kernelSize; y <= kernelSize; ++y ) 
		{ 
			//float screenSpaceDepth = 	-DepthBias + lightScreenPosition.z + rightTexelDepthDelta * ( (float) x ) + 
										//upTexelDepthDelta * ( (float) y ); 

			//// Shader model 4.0 and above (need better depth buffer
			//fPercentLit += lightWSPositionMap.SampleCmpLevelZero( 
							//ShadowSampler, 
							//float2( lightScreenPosition.x + ( ( (float) x ) * nativeTexelSize.x ),
									//lightScreenPosition.y + ( ( (float) y ) * nativeTexelSize.y ) ), 
							//screenSpaceDepth ).x;


			// older Shader Models
			//fPercentLit += step(
			float4 posLightWorld = lightWSPositionMap.SampleLevel(defaultSampler, 
										float2( lightScreenPosition.x + ( ( (float) x ) * nativeTexelSize.x ),
												lightScreenPosition.y + ( ( (float) y ) * nativeTexelSize.y ) ), 
										0);
			float distLightMap = (distance(posLightWorld.xyz, lightPositionAngle.xyz));
			float distPosLight = distance(worldPosition.xyz, lightPositionAngle.xyz) - DepthBias + rightTexelDepthDelta * ( (float) x ) + upTexelDepthDelta * ( (float) y ); //
			if( distLightMap < distPosLight)
			{
				fPercentLit += 1.0f;
			}

		}
	}  
	// number of Pixels traversed
	int numPixels = mad(kernelSize, 2, 1);
	numPixels *= numPixels;

	return (1- fPercentLit / numPixels );
}

//--------------------------------------------------------------------------------------
// Pixel Shader - SPOTLIGHT
//--------------------------------------------------------------------------------------
float4 DeferredShadePS( GSPS_INPUT input) : SV_Target
{
	float4 result = float4(0,0,0,1.0);
	float4 wsPosition = wsPositionMap.SampleLevel(defaultSampler, input.TexCoord, 0);
	float4 diffuseColor = diffuseMap.SampleLevel(defaultSampler, input.TexCoord, 0);
	float4 specularColor = specularMap.SampleLevel(defaultSampler, input.TexCoord, 0);
	
	// make early check against light cone (need angle)
	float3 lightToWSPosition = normalize(wsPosition.xyz - lightPositionAngle.xyz);
	float lightToPosAngle = dot(lightDirectionDistance.xyz, lightToWSPosition); // not for orthogonal!
	// wsPosition is outside of spotlightCone
	float angle = cos(lightPositionAngle.w * 0.5f);

	result.rgb = diffuseColor.rgb;

	if( angle+0.05f > lightToPosAngle)
	{
		//discard;
		// TODO SHADOWS IN FALLOFF
		result.rgb *= (smoothstep(angle, angle + 0.05f, lightToPosAngle));//* clamp(ComputeShadow(wsPosition), 0.0, 1.0)); // * 0.9 + 0.1);
		specularColor.rgb *= (smoothstep(angle, angle + 0.05f, lightToPosAngle));
		//result.rgb *= lightPower.rgb * 10.0f;
		//return result;
	}
	 
	float3 normal = normalMap.SampleLevel(defaultSampler, input.TexCoord, 0).xyz;
	
	float cosAngleNormLight = dot(normal, -lightDirectionDistance.xyz);
	
	
	if(cosAngleNormLight > 0.0f && angle-0.05f < lightToPosAngle)
	{
		//result.rgb *= lightPower.rgb * 100.0f;
		float3 kDiffuse = result.rgb / PI; //diffuseColor.rgb / PI;
		float3 kSpecular = ((specularColor.a + 8) / 8*PI) * specularColor.rgb;

		result.rgb = PhongShade( wsPosition, normal, kDiffuse, kSpecular,  specularColor.a, lightToWSPosition) * 100.0f;
		
		result.rgb = result.rgb * clamp(ComputeShadow(wsPosition), 0.0f, 1.0f);
		//return result;	
	}
	else
	 result.rgb *= 0.0f;

	result.rgb += 0.1f * diffuseColor.rgb;

	return result;
	//return float4(1.0, 0, 0, 1.0);
}

//--------------------------------------------------------------------------------------
// Pixel Shader - SUNLIGHT
//--------------------------------------------------------------------------------------
float4 DeferredSunShadePS( GSPS_INPUT input) : SV_Target
{
	float4 result = float4(0,0,0,1.0);
	float4 wsPosition = wsPositionMap.SampleLevel(defaultSampler, input.TexCoord, 0);
	float4 diffuseColor = diffuseMap.SampleLevel(defaultSampler, input.TexCoord, 0);
	float4 specularColor = specularMap.SampleLevel(defaultSampler, input.TexCoord, 0);

	//TODO distribute values equally (histogram)
	float3 normal = normalMap.SampleLevel(defaultSampler, input.TexCoord, 0).xyz;
	
	//clamp(0.1, 1.0, dot(normal, lightDirectionDistance.xyz))
	float cosAngleNormLight = dot(normal, -lightDirectionDistance.xyz);
	
		
	//result.rgb = diffuseColor.rgb * lightPower.rgb *0.1f; 
	//result.rgb = diffuseColor.rgb * lightPower.rgb * 25.0f;
	float3 kDiffuse = diffuseColor.rgb / PI;
	float3 kSpecular = ((specularColor.a + 8) / 8*PI) * specularColor.rgb;

	result.rgb = PhongShade( wsPosition, normal, kDiffuse, kSpecular,  specularColor.a, lightDirectionDistance.xyz) * 100.0f;
	if(cosAngleNormLight > 0.0f)
	{
		result.rgb = result.rgb * clamp(ComputeShadow(wsPosition), 0.0f, 1.0f);
		return result;	
	}
	result.rgb = clamp(cosAngleNormLight, 0.1f, 1.0f) * result.rgb;

	return result;
	//return float4( ComputeShadow(wsPosition), 0, 0, 1.0);
}



#endif // RENDERVERTEX_HLSL