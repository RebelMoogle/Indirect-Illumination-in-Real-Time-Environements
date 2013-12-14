#ifndef _IOPTIX_RESOURCE__
#define _IOPTIX_RESOURCE__

// Interface for optix resources that do not depend on the swap chain.
class IOptixDeviceResource
{
	public:
		virtual bool OptixCreateDevice(optix::Context& Context) = 0;		
		virtual void OptixReleaseDevice() = 0;
};

// Interface for optix resources that depend on the swap chain.
class IOptixSwapChainResource
{
	public:
		virtual bool OptixCreateSwapChain(optix::Context& Context, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc) = 0;
		virtual void OptixReleaseSwapChain() = 0;
};

#endif