#ifndef __ID3D_RESOURCE__
#define __ID3D_RESOURCE__

// Interface for d3d resources that do not depend on the swap chain.
class ID3DDeviceResource
{
	public:
		virtual bool D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc) = 0;		
		virtual void D3DReleaseDevice() = 0;
};

// Interface for d3d resources that depend on the swap chain.
class ID3DSwapChainResource
{
	public:
		virtual bool D3DCreateSwapChain(ID3D11Device* Device, IDXGISwapChain* SwapChain, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc) = 0;
		virtual void D3DReleaseSwapChain() = 0;
};

#endif