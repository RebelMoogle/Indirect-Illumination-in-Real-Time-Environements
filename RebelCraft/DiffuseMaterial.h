#ifndef __DIFFUSE_MATERIAL__
#define __DIFFUSE_MATERIAL__

#include "RebelMaterial.h"
#include "ConstantBuffer.h"

struct diffuseConstants
{
	D3DXVECTOR4 diffuseColor;
};

class DiffuseMaterial : public RebelMaterial
{
	public:
		// Constructor.	
		explicit DiffuseMaterial(const optix::float3& Diffuse);
		// Destructor.
		~DiffuseMaterial();


		bool D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc);
		void D3DReleaseDevice();
		bool OptixCreateDevice(optix::Context& Context);
		void OptixReleaseDevice();

	protected:

		// Binds effect parameters.
		void BindParams(ID3D11DeviceContext* ImmediateContext);

	private:		

		optix::float3 _Diffuse;

		ConstantBuffer<diffuseConstants>* diffuseCBuffer;

		static bool isInitialized;
		static optix::Program closestHit_DirectDiffuse;
		static optix::Program closestHit_PhotonRay;
		
};

#endif