#pragma once
// here are some constants

// ## craftMyGBuffer constants ##

#define DIFFUSECONSTANT			D3DXVECTOR4(1.0,1.0,1.0,1.0)
#define SPECULARCONSTANT		D3DXVECTOR4(1.0,1.0,1.0,1.0)
#define TRANSMISSIVECONSTANT	D3DXVECTOR3(1.0,1.0,1.0)

// BoxCity
#define PHOTONS_PER_SCENE 45000
#define PHOTONS_PER_SPOTLIGHT_QUOTA (0.01f)
//
////// cornell Box
//#define PHOTONS_PER_SCENE 20000
//#define PHOTONS_PER_SPOTLIGHT_QUOTA (0.5f)

#define BOUNCEMAP_HEIGHT 1024.0f
#define BOUNCEMAP_WIDTH 1024.0f

#define CAMERA_NEAR 1.0f
#define CAMERA_FAR 2000.0f

//TODO: calculate this	
//	- spotlight: (1 - sunlightquota) / nr of spotlights
//  - calculated from distance and visibility of spotlights
#define SUNLIGHT_QUOTA 1.0f
struct LightConstants
{
	D3DXMATRIX	lightViewProj;
	D3DXMATRIX  shadowToScreen;
	D3DXVECTOR4	lightPower;
	D3DXVECTOR4 lightPositionAngle; //contains distance in w // if w is < 0.0f it is the sun!
	D3DXVECTOR4 lightDirectionDistance; // normalized; contains Distance in w
};

struct PositionTexCoordVertex
{
	D3DXVECTOR3 position;
	D3DXVECTOR2 TexCoord;
};

struct PositionNormalTexCoordVertex
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 TexCoord;
};

__declspec(align(16))
struct TextureToScreenConstants
{
	D3DXVECTOR4 mDestRect; 
};

__declspec(align(16))
struct CraftMyGBufferConstants
{
	D3DXVECTOR4 diffuseConstant;
	D3DXVECTOR4 specularConstant;
	D3DXVECTOR3 transmissiveConstant;
};

struct geometryConstants
{
	D3DXMATRIX objToWorld;
};

__declspec(align(16))
struct CameraConstants
{
	D3DXMATRIX World; // object to world (of light?)
	D3DXMATRIX View;
	D3DXMATRIX WorldViewProjection;
	D3DXVECTOR3 cameraPosition;
	float refractiveIndexETA;
};