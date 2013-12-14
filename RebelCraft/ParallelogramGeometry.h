#pragma once

#include "RebelGeometry.h"

class ParallelogramGeometry : public RebelGeometry
{
	public:
		// Constructor. Creates a geometry containing a parallelogram.	
		ParallelogramGeometry(const optix::float3& anchor, const optix::float3& offset1, const optix::float3& offset2);
		// Destructor.
		~ParallelogramGeometry();


		bool D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc);
		void D3DReleaseDevice();
		bool OptixCreateDevice(optix::Context& Context);
		void OptixReleaseDevice();

	private:

		const optix::float3 _Anchor;
		const optix::float3 _Offset1;
		const optix::float3 _Offset2;

		static bool _Initialized;
		static optix::Program _ProgBoundingBox;
		static optix::Program _ProgIntersection;
		
};
