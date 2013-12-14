#include "DXUT.h"
#include "SpotLight.h"
#include "OptixTypes.h"
#include "ContentLoader.h"

bool SpotLight::isInitialized = false;

SpotLight::SpotLight(void) : 
Light()
{
	mDirection = D3DXVECTOR3(0.1,-0.8,0.1);
	mDistance = 2000.0f;
	mAngle = D3DX_PI/4;
	mStaticCamera = NULL;
	mPower = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
	float mWidth = 0.0f;
	float mHeight = 0.0f;
}


SpotLight::~SpotLight(void)
{
	SAFE_DELETE(mStaticCamera);
}

D3DXVECTOR3* SpotLight::GetPosition()
{
	return &mPosition;
}

const D3DXVECTOR3& SpotLight::GetDirection() const
{
	return mDirection;
}

float SpotLight::GetDistance()
{
	return D3DXVec3Length(&mDirection);
}

float SpotLight::GetAngle()
{
	return mAngle;
}

void SpotLight::SetPosition(D3DXVECTOR3 position)
{
	mPosition = position;
}

// is normalized!
void SpotLight::SetDirection(D3DXVECTOR3 direction)
{
	mDirection = direction / D3DXVec3Length(&direction);
}

void SpotLight::SetDistance( float length)
{	
	mDistance = length;
}

void SpotLight::SetAngle(float angle)
{
	mAngle = angle;
}

// create a new StaticCamera when not present already.
StaticCamera* SpotLight::GetCamera()
{
	if(!mStaticCamera)
	{
		mStaticCamera = new StaticCamera();
		mStaticCamera->SetFromSpotlight(&mPosition, &mDirection, mAngle, mDistance, mPower);
	}
	return mStaticCamera;
}

// resets the camera with current values
void SpotLight::ResetCamera()
{
	if(!mStaticCamera)
	{
		mStaticCamera = new StaticCamera();
	}
	mStaticCamera->SetFromSpotlight(&mPosition, &mDirection, mAngle, mDistance, mPower);

}

bool SpotLight::OptixCreateDevice(optix::Context& Context)
{
	return true;
}

void SpotLight::OptixReleaseDevice()
{
}

bool SpotLight::D3DCreateDevice( ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc )
{
	if(!GetCamera())
	{
		DXUT_ERR_MSGBOX(L"no static camera in spotlight", S_FALSE);
		return false;
	}
	
	mStaticCamera->D3DCreateDevice(Device, BackBufferSurfaceDesc);

	return true;
}

void SpotLight::D3DReleaseDevice()
{
	if(mStaticCamera)
		mStaticCamera->D3DReleaseDevice();
}
