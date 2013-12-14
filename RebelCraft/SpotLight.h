#pragma once

//--------------------------------------
// File: SpotLight.h
//
// describes a Spotlight
// also includes a StaticCamera, using the Spotlight orientation
//  has to be updated by hand if values are changed
//
// copyright John McLaughlin
//---------------------------------------

#include "Light.h"
#include "StaticCamera.h"

class SpotLight : public Light
{
public:
	SpotLight(void);
	~SpotLight(void);

	D3DXVECTOR3* GetPosition();

	//normalized direction
	const D3DXVECTOR3& GetDirection() const;
	float GetDistance();
	float GetAngle();
	
	StaticCamera* GetCamera();

	void SetPosition(D3DXVECTOR3);

	//will be normalized
	void SetDirection(D3DXVECTOR3);
	void SetDistance(float);
	void SetAngle(float);	
	void ResetCamera();

	bool D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc);		
	void D3DReleaseDevice();

	bool OptixCreateDevice(optix::Context& Context);
	void OptixReleaseDevice();		

private:
	D3DXVECTOR3 mPosition;
	D3DXVECTOR3 mDirection;
	float mDistance;
	float mAngle;	
	StaticCamera* mStaticCamera;
	static bool isInitialized;
	
};

