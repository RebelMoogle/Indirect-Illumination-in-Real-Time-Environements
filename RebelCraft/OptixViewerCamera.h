#pragma once
#include "DXUTcamera.h"
#include "IOptixResource.h"
#include "ConstantBuffer.h"
#include "Constants.h"
#include "ID3DResource.h"

class OptixViewerCamera :
	public CFirstPersonCamera,
	public IOptixDeviceResource,
	public ID3DDeviceResource
{
public:
	OptixViewerCamera(void);
	~OptixViewerCamera(void);

	bool D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc);		
	void D3DReleaseDevice();

	bool OptixCreateDevice(optix::Context& Context);		
	void OptixReleaseDevice();

	virtual void FrameMove( FLOAT fElapsedTime );
	
	ConstantBuffer<CameraConstants>* GetCameraCBuffer(){return cameraCBuffer;};

private:
	optix::Program rayGenerationProgram;

	void UpdateReferenceFrame();

	optix::float3 uAxis;
	optix::float3 vAxis;
	optix::float3 wAxis;

	ConstantBuffer<CameraConstants>* cameraCBuffer;

};

