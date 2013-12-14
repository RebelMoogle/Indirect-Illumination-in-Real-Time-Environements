#pragma once
/***
	Does everything needed for the Real-TIme Photon Mapping, 
		- requires a GBuffer with diffuse, specular, transmissive and depth, rendered from light
		- requires photon data from scene (count, power, etc)

		- Renders Bouncemap to Buffer or Rendertargets
		- starts optix, sets up required buffers for photon tracing and splatting
		- renders photon splats

	(c) John McLaughlin
***/

#include "Shader.h"
#include "DXUT.h"
#include "Constants.h"
#include "SpotLight.h"
#include "SunLight.h"
#include <vector>
#include <memory>


struct SplatConstants
{
	D3DXMATRIX lightViewProjection;
};

//__declspec(align(32))
//__declspec(align(16))
struct Photon
{
	// Photon:
	D3DXVECTOR4 position;
	D3DXVECTOR2 TexCoord;
	D3DXVECTOR4 incomingPower;
	D3DXVECTOR3 incomingDir;
	FLOAT  travelDistance;
	D3DXVECTOR4 surfaceNormal;
};

struct bounceInfo 
{
	// percentage of photons needed to be killed to fulfill photon quota
	float killPercentage;
};

class PhotonMapping
{
public:
	PhotonMapping(void);
	~PhotonMapping(void);

	bool D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc, ID3D11InputLayout* vertexLayout);		
	void D3DReleaseDevice();

	bool OptixCreateDevice(optix::Context& Context);
	void OptixReleaseDevice();	

	void CreateBounceBuffer(	ID3D11DeviceContext* d3dDeviceContext, std::vector<ID3D11ShaderResourceView*> gBufferSRV, ID3D11DepthStencilView* depthBufferView, 
								SpotLight* spotlight, float sumLightImportance,  bool isFirstLight = true);
	void CreateBounceBuffer(	ID3D11DeviceContext* d3dDeviceContext, std::vector<ID3D11ShaderResourceView*> gBufferSRV, ID3D11DepthStencilView* depthBufferView, 
		SunLight* sunlight, float sumLightImportance);


	void LaunchPhotons(ID3D11DeviceContext* d3dDeviceContext, optix::Context &Context);

	// renders the currently available photons splats
	void SplatPhotons(ID3D11DeviceContext*, std::vector<ID3D11ShaderResourceView*>, ID3D11RenderTargetView*);

	void ToneMapping(ID3D11DeviceContext* d3dDeviceContext, ID3D11ShaderResourceView* inputRender, ID3D11Buffer* quadVertices);


	// returns array of both shader resource views
	ID3D11ShaderResourceView* GetPowerDistanceSRV()
	{
		return powerDistanceSRV;
	}

	ID3D11ShaderResourceView* GetOutgoingDensitySRV()
	{
		return outgoingDensitySRV;
	}

	void RenderVertexGrid(ID3D11DeviceContext*, ID3D11DepthStencilView*, ID3D11RenderTargetView*);

	void UpdatePhotonCount(ID3D11DeviceContext* immediateContext);

private:

	//void SplatBounceMap(ID3D11DeviceContext*, ID3D11RenderTargetView*, SpotLight, std::vector<ID3D11ShaderResourceView*>);

	ID3D11Texture2D* powerDistance;
	ID3D11RenderTargetView* powerDistanceRTV;
	ID3D11ShaderResourceView* powerDistanceSRV;

	ID3D11Texture2D* outgoingDensity;
	ID3D11RenderTargetView* outgoingDensityRTV;
	ID3D11ShaderResourceView* outgoingDensitySRV;

	VertexShader*	bounceBufferVS;
	ID3D11GeometryShader* bounceBufferGS;

	ID3D11InputLayout* positionTexCoordLayout;
	ID3D11InputLayout* photonLayout;
	ID3D11Buffer*	quadConstants;
	ID3D11Buffer*	bounceLightConstants;
	//ID3D11Buffer*	splatCBuffer;

	VertexShader* toneMappingVS;
	PixelShader*  toneMappingPS;

	// Vertex Grid
	ID3D11Buffer*	vertexGrid;
	VertexShader*	renderVertexVS;
	PixelShader*	renderVertexPS;

	//splat photons
	VertexShader* splatPhotonsVS;
	GeometryShader* splatPhotonsGS;
	PixelShader* splatPhotonsPS;

	ID3D11BlendState* splatPhotonsBlendState;
	ID3D11DepthStencilState* splatPhotonsDepthState;
	ID3D11RasterizerState*	splatPhotonsRasterizerState;

	//initial Bounce Buffers
	ID3D11Buffer*	d3dPhotonInitialBuffer;
	ID3D11Buffer*	photonResultBufferStaging;
	ID3D11Buffer*	photonInitialBufferStaging;
	optix::Buffer optixPhotonBuffer;

	// resulting Bounce Buffers
	ID3D11Buffer* d3dPhotonResultBuffer;
	optix::Buffer optixPhotonResultBuffer;

	optix::Program optixPhotonEmitter;
	optix::Buffer optixRandomSeeds;

	ID3D11Query* photonCountQuery;

	UINT mPhotonCount;
	ConstantBuffer<bounceInfo>* mBounceCBuffer;
};

