#include "DXUT.h"
#include "Util.h"
#include "StaticCamera.h"
#include "XNAMath.h"
#include "ContentLoader.h"
#include "OptixTypes.h"


StaticCamera::StaticCamera(void) : 
CBaseCamera()
{
	//TODO set light at the beginning?
	cameraCBuffer = new ConstantBuffer<CameraConstants>();
}

StaticCamera::~StaticCamera(void)
{
}

void StaticCamera::SetFromSpotlight(D3DXVECTOR3* position, D3DXVECTOR3* direction, float FoV, float farPlane, D3DXVECTOR4 power)
{
	D3DXVECTOR3 d3dLookAt = (*position + *direction * farPlane);
	CBaseCamera::SetViewParams(position, &d3dLookAt);
	// let's calculate the appropriate opening angle
	//float fov = FoV;
	// aspect ratio according to BounceMap sizes
	float aspectRatio = BOUNCEMAP_WIDTH / BOUNCEMAP_HEIGHT;

	CBaseCamera::SetProjParams(FoV, aspectRatio, 1.0f, farPlane);

	mLightColor = power;
	
	//set constant buffer
	D3DXMATRIX identityMatrix;
	D3DXMatrixIdentity(&identityMatrix);

	cameraCBuffer->Data.World	= identityMatrix;
	cameraCBuffer->Data.refractiveIndexETA = 1.0f;
	cameraCBuffer->Data.View	= *GetViewMatrix();
	cameraCBuffer->Data.WorldViewProjection = identityMatrix * cameraCBuffer->Data.View * (*GetProjMatrix());
}

bool StaticCamera::D3DCreateDevice( ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc )
{
	m_fFarPlane = CAMERA_FAR;
	m_fNearPlane = CAMERA_NEAR;
	return cameraCBuffer->D3DCreateDevice(Device,BackBufferSurfaceDesc);
}

void StaticCamera::D3DReleaseDevice()
{
	cameraCBuffer->D3DReleaseDevice();
}


void StaticCamera::SetOrthoParams( float width, float height, float nearPlane, float farPlane)
{
	// Set attributes for the projection matrix
	m_fNearPlane = nearPlane;
	m_fFarPlane = farPlane;


	D3DXMatrixOrthoLH(&m_mProj, width, height, m_fNearPlane, m_fFarPlane);
}

void StaticCamera::SetFromSunlight(D3DXVECTOR3* position, D3DXVECTOR3* direction, D3DXVECTOR4 Power, float Width, float Height, float Depth)
{
	D3DXVECTOR3 d3dLookAt = D3DXVECTOR3(Width, Height, Depth) * 0.5f;//(*position + *Direction * m_fFarPlane);
	D3DXVECTOR3 eye = *position / D3DXVec3Length(position);
	eye *= D3DXVec3Length(&d3dLookAt) * 1.1f;
	eye += d3dLookAt;

	CBaseCamera::SetViewParams(&eye, &d3dLookAt);
	mLightColor = Power;

	*position = eye;
	d3dLookAt = d3dLookAt - eye;
	d3dLookAt /= D3DXVec3Length(&d3dLookAt);
	*direction = d3dLookAt;

	D3DXVECTOR3 cornersWorld[] = {
		D3DXVECTOR3(0,0,0), 
		D3DXVECTOR3(Width, 0, 0), 
		D3DXVECTOR3(Width, Height, 0), 
		D3DXVECTOR3(Width, Height, Depth),
		D3DXVECTOR3(0, Height, 0), 
		D3DXVECTOR3(0, Height, Depth), 
		D3DXVECTOR3(0,0,Depth), 
		D3DXVECTOR3(Width, 0, Depth)
	};

	D3DXVECTOR3 lightSpaceBoxMin = D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX);
	D3DXVECTOR3 lightSpaceBoxMax = D3DXVECTOR3(FLT_MIN, FLT_MIN, FLT_MIN);

	for (int i = 0; i < 8; ++i)
	{
		D3DXVec3TransformCoord(&cornersWorld[i], &cornersWorld[i], GetViewMatrix());
		lightSpaceBoxMin.x = lightSpaceBoxMin.x < cornersWorld[i].x ? lightSpaceBoxMin.x : cornersWorld[i].x;
		lightSpaceBoxMin.y = lightSpaceBoxMin.y < cornersWorld[i].y ? lightSpaceBoxMin.y : cornersWorld[i].y;
		lightSpaceBoxMin.z = lightSpaceBoxMin.z < cornersWorld[i].z ? lightSpaceBoxMin.z : cornersWorld[i].z;


		lightSpaceBoxMax.x = lightSpaceBoxMax.x > cornersWorld[i].x ? lightSpaceBoxMax.x : cornersWorld[i].x;
		lightSpaceBoxMax.y = lightSpaceBoxMax.y > cornersWorld[i].y ? lightSpaceBoxMax.y : cornersWorld[i].y;
		lightSpaceBoxMax.z = lightSpaceBoxMax.z > cornersWorld[i].z ? lightSpaceBoxMax.z : cornersWorld[i].z;
	};

	SetOrthoParams(lightSpaceBoxMax.x - lightSpaceBoxMin.x, lightSpaceBoxMax.y - lightSpaceBoxMin.y, lightSpaceBoxMin.z, lightSpaceBoxMax.z);
	
	//set constant buffer
	D3DXMATRIX identityMatrix;
	D3DXMatrixIdentity(&identityMatrix);

	cameraCBuffer->Data.World	= identityMatrix;
	cameraCBuffer->Data.refractiveIndexETA = 1.0f;
	cameraCBuffer->Data.View	= *GetViewMatrix();
	cameraCBuffer->Data.WorldViewProjection = identityMatrix * cameraCBuffer->Data.View * (*GetProjMatrix());
	//float test = D3DXMatrixDeterminant(GetProjMatrix());
	//test = D3DXMatrixDeterminant(GetViewMatrix());
	//test = test +1;
}

void StaticCamera::FrameMove( FLOAT fElapsedTime )
{

}

bool StaticCamera::OptixCreateDevice( optix::Context& Context )
{
	return true;
}

void StaticCamera::OptixReleaseDevice()
{

}
