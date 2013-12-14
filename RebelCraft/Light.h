#pragma once

#include "IOptixResource.h"

// basic light class. 
// has all the basic light values.
// inherit for light types.

class StaticCamera;

class Light : public IOptixDeviceResource
{
public:
	explicit Light();
	Light(D3DXVECTOR4, D3DXVECTOR4, D3DXVECTOR4, float);
	~Light(void);

	void SetAmbientColor(D3DXVECTOR4);
	void SetDiffuseColor(D3DXVECTOR4);
	void SetSpecularColor(D3DXVECTOR4);
	void SetSpecularPower(float);
	void SetPower(D3DXVECTOR4);

	// sets light importance (for photon distribution)
	void SetImportance(float);

	D3DXVECTOR4 GetAmbientColor(void);
	D3DXVECTOR4 GetDiffuseColor(void);
	D3DXVECTOR4 GetSpecularColor(void);
	float		GetSpecularPower(void);
	D3DXVECTOR4 GetPower();

	// gets light importance (for photon distribution)
	float		GetImportance();

protected:
	D3DXVECTOR4 mPower;
	float		mImportance;

private:
	D3DXVECTOR4 mAmbientColor;
	D3DXVECTOR4 mDiffuseColor;
	D3DXVECTOR4 mSpecularColor;
	float		mSpecularPower;

};

