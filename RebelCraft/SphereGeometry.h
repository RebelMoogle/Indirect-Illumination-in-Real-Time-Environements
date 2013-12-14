#pragma once

#include "RebelGeometry.h"

class SphereGeometry : public RebelGeometry
{
	public:
		// Constructor. Creates a geometry containing a cylinder.	
		SphereGeometry(const optix::float3& center, const float& radius);
		// Destructor.
		~SphereGeometry();


		bool D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc);
		void D3DReleaseDevice();
		bool OptixCreateDevice(optix::Context& Context);
		void OptixReleaseDevice();

	private:

		const optix::float3 _Center;		
		const float _Radius;

		static bool _Initialized;
		static optix::Program _ProgBoundingBox;
		static optix::Program _ProgIntersection;
		
};