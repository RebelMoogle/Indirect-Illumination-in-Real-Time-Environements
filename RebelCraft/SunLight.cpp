#include "DXUT.h"
#include "SunLight.h"
#include "OptixTypes.h"


SunLight::SunLight(void) : 
Light()
{
	mPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	mDirection = D3DXVECTOR3(0.0,-1.0,0.0);
	mStaticCamera = NULL;
	mPower = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
}


SunLight::~SunLight(void)
{
	SAFE_DELETE(mStaticCamera);
}

bool SunLight::D3DCreateDevice( ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc )
{
	if(!GetCamera())
	{
		DXUT_ERR_MSGBOX(L"no static camera in spotlight", S_FALSE);
		return false;
	}

	mStaticCamera->D3DCreateDevice(Device, BackBufferSurfaceDesc);

	if(mWidth <= 0)
		mWidth = CAMERA_FAR/2;
	if(mHeight <= 0)
		mHeight = CAMERA_FAR/2;

	return true;
}

void SunLight::D3DReleaseDevice()
{
	if(mStaticCamera)
		mStaticCamera->D3DReleaseDevice();
}

bool SunLight::OptixCreateDevice( optix::Context& Context )
{
	return true;
}

void SunLight::OptixReleaseDevice()
{

}


const D3DXVECTOR3& SunLight::GetDirection() const
{
	//mDirection = *mStaticCamera->GetLookAtPt();
	//mDirection /= D3DXVec3Length(&mDirection);
	return mDirection;
}

StaticCamera* SunLight::GetCamera()
{
	if(!mStaticCamera)
	{
		mStaticCamera = new StaticCamera();
		mStaticCamera->SetFromSunlight(&mPosition, &mDirection, mPower, mWidth, mHeight, mDepth);
	}
	return mStaticCamera;
}

void SunLight::ResetCamera()
{
	if(!mStaticCamera)
	{
		mStaticCamera = new StaticCamera();
	}
	mStaticCamera->SetFromSunlight(&mPosition, &mDirection, mPower, mWidth, mHeight, mDepth);
}

const D3DXVECTOR3& SunLight::GetPosition() const
{
	//mPosition = mStaticCamera->GetEyePt();
	return mPosition;
}

void SunLight::SetWidth( float width)
{
	mWidth = width;
}

void SunLight::SetHeight( float height)
{
	mHeight = height;
}

void SunLight::SetDepth( float depth)
{
	mDepth = depth;
}

void SunLight::SetPosition(const D3DXVECTOR3& position )
{
	mPosition = position;
}

float SunLight::GetWidth( )
{
	return mWidth;
}


float SunLight::GetHeight( )
{
	return mHeight;
}


float SunLight::GetDepth( )
{
	return mDepth;
}
