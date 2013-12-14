#pragma once
#include "RebelGeometry.h"
class BoxGeometry :
	public RebelGeometry
{
public:
	BoxGeometry(const D3DXVECTOR3& position, const D3DXVECTOR3& scale, bool isStatic = true);
	~BoxGeometry(void);

	bool D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc);
	void D3DReleaseDevice();
	bool OptixCreateDevice(optix::Context& Context);
	void OptixReleaseDevice();

	void SetPosition(D3DXVECTOR3);
	void SetScale(D3DXVECTOR3);

private:

	D3DXVECTOR3 mPosition;
	D3DXVECTOR3 mScale;

	static bool mInitialized;
	static optix::Program mProgBoundingBox;
	static optix::Program mProgIntersection;
};

