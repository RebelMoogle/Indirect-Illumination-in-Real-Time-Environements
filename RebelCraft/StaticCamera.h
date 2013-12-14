//--------------------------------------------------------------------------------------
// File: StaticCamera.h
//
// A static camera derived from CBaseCamera in DXUTcamera.h
//
// Copyright (c) John McLaughlin
//--------------------------------------------------------------------------------------

#pragma once

#include "DXUT.h"
#include "DXUTcamera.h"
#include "IOptixResource.h"
#include "ID3DResource.h"
#include "SDKmesh.h"
#include "Shader.h"
#include "Constants.h"
#include "ConstantBuffer.h"
#include <vector>
#include <memory>

// to prevent circular dependency. (spotlight has a StaticCamera)
class SpotLight;
class SunLight;

class StaticCamera : public CBaseCamera,
	public IOptixDeviceResource,
	public ID3DDeviceResource
{
public:
	StaticCamera(void);
	~StaticCamera(void);

	bool D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc);		
	void D3DReleaseDevice();

	bool OptixCreateDevice(optix::Context& Context);
	void OptixReleaseDevice();

	ConstantBuffer<CameraConstants>* GetCameraCBuffer(){return cameraCBuffer;};

	virtual void FrameMove( FLOAT fElapsedTime );
	//a function to set the cameras matrices from light information
	void SetFromSpotlight(D3DXVECTOR3*, D3DXVECTOR3*, float, float, D3DXVECTOR4); //D3DXVECTOR3 position, D3DXVECTOR3 direction, float radius = 30.0f, float farPlane = 10.0f, D3DXVECTOR3 color = D3DXVECTOR3(1.0f, 1.0f, 1.0f));
	void SetFromSunlight(D3DXVECTOR3* position, D3DXVECTOR3* direction, D3DXVECTOR4 Power, float Width, float Height, float Depth);
	// Functions to get state
	const D3DXMATRIX* GetWorldMatrix() const
	{
		return &mCameraWorld;
	}

	const D3DXVECTOR3* GetWorldRight() const
	{
		return ( D3DXVECTOR3* )&mCameraWorld._11;
	}
	const D3DXVECTOR3* GetWorldUp() const
	{
		return ( D3DXVECTOR3* )&mCameraWorld._21;
	}
	const D3DXVECTOR3* GetWorldAhead() const
	{
		return ( D3DXVECTOR3* )&mCameraWorld._31;
	}
	const D3DXVECTOR3* GetEyePt() const
	{
		return ( D3DXVECTOR3* )&mCameraWorld._41;
	}

	const D3DXVECTOR4* GetLightColor() const
	{
		return &mLightColor;
	}



private:

	void SetOrthoParams( float width, float height, float nearPlane = CAMERA_NEAR, float farPlane = CAMERA_FAR);

	D3DXMATRIX mCameraWorld;       // World matrix of the camera (inverse of the view matrix)

	D3DXVECTOR4 mLightColor;

	ConstantBuffer<CameraConstants>* cameraCBuffer;


};

