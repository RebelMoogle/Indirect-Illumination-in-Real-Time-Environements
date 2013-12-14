// Copyright John McLaughlin
// have fun with it, and credit me.
// sending me a note you used this, would be awesome. Would be great to know people actually use this!

#pragma once

#include "DXUT.h"
#include "DXUTcamera.h"
#include "StaticCamera.h"
#include "SDKmesh.h"
#include <vector>
#include <memory>
#include "Constants.h"
#include "SpotLight.h"
#include "Shader.h"
#include "PhotonMapping.h"
#include "ID3DResource.h"
#include "IOptixResource.h"
#include "SDKmisc.h"
#include "DXUTsettingsdlg.h"
#include <iostream>
#include <sstream>

class OptixAPI;
class BaseSceneBuilder;

class App: public ID3DSwapChainResource, public ID3DDeviceResource, public IOptixDeviceResource, public IOptixSwapChainResource
{	
public:
	App();
	~App(void);

	void OnD3D11ResizedSwapChain(ID3D11Device* d3dDevice, const DXGI_SURFACE_DESC* backBufferDesc);

	void Move(float elapsedTime);

	void Render(ID3D11Device*, ID3D11DeviceContext*, 
				const D3D11_VIEWPORT*, 
				float fElapsedTime
				);

	void HandleMessages(HWND, UINT, WPARAM, LPARAM, bool*);

	bool OptixCreateDevice( optix::Context&  optixContext );
	bool OptixCreateSwapChain( optix::Context& optixContext, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc );

	void OptixReleaseDevice();
	void OptixReleaseSwapChain();

	bool D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc);
	void D3DReleaseDevice();

	bool D3DCreateSwapChain(ID3D11Device* Device, IDXGISwapChain* SwapChain, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc);
	void D3DReleaseSwapChain();

	OptixAPI* GetOptixPhotonMapping();

	PhotonMapping* GetPhotonMapper();

	CDXUTTextHelper* GetTextHelper();

	double GetTimeSinceLastCall();

	void RenderTextureToScreen(	ID3D11DeviceContext* d3dDeviceContext, ID3D11RenderTargetView* backBuffer, D3DXVECTOR4* destRectangle, // const UIConstants* ui,
		ID3D11ShaderResourceView* textureSRV);

	std::wstringstream textToRender;

	ID3D11InputLayout* mPositionTexCoordLayout;

private:


	void InitScene();

	OptixAPI* optixPhotonMapping;
	BaseSceneBuilder* sceneBuilder;

	PhotonMapping* photonMapper;

	//meshes
	CDXUTSDKMesh gOpague;
	CDXUTSDKMesh gAlpha;

	//camera
	float gAspectRatio; //default size

	//lights
	SpotLight* mSpotLight;

	ID3D11Buffer* craftMyGBufferChangesCamera;

	//render Texture
	VertexShader* mTextureToScreenVS;
	GeometryShader* mTextureToScreenGS;
	PixelShader* mTextureToScreenPS;

	ID3D11Buffer* mTextureToScreenVertexBuffer;
	ID3D11Buffer* mTextureToScreenCBuffer;

	ID3D11RasterizerState* mWireRasterizerState;
	ID3D11RasterizerState* mNoDepthRasterizerState;

	ID3D11DepthStencilState* mDefaultDepthState;

	ID3D11DepthStencilView* mDepthBufferLightCamView;

	//ID3D11SamplerState* defaultSampler;

	//text output stuff (onscreen)
	CDXUTDialogResourceManager mDialogMgr;
	CDXUTTextHelper* mText;
	
	//Timer
	INT64 mCounterLast;
	double mCounterFreq;

	//profiling
	ID3D11Query* pEvent;
};

