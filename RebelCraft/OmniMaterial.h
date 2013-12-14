#pragma once
#include "RebelMaterial.h"
#include "ConstantBuffer.h"
#include "OptixTypes.h"

struct OmniConstants
{
	D3DXVECTOR4 diffuseColor;
	D3DXVECTOR4 specularColor;
	D3DXVECTOR4 transmissive;
};

class OmniMaterial : public RebelMaterial
{
public:
	// Constructor.	
	explicit OmniMaterial(const optix::float3& diffuse = optix::make_float3(1.0f,1.0f,1.0f), const optix::float3& specular = optix::make_float3(1.0f,1.0f,1.0f), const optix::float3& transmissive = optix::make_float3(0.0f,0.0f,0.0f), float refractiveETA = RefractIdx_Vacuum, float specularExponent = 1.0f);
	// Destructor.
	~OmniMaterial();

	// Binds effect parameters.
	void BindParams(ID3D11DeviceContext* ImmediateContext);

	bool D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc);
	void D3DReleaseDevice();
	bool OptixCreateDevice(optix::Context& Context);
	void OptixReleaseDevice();

protected:



private:		

	ConstantBuffer<OmniConstants>* omniCBuffer;

	static bool isInitialized;
	static optix::Program closestHit_DirectDiffuse;
	static optix::Program closestHit_PhotonRay;

};