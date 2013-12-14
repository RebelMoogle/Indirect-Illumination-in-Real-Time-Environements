#include "DXUT.h"
#include "Light.h"

Light::Light()
{
	mAmbientColor = D3DXVECTOR4(0,0,0,0);
	mDiffuseColor = D3DXVECTOR4(0,0,0,0);
	mSpecularColor = D3DXVECTOR4(0,0,0,0);
	mSpecularPower = 0.0f;
}

Light::Light(D3DXVECTOR4 ambient, D3DXVECTOR4 diffuse, D3DXVECTOR4 specular, float specularPower)
{
	mAmbientColor = ambient;
	mDiffuseColor = diffuse;
	mSpecularColor = specular;
	mSpecularPower = specularPower;
}


Light::~Light(void)
{
}


D3DXVECTOR4 Light::GetAmbientColor()
{
	return mAmbientColor;
}

D3DXVECTOR4 Light::GetDiffuseColor()
{
	return mDiffuseColor;
}

D3DXVECTOR4 Light::GetSpecularColor()
{
	return mSpecularColor;
}

float Light::GetSpecularPower()
{
	return mSpecularPower;
}

void Light::SetAmbientColor(D3DXVECTOR4 ambient)
{
	mAmbientColor = ambient;
	return;
}

void Light::SetDiffuseColor(D3DXVECTOR4 diffuse)
{
	mDiffuseColor = diffuse;
	return;
}

void Light::SetSpecularColor(D3DXVECTOR4 specular)
{
	mSpecularColor = specular;
	return;
}

void Light::SetSpecularPower(float power)
{
	mSpecularPower = power;
	return;
}


void Light::SetPower( D3DXVECTOR4 power)
{
	mPower = power;
}

D3DXVECTOR4 Light::GetPower()
{
	return mPower;
}

void Light::SetImportance( float importance)
{
	mImportance = importance;
}

float Light::GetImportance()
{
	return mImportance;
}
