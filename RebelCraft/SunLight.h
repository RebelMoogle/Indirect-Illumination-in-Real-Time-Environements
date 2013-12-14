#pragma once
#include "Light.h"
#include "StaticCamera.h"
class SunLight :
	public Light
{
public:
	SunLight(void);
	~SunLight(void);

	//normalized direction
	const D3DXVECTOR3& GetPosition() const;
	const D3DXVECTOR3& GetDirection() const;

	StaticCamera* GetCamera();

	void SetPosition(const D3DXVECTOR3& position);
	void SetWidth(float);
	void SetHeight(float);
	void SetDepth(float);
	void ResetCamera();

	float GetWidth();
	float GetHeight();
	float GetDepth();

	bool D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc);		
	void D3DReleaseDevice();

	bool OptixCreateDevice(optix::Context& Context);
	void OptixReleaseDevice();		

private:
	D3DXVECTOR3 mPosition;
	D3DXVECTOR3 mDirection;

	float mWidth;
	float mHeight;
	float mDepth;
	StaticCamera* mStaticCamera;
	static bool isInitialized;
};

