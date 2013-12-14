#include "DXUT.h"
#include "Scene.h"
#include "RebelGeometry.h"
#include "RebelMaterial.h"
#include "RebelGeometryInstance.h"
#include "OptixViewerCamera.h"
#include "File.h"
#include "Debug.h"
#include "Light.h"
#include "SunLight.h"
#include "SpotLight.h"
#include "StaticCamera.h"
#include "App.h"
#include "OptixAPI.h"
#include "OptixTypes.h"
#include "SDKmisc.h"
#ifdef PROFILING
	#include "Profiler.h"
#endif

// ================================================================================
Scene::Scene(App* givenApp) :
parentApp(givenApp),
optixViewportQuadVertices(NULL),
optixDebugOutputVS(NULL),
optixDebugOutputPS(NULL),
optixInputLayoutViewportQuad(NULL),
d3dRaycastOutput(NULL),
d3dRaycastOutputSRV(NULL),
d3dRaycastOutputStaging(NULL)
{
	_Geometries.clear();
	_Materials.clear();
	_GeometryInstances.clear();
	mSpotLights.clear();
	mSunLight = NULL;
	mSumLightImportance = 1.0f;
}

// ================================================================================
Scene::~Scene()
{
	_Geometries.clear();
	_Materials.clear();
	_GeometryInstances.clear();
	SAFE_DELETE(viewerCamera);
	mSpotLights.clear();
	SAFE_DELETE(mSunLight);
}

// ================================================================================
App* Scene::GetApp()
{
	return parentApp;
}

// ================================================================================
bool Scene::D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc)
{
	for (std::vector<RebelGeometry*>::iterator it = _Geometries.begin(); it != _Geometries.end(); ++it)
		if (!(*it)->D3DCreateDevice(Device, BackBufferSurfaceDesc))
			return false;
	for (std::vector<RebelMaterial*>::iterator it = _Materials.begin(); it != _Materials.end(); ++it)
		if (!(*it)->D3DCreateDevice(Device, BackBufferSurfaceDesc))
			return false;
	for (std::vector<SpotLight*>::iterator it = mSpotLights.begin(); it != mSpotLights.end(); ++it)
		if (!(*it)->D3DCreateDevice(Device, BackBufferSurfaceDesc))
			return false;
	if(mSunLight)
		mSunLight->D3DCreateDevice(Device, BackBufferSurfaceDesc);
	
	//load shader
	craftMyGBufferVS = new VertexShader(Device, L"Shaders\\CraftMyGBuffer.hlsl", "CraftMyGBufferVS");
	craftMyGBufferPS = new PixelShader(Device, L"Shaders\\CraftMyGBuffer.hlsl", "CraftMyGBufferPS");

	craftMyLightGBufferVS = new VertexShader(Device, L"Shaders\\CraftMyGBuffer.hlsl", "CraftMyLightGBufferVS");
	craftMyLightGBufferPS = new PixelShader(Device, L"Shaders\\CraftMyGBuffer.hlsl", "CraftMyLightGBufferPS");

	// Create  mesh input layout
	{
		ID3D10Blob *bytecode = craftMyGBufferVS->GetByteCode();

		const D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{"position",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"normal",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"texCoord",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		Device->CreateInputLayout( 
			layout, ARRAYSIZE(layout), 
			bytecode->GetBufferPointer(),
			bytecode->GetBufferSize(), 
			&mMeshVertexLayout);
	}

	{
		CD3D11_RASTERIZER_DESC desc(D3D11_DEFAULT);
		desc.CullMode = D3D11_CULL_BACK;
		desc.FrontCounterClockwise = FALSE;
		Device->CreateRasterizerState(&desc, &mRasterizerState);
	}

	// Create sampler state
	{
		CD3D11_SAMPLER_DESC desc(D3D11_DEFAULT);
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		Device->CreateSamplerState(&desc, &mDiffuseSampler);
	}

	{
		CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);
		desc.StencilEnable = false;
		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		desc.DepthEnable = true;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		Device->CreateDepthStencilState(&desc, &mDepthState);
	}

	// Create geometry phase blend state
	{
		CD3D11_BLEND_DESC desc(D3D11_DEFAULT);
		Device->CreateBlendState(&desc, &mGeometryBlendState);
	}

	// Create deferred shade (direct light) blend state
	{
		// splat photon blend state
		CD3D11_BLEND_DESC* blendDesc = new CD3D11_BLEND_DESC();
		blendDesc->AlphaToCoverageEnable = false;
		blendDesc->IndependentBlendEnable = false;

		D3D11_RENDER_TARGET_BLEND_DESC RTBlendDesc = D3D11_RENDER_TARGET_BLEND_DESC();
		RTBlendDesc.BlendEnable = true;
		RTBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
		RTBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		RTBlendDesc.DestBlend = D3D11_BLEND_ONE;
		RTBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		RTBlendDesc.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		RTBlendDesc.DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
		RTBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		blendDesc->RenderTarget[0] = RTBlendDesc;


		HRESULT hr = Device->CreateBlendState(blendDesc, &mDeferredShadeBlend);
		if (FAILED(hr)) {
			DXUT_ERR_MSGBOX(L"failed to create deferred shade blend state", hr);
		}
	}

	viewerCamera->D3DCreateDevice(Device, BackBufferSurfaceDesc);

	// ### deferred shading direct light shaders ###
	DeferredShadeVS = new VertexShader(Device, L"Shaders\\DeferredShade.hlsl", "DeferredShadeVS");
	DeferredShadePS = new PixelShader(Device, L"Shaders\\DeferredShade.hlsl", "DeferredShadePS");
	DeferredSunShadePS = new PixelShader(Device, L"Shaders\\DeferredShade.hlsl", "DeferredSunShadePS");

	D3D11_SAMPLER_DESC shadowDesc = D3D11_SAMPLER_DESC();
	ZeroMemory(&shadowDesc, sizeof(D3D11_SAMPLER_DESC));
	shadowDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowDesc.BorderColor[0] = shadowDesc.BorderColor[1] =shadowDesc.BorderColor[2] =shadowDesc.BorderColor[3] = 1;
	Device->CreateSamplerState(&shadowDesc, &shadowSampler);


	// ### full screen quad for direct light render ###
	PositionTexCoordVertex vertices[] =
	{
		{ D3DXVECTOR3( -1.0f, 1.0f, 0.0f),	D3DXVECTOR2(0.0f, 0.0f) }, //top left
		{ D3DXVECTOR3(	1.0f, 1.0f, 0.0f),	D3DXVECTOR2(1.0f, 0.0f) }, //top right
		{ D3DXVECTOR3( -1.0f, -1.0f, 0.0f), D3DXVECTOR2(0.0f, 1.0f) }, //bottom left
		{ D3DXVECTOR3( 1.0f, -1.0f, 0.0f),	D3DXVECTOR2(1.0f, 1.0f) }, //bottom right
	};

	D3D11_BUFFER_DESC screenQuadDesc;
	ZeroMemory( &screenQuadDesc, sizeof(D3D11_BUFFER_DESC) );
	screenQuadDesc.Usage = D3D11_USAGE_DEFAULT;
	screenQuadDesc.ByteWidth = sizeof( PositionTexCoordVertex ) * ARRAYSIZE(vertices);
	screenQuadDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	screenQuadDesc.CPUAccessFlags = 0;
	screenQuadDesc.MiscFlags = 0;
	screenQuadDesc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory( &InitData, sizeof(InitData) );
	InitData.pSysMem = vertices;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;
	Device->CreateBuffer( &screenQuadDesc, &InitData, &screenQuadVertices);

	{
		// ## Light constant buffer ##
		D3DXMATRIX identityMatrix;
		D3DXMatrixIdentity(&identityMatrix);
		LightConstants lightConstData;
		lightConstData.lightViewProj = identityMatrix;
		lightConstData.lightPositionAngle = D3DXVECTOR4(0,0,0, D3DX_PI/4);
		lightConstData.lightPower = D3DXVECTOR4(1,1,1,1);
		lightConstData.lightDirectionDistance = D3DXVECTOR4(0,1,0,100.0f);


		D3D11_BUFFER_DESC constantbd = CD3D11_BUFFER_DESC();
		constantbd.Usage = D3D11_USAGE_DEFAULT;
		constantbd.ByteWidth = sizeof( LightConstants );
		constantbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantbd.CPUAccessFlags = 0;
		constantbd.MiscFlags = 0;
		constantbd.StructureByteStride = 0;

		// Fill in the subresource data.
		D3D11_SUBRESOURCE_DATA ConstInitData;
		ConstInitData.pSysMem = &lightConstData;
		ConstInitData.SysMemPitch = 0;
		ConstInitData.SysMemSlicePitch = 0;

		Device->CreateBuffer(&constantbd, &ConstInitData, &lightCBuffer);
	}




	// Debug output stuff for optix
	// --------------------------------
	// Create resources used to depict the viewport quad with the output data from optix.
	float3 verts[] = { make_float3(-1,-1,0), make_float3(-1,1,0), make_float3(1,-1,0), make_float3(1,1,0) };		

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = 4 * 3 * sizeof(float);
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;	

	D3D11_SUBRESOURCE_DATA init;
	ZeroMemory(&init, sizeof(D3D11_SUBRESOURCE_DATA));
	init.pSysMem = verts;

	if (FAILED(Device->CreateBuffer(&desc, &init, &optixViewportQuadVertices))) return false;
	DXUT_SetDebugName(optixViewportQuadVertices, "ViewportQuad_VB");

	optixDebugOutputVS = new VertexShader(Device, L"Shaders\\OptixDebugRender.hlsl", "OptixDebugRenderVS");
	optixDebugOutputPS = new PixelShader(Device, L"Shaders\\OptixDebugRender.hlsl", "OptixDebugRenderPS");
	
	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 }, 
	};

	ID3D10Blob* bytecode = optixDebugOutputVS->GetByteCode();
	if (FAILED(Device->CreateInputLayout(layout, 1, bytecode->GetBufferPointer(), bytecode->GetBufferSize(), &optixInputLayoutViewportQuad))) return false;
	DXUT_SetDebugName(optixInputLayoutViewportQuad, "ViewportQuad_InputLayout");

	// light importance distribution (static)
	mSumLightImportance = 0;
	if(mSunLight)
	{
		mSunLight->SetImportance(SUNLIGHT_QUOTA);
		mSumLightImportance = SUNLIGHT_QUOTA;
	}
	if(mSpotLights.front())
	{
		for (int i = 0; i < mSpotLights.size(); ++i)
		{
			mSpotLights[i]->SetImportance(PHOTONS_PER_SPOTLIGHT_QUOTA);
			mSumLightImportance += PHOTONS_PER_SPOTLIGHT_QUOTA;
		}
	}

	return true;
}

// ================================================================================
void Scene::D3DReleaseDevice()
{
	for (std::vector<RebelGeometry*>::iterator it = _Geometries.begin(); it != _Geometries.end(); ++it)
		(*it)->D3DReleaseDevice();
	for (std::vector<RebelMaterial*>::iterator it = _Materials.begin(); it != _Materials.end(); ++it)
		(*it)->D3DReleaseDevice();
	for (std::vector<SpotLight*>::iterator it = mSpotLights.begin(); it != mSpotLights.end(); ++it)
		(*it)->D3DReleaseDevice();
	if(mSunLight)
		mSunLight->D3DReleaseDevice();
	
	viewerCamera->D3DReleaseDevice();

	SAFE_RELEASE(mMeshVertexLayout);
	SAFE_DELETE(craftMyGBufferVS);
	SAFE_DELETE(craftMyGBufferPS);
	SAFE_DELETE(craftMyLightGBufferVS);
	SAFE_DELETE(craftMyLightGBufferPS);
	SAFE_RELEASE(mRasterizerState);
	SAFE_RELEASE(mDiffuseSampler);
	SAFE_RELEASE(mDepthState);
	SAFE_RELEASE(mGeometryBlendState);
	SAFE_RELEASE(mDeferredShadeBlend);
	SAFE_DELETE(DeferredShadePS);
	SAFE_DELETE(DeferredShadeVS);
	SAFE_DELETE(DeferredSunShadePS);
	SAFE_RELEASE(shadowSampler);


	SAFE_RELEASE(optixViewportQuadVertices);
	SAFE_RELEASE(optixInputLayoutViewportQuad);
	SAFE_DELETE(optixDebugOutputVS);
	SAFE_DELETE(optixDebugOutputPS);
	SAFE_RELEASE(splattingRTV);
	SAFE_RELEASE(splattingSRV);
}

// ================================================================================
bool Scene::OptixCreateDevice(optix::Context& Context)
{

	// Root node of the scene
	_Root = Context->createGroup();	
	_Root->setAcceleration(Context->createAcceleration("MedianBvh", "Bvh"));

	// Group node for all static geometries.
	_StaticGroup = Context->createGeometryGroup();
	_StaticGroup->setAcceleration(Context->createAcceleration("MedianBvh", "Bvh"));
	_StaticGroup->setChildCount(0);
	_Root->setChildCount(1);	// Todo... increase, if dynamic groups are added...
	_Root->setChild(0, _StaticGroup);

	// Group node for all static geometries.
	_DynamicGroup = Context->createGeometryGroup();
	_DynamicGroup->setAcceleration(Context->createAcceleration("MedianBvh", "Bvh"));
	_DynamicGroup->setChildCount(0);

	// Matrices are row-major.
	const float m[16] = {	1, 0, 0, 0,
							0, 1, 0, 0,
							0, 0, 1, 0,
							0, 0, 0, 1 };

	// create Transform node for the dynamic geometry
	_Transform = Context->createTransform();
	_Transform->setMatrix(false, m, 0);
	_Transform->setChild<GeometryGroup>(_DynamicGroup);
	_Root->setChildCount(2);
	_Root->setChild(1, _Transform);




	Context["top_object"]->set(_Root);


	for (std::vector<RebelGeometry*>::iterator it = _Geometries.begin(); it != _Geometries.end(); ++it)
		if (!(*it)->OptixCreateDevice(Context))
			return false;
	for (std::vector<RebelMaterial*>::iterator it = _Materials.begin(); it != _Materials.end(); ++it)
		if (!(*it)->OptixCreateDevice(Context))
			return false;
	for (std::vector<RebelGeometryInstance*>::iterator it = _GeometryInstances.begin(); it != _GeometryInstances.end(); ++it)
		if (!(*it)->OptixCreateDevice(Context))
			return false; 
	for (std::vector<SpotLight*>::iterator it = mSpotLights.begin(); it != mSpotLights.end(); ++it)
		if (!(*it)->OptixCreateDevice(Context))
			return false; 

	if(mSunLight)
		mSunLight->OptixCreateDevice(Context);

	if (!viewerCamera->OptixCreateDevice(Context)) return false;

	return true;
}

// ================================================================================
void Scene::OptixReleaseDevice()
{
	for (std::vector<RebelGeometry*>::iterator it = _Geometries.begin(); it != _Geometries.end(); ++it)
		(*it)->OptixReleaseDevice();
	for (std::vector<RebelMaterial*>::iterator it = _Materials.begin(); it != _Materials.end(); ++it)
		(*it)->OptixReleaseDevice();
	for (std::vector<RebelGeometryInstance*>::iterator it = _GeometryInstances.begin(); it != _GeometryInstances.end(); ++it)
		(*it)->OptixReleaseDevice();
	for (std::vector<SpotLight*>::iterator it = mSpotLights.begin(); it != mSpotLights.end(); ++it)
		(*it)->OptixReleaseDevice();
	if(mSunLight)
		mSunLight->OptixReleaseDevice();

	viewerCamera->OptixReleaseDevice();
}

// ================================================================================
bool Scene::D3DCreateSwapChain(ID3D11Device* d3dDevice, IDXGISwapChain* SwapChain, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc)
{
	if (viewerCamera)
		viewerCamera->SetProjParams(D3DX_PI / 4, (float)BackBufferSurfaceDesc->Width / (float)BackBufferSurfaceDesc->Height, CAMERA_NEAR, CAMERA_FAR);

	mGBufferWidth = BackBufferSurfaceDesc->Width;
	mGBufferHeight = BackBufferSurfaceDesc->Height;

	// Create/recreate any textures related to screen size
	mGBuffer.resize(0);
	mGBufferRTV.resize(0);
	mGBufferSRV.resize(0);

	//light GBuffer
	mLightGBufferWidth = mGBufferWidth; //BOUNCEMAP_WIDTH;
	mLightGBufferHeight = mGBufferHeight; //BOUNCEMAP_HEIGHT;
	mLightGBuffer.resize(0);
	mLightGBufferRTV.resize(0);
	mLightGBufferSRV.resize(0);

	// GBuffer viewercamera
	{
		ID3D11Texture2D *tempTexture;
		CD3D11_TEXTURE2D_DESC texDesc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R16G16B16A16_FLOAT, mGBufferWidth, mGBufferHeight, 1, 1, D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT);
		d3dDevice->CreateTexture2D(&texDesc, 0, &tempTexture);

		//diffuse
		mGBuffer.push_back(tempTexture);

		d3dDevice->CreateTexture2D(&texDesc, 0, &tempTexture);
		//specular
		mGBuffer.push_back(tempTexture);

		d3dDevice->CreateTexture2D(&texDesc, 0, &tempTexture);
		//transmittance
		mGBuffer.push_back(tempTexture);

		d3dDevice->CreateTexture2D(&texDesc, 0, &tempTexture);
		//normal
		mGBuffer.push_back(tempTexture);

		texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		d3dDevice->CreateTexture2D(&texDesc, 0, &tempTexture);
		//normal
		mGBuffer.push_back(tempTexture);
	};

	// ### Set up GBuffer resource list
	mGBufferRTV.resize(mGBuffer.size(), 0);
	mGBufferSRV.resize(mGBuffer.size(), 0);

	D3D11_RENDER_TARGET_VIEW_DESC* rtViewDesc = &D3D11_RENDER_TARGET_VIEW_DESC();
	D3D11_SHADER_RESOURCE_VIEW_DESC* srViewDesc = &D3D11_SHADER_RESOURCE_VIEW_DESC();
	CD3D11_TEXTURE2D_DESC* tempTexDesc = &CD3D11_TEXTURE2D_DESC();
	rtViewDesc->ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	srViewDesc->ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srViewDesc->Texture2D.MipLevels = 1;
	srViewDesc->Texture2D.MostDetailedMip = 0;

	for (std::size_t i = 0; i < mGBuffer.size(); ++i) 
	{
		mGBuffer[i]->GetDesc(tempTexDesc);

		rtViewDesc->Format = tempTexDesc->Format;
		srViewDesc->Format = tempTexDesc->Format;
		d3dDevice->CreateRenderTargetView(mGBuffer[i], rtViewDesc, &mGBufferRTV[i]);
		d3dDevice->CreateShaderResourceView(mGBuffer[i], srViewDesc,&mGBufferSRV[i] );
	}

	//// ### Setup LIGHTCAM DEPTH and GBUFFER ###
	//// standard depth, no stencil buffer <-- do I need a diff depth buffer for the diff size?
	//{
	//	HRESULT hr;

	//	ID3D11Texture2D* tempTexture;
	//	CD3D11_TEXTURE2D_DESC* depthTexDesc =  &CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R32_TYPELESS, mLightGBufferWidth, mLightGBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL, D3D11_USAGE_DEFAULT);
	//	hr = d3dDevice->CreateTexture2D(depthTexDesc, 0, &tempTexture);
	//	if( FAILED(hr) )
	//	{
	//		DWORD dwError = GetLastError();
	//		DXUT_ERR_MSGBOX( L"createDepthTex", HRESULT_FROM_WIN32(dwError) );
	//		return false;
	//	}

	//	CD3D11_DEPTH_STENCIL_VIEW_DESC* depthViewDesc = &CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D);
	//	depthViewDesc->Flags = 0; //D3D11_DSV_READ_ONLY_DEPTH;
	//	depthViewDesc->Format = DXGI_FORMAT_D32_FLOAT;

	//	hr = d3dDevice->CreateDepthStencilView(tempTexture, depthViewDesc, &mDepthBufferLightCamView);

	//	if( FAILED(hr) )
	//	{
	//		DWORD dwError = GetLastError();
	//		DXUT_ERR_MSGBOX( L"Create Depth View", HRESULT_FROM_WIN32(dwError) );
	//		return false;
	//	}
	//}

	//GBuffer light
	{
		ID3D11Texture2D *tempTexture;

		CD3D11_TEXTURE2D_DESC texDesc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R16G16B16A16_FLOAT, mLightGBufferWidth, mLightGBufferHeight, 1, 1, D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT);
		d3dDevice->CreateTexture2D(&texDesc, 0, &tempTexture);

		//diffuse
		mLightGBuffer.push_back(tempTexture);

		//texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		d3dDevice->CreateTexture2D(&texDesc, 0, &tempTexture);
		//specular
		mLightGBuffer.push_back(tempTexture);

		d3dDevice->CreateTexture2D(&texDesc, 0, &tempTexture);
		//transmittance
		mLightGBuffer.push_back(tempTexture);

		d3dDevice->CreateTexture2D(&texDesc, 0, &tempTexture);
		//normal
		mLightGBuffer.push_back(tempTexture);

		texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		d3dDevice->CreateTexture2D(&texDesc, 0, &tempTexture);
		//normal
		mLightGBuffer.push_back(tempTexture);
	};

	// ### Set up GBuffer resource list
	mLightGBufferRTV.resize(mLightGBuffer.size(), 0);
	mLightGBufferSRV.resize(mLightGBuffer.size(), 0);

	for (std::size_t i = 0; i < mLightGBuffer.size(); ++i) {
		mLightGBuffer[i]->GetDesc(tempTexDesc);

		rtViewDesc->Format = tempTexDesc->Format;
		srViewDesc->Format = tempTexDesc->Format;
		d3dDevice->CreateRenderTargetView(mLightGBuffer[i], rtViewDesc, &mLightGBufferRTV[i]);
		d3dDevice->CreateShaderResourceView(mLightGBuffer[i], srViewDesc,&mLightGBufferSRV[i] );
	}

	{
		ID3D11Texture2D *tempTexture;
		tempTexDesc = new CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_D32_FLOAT, mLightGBufferWidth, mLightGBufferHeight);
		tempTexDesc->MipLevels = 1;
		tempTexDesc->BindFlags = D3D11_BIND_DEPTH_STENCIL;
		HRESULT hr = d3dDevice->CreateTexture2D(tempTexDesc, NULL, &tempTexture);
		if(FAILED(hr))
			DXUT_ERR_MSGBOX(L" DEPTH texture not created", hr);
	
		// light depth View
		CD3D11_DEPTH_STENCIL_VIEW_DESC depthDesc = CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D);
		depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	
		hr =d3dDevice->CreateDepthStencilView(tempTexture, &depthDesc, &lightDepthView);
		
		if(FAILED(hr))
			DXUT_ERR_MSGBOX(L"DEPTH View not created", hr);

		SAFE_RELEASE(tempTexture);
	}


	// splatting Render Target
	{
		CD3D11_TEXTURE2D_DESC texDesc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R32G32B32A32_FLOAT, BackBufferSurfaceDesc->Width, BackBufferSurfaceDesc->Height, 1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT);
		d3dDevice->CreateTexture2D(&texDesc, 0, &splattingTexture);

		rtViewDesc->Format = texDesc.Format;
		srViewDesc->Format = texDesc.Format;
		d3dDevice->CreateRenderTargetView(splattingTexture, rtViewDesc, &splattingRTV);
		d3dDevice->CreateShaderResourceView(splattingTexture, srViewDesc, &splattingSRV);
	}



	//raycast output buffer
	{
		// Create the output buffer for optix (Since Optix still requires an CPU_ACCESS_FLAG we need a staging buffer.......)
		D3D11_BUFFER_DESC descOut;
		ZeroMemory(&descOut, sizeof(D3D11_BUFFER_DESC));		
		descOut.ByteWidth = sizeof(float) * BackBufferSurfaceDesc->Width * BackBufferSurfaceDesc->Height;
		descOut.CPUAccessFlags = D3D11_CPU_ACCESS_READ;	
		descOut.Usage = D3D11_USAGE_STAGING;
		HRESULT hr = d3dDevice->CreateBuffer(&descOut, NULL, &d3dRaycastOutputStaging);
		if (FAILED(hr))
		{
			Debug::PrintError("D3D", "Failed to create staging output buffer for Optix.", hr);
			return false;
		}
		DXUT_SetDebugName(d3dRaycastOutputStaging, "OutputBuffer_Staging");

		// Create the output buffer for optix (as byte address buffer).
		descOut.BindFlags = D3D11_BIND_SHADER_RESOURCE;	
		descOut.CPUAccessFlags = 0;
		descOut.StructureByteStride = sizeof(float);
		descOut.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
		descOut.Usage = D3D11_USAGE_DEFAULT;
		hr = d3dDevice->CreateBuffer(&descOut, NULL, &d3dRaycastOutput);
		if (FAILED(hr))
		{
			Debug::PrintError("D3D", "Failed to create output buffer for Optix.", hr);
			return false;
		}	
		DXUT_SetDebugName(d3dRaycastOutput, "OutputBuffer");

		// Create a shader resource view on the output buffer of optix (after staging).
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srvDesc.BufferEx.NumElements = descOut.ByteWidth / descOut.StructureByteStride;    
		srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
		srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		hr = d3dDevice->CreateShaderResourceView(d3dRaycastOutput, &srvDesc, &d3dRaycastOutputSRV);
		if (FAILED(hr))
		{
			Debug::PrintError("D3D", "Failed to create shader resource view on output buffer for Optix.", hr);
			return false;
		}	
		DXUT_SetDebugName(d3dRaycastOutputSRV, "Srv::OutputBuffer");
	}

	return true;
}

// ================================================================================
void Scene::D3DReleaseSwapChain()
{
	for (std::size_t i = 0; i < mGBuffer.size(); ++i) {
		SAFE_RELEASE(mGBuffer[i]);
		SAFE_RELEASE(mGBufferRTV[i]);
		SAFE_RELEASE(mGBufferSRV[i]);
	}
	mGBuffer.clear();
	mGBufferRTV.clear();
	mGBufferSRV.clear();

	for (std::size_t i = 0; i < mLightGBuffer.size(); ++i) {
		SAFE_RELEASE(mLightGBuffer[i]);
		SAFE_RELEASE(mLightGBufferRTV[i]);
		SAFE_RELEASE(mLightGBufferSRV[i]);
	}
	mLightGBuffer.clear();
	mLightGBufferRTV.clear();
	mLightGBufferSRV.clear();

	SAFE_RELEASE(lightDepthView);

	SAFE_RELEASE(d3dRaycastOutput);
	SAFE_RELEASE(d3dRaycastOutputSRV);
	SAFE_RELEASE(d3dRaycastOutputStaging);
}

// ================================================================================
bool Scene::OptixCreateSwapChain(optix::Context& optixContext, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc)
{
	optixRaycastOutput = optixContext->createBufferFromD3D11Resource(RT_BUFFER_OUTPUT, d3dRaycastOutput);
	optixRaycastOutput->setFormat(RT_FORMAT_UNSIGNED_INT);
	optixRaycastOutput->setSize(BackBufferSurfaceDesc->Width, BackBufferSurfaceDesc->Height); // needs to hold all photons, size accordingly
	optixRaycastOutput->validate();
	optixContext["Width"]->setUint(BackBufferSurfaceDesc->Width);
	optixContext["Height"]->setUint(BackBufferSurfaceDesc->Height);
	optixContext["raycast_buffer"]->set(optixRaycastOutput);
	return true;
}

// ================================================================================
void Scene::OptixReleaseSwapChain()
{
	optixRaycastOutput->destroy();
}

// ================================================================================
void Scene::AddGeometry(RebelGeometry* Geometry)
{
	_Geometries.push_back(Geometry);
}

// ================================================================================
void Scene::AddMaterial(RebelMaterial* Material)
{
	_Materials.push_back(Material);
}

// ================================================================================
void Scene::SetCamera(OptixViewerCamera* givenCamera)
{
	viewerCamera = givenCamera;
}

// ================================================================================
void Scene::AddSpotLight(SpotLight* givenSpotLight)
{
	mSpotLights.push_back(givenSpotLight);
}

// ================================================================================
void Scene::CreateGeometryInstance(RebelGeometry* Geometry, RebelMaterial* Material)
{
	RebelGeometryInstance* inst = new RebelGeometryInstance(Geometry, Material, this);
	_GeometryInstances.push_back(inst);
}

// ================================================================================
void Scene::Render(ID3D11DeviceContext* ImmediateContext, const D3D11_VIEWPORT* mainViewPort, float fElapsedTime)
{
	//TODO: also do for one directional light
	//clear backbuffer
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	ImmediateContext->ClearRenderTargetView(pRTV, ClearColor );
	ImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	ImmediateContext->ClearRenderTargetView(splattingRTV, ClearColor );

	// render gbuffer for viewer camera rendering
	if (!viewerCamera) return;

	viewerCamera->GetCameraCBuffer()->UpdateBuffer(ImmediateContext);
	ID3D11Buffer* tempCameraCBuffer = viewerCamera->GetCameraCBuffer()->GetBuffer();
	D3DXVECTOR3 camPos = *viewerCamera->GetEyePt();
	D3DXVECTOR3 camLook = *viewerCamera->GetLookAtPt();

 	ImmediateContext->VSSetConstantBuffers(0, 1, &tempCameraCBuffer);
	ImmediateContext->GSSetConstantBuffers(0, 1, &tempCameraCBuffer);
	ImmediateContext->PSSetConstantBuffers(0, 1, &tempCameraCBuffer);
	ImmediateContext->PSSetSamplers(0, 1, &mDiffuseSampler);
	ImmediateContext->GSSetSamplers(0, 1, &mDiffuseSampler);
	ImmediateContext->PSSetSamplers(1, 1, &shadowSampler);

	double totalTime = 0.0;
	double currentTime = 0.0f;

	currentTime = parentApp->GetTimeSinceLastCall();
	parentApp->textToRender << "between Frame Renders : " << currentTime << "ms \n";
	totalTime += currentTime;

	RenderGBuffer(ImmediateContext, mainViewPort);


	// ### render all the lights

	// SUNLIGHT
	// start rendering all the lights
	if(mSunLight)
	{
		//move sun with camera
		//D3DXVECTOR3 sunPos = D3DXVECTOR3(viewerCamera->GetEyePt()->x, 800.0f, viewerCamera->GetEyePt()->z);
		//mSunLight->SetPosition(sunPos);
		//mSunLight->ResetCamera();
		mSunLight->GetCamera()->GetCameraCBuffer()->UpdateBuffer(ImmediateContext);
		ID3D11Buffer* tempCameraCBuffer = mSunLight->GetCamera()->GetCameraCBuffer()->GetBuffer();

		ImmediateContext->VSSetConstantBuffers(1, 1, &tempCameraCBuffer);
		ImmediateContext->GSSetConstantBuffers(1, 1, &tempCameraCBuffer);
		ImmediateContext->PSSetConstantBuffers(1, 1, &tempCameraCBuffer);

		// render GBuffer
		RenderLightGBuffer(ImmediateContext, mainViewPort, mSunLight->GetCamera());

		// render BounceBuffer
		// - BounceBuffer should be saved together with each light?
		// - or in a vector?
		parentApp->GetPhotonMapper()->CreateBounceBuffer(ImmediateContext, mLightGBufferSRV, lightDepthView, mSunLight, mSumLightImportance);

		//set light constant data
		LightConstants lightConstData;
		lightConstData.lightViewProj = (*mSunLight->GetCamera()->GetViewMatrix()) * (*mSunLight->GetCamera()->GetProjMatrix());
		lightConstData.lightPositionAngle = D3DXVECTOR4(mSunLight->GetPosition(), 0.0f);
		lightConstData.lightPower = mSunLight->GetPower();
		lightConstData.lightDirectionDistance = D3DXVECTOR4(mSunLight->GetDirection(), 1.0f);
		RenderForward(ImmediateContext, mainViewPort, &lightConstData, true);

	}
	currentTime = parentApp->GetTimeSinceLastCall();
	parentApp->textToRender << "sunlight rendering: " << currentTime << "ms \n";
	totalTime += currentTime;

	ImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	if(mSpotLights.front())
	{
		//setup
		SpotLight* currentLight = NULL;
		for(uint i = 0; i < mSpotLights.size(); i++)
		{
			currentLight = mSpotLights[i];
			if(!currentLight)
				continue;

			//if(i==0)
			//{
			//	D3DXVECTOR3 viewerDirection = (*viewerCamera->GetLookAtPt()) - (*viewerCamera->GetEyePt());
			//	D3DXVec3Normalize(&viewerDirection, &viewerDirection); // !!! WARNING !!! D3DXVec3Normalize is broken!!!!

			//	D3DXVECTOR3 lightPos = D3DXVECTOR3(viewerCamera->GetEyePt()->x, currentLight->GetPosition()->y, viewerCamera->GetEyePt()->z);
			//	currentLight->SetPosition(lightPos);
			//	currentLight->ResetCamera();
			//}

			currentLight->GetCamera()->GetCameraCBuffer()->UpdateBuffer(ImmediateContext);
			ID3D11Buffer* tempCameraCBuffer = currentLight->GetCamera()->GetCameraCBuffer()->GetBuffer();

			ImmediateContext->VSSetConstantBuffers(1, 1, &tempCameraCBuffer);
			ImmediateContext->GSSetConstantBuffers(1, 1, &tempCameraCBuffer);
			ImmediateContext->PSSetConstantBuffers(1, 1, &tempCameraCBuffer);

			// render GBuffer
			RenderLightGBuffer(ImmediateContext, mainViewPort, currentLight->GetCamera());

			//currentTime = parentApp->GetTimeSinceLastCall();
			//parentApp->textToRender << "Spotlight #" << i << "GBuffer Creation : " << currentTime << "ms \n";
			//totalTime += currentTime;

			// render BounceBuffer
			// - BounceBuffer should be saved together with each light?
			//// - or in a vector?
			if(i == 0)
				parentApp->GetPhotonMapper()->CreateBounceBuffer(ImmediateContext, mLightGBufferSRV, lightDepthView, currentLight, mSumLightImportance, (bool)(!mSunLight) );
			else
				parentApp->GetPhotonMapper()->CreateBounceBuffer(ImmediateContext, mLightGBufferSRV, lightDepthView, currentLight, mSumLightImportance, false);

			//currentTime = parentApp->GetTimeSinceLastCall();
			//parentApp->textToRender << "Spotlight #" << i << "Bounce Buffer Creation : " << currentTime << "ms \n";
			//totalTime += currentTime;

			//set light constant data
			LightConstants lightConstData;
			lightConstData.lightViewProj = (*currentLight->GetCamera()->GetViewMatrix()) * (*currentLight->GetCamera()->GetProjMatrix());
			lightConstData.lightPositionAngle = D3DXVECTOR4(*currentLight->GetPosition(), currentLight->GetAngle());
			lightConstData.lightPower = currentLight->GetPower();
			lightConstData.lightDirectionDistance = D3DXVECTOR4(currentLight->GetDirection(), currentLight->GetDistance());
			RenderForward(ImmediateContext, mainViewPort, &lightConstData);

	
		}
		
	}

	currentTime = parentApp->GetTimeSinceLastCall();
	parentApp->textToRender << "Spotlights rendering: " << currentTime << "ms \n";
	totalTime += currentTime;


	////as late as possible, but before launching photons
	//parentApp->GetPhotonMapper()->UpdatePhotonCount(ImmediateContext);
	parentApp->GetPhotonMapper()->LaunchPhotons(ImmediateContext, parentApp->GetOptixPhotonMapping()->GetContext());

	currentTime = parentApp->GetTimeSinceLastCall();
	parentApp->textToRender << "Optix photon launch/trace: " << currentTime << "ms \n";
	totalTime += currentTime;
	
	parentApp->GetPhotonMapper()->SplatPhotons(ImmediateContext, mGBufferSRV, splattingRTV);

	//currentTime = parentApp->GetTimeSinceLastCall();
	//parentApp->textToRender << "photon splatting: " << currentTime << "ms \n";
	//totalTime += currentTime;

	ImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	ImmediateContext->RSSetState(mRasterizerState);
	parentApp->GetPhotonMapper()->ToneMapping(ImmediateContext, splattingSRV, screenQuadVertices);

	currentTime = parentApp->GetTimeSinceLastCall();
	parentApp->textToRender << "Splatting & Tone Mapping: " << currentTime << "ms \n";
	totalTime += currentTime;


	//parentApp->RenderTextureToScreen(ImmediateContext, pRTV, new D3DXVECTOR4(0.1f,-0.1f,0.6f,- (0.6f)), mGBufferSRV[0]); //specular
	//parentApp->RenderTextureToScreen(ImmediateContext, pRTV, new D3DXVECTOR4(0.1f,-0.71f,0.6f,- (0.6f)), mGBufferSRV[1]); //diffuse
	//parentApp->RenderTextureToScreen(ImmediateContext, pRTV, new D3DXVECTOR4(0.1f,-1.31f,0.6f,- (0.6f)), mGBufferSRV[2]); //transmissive
	//parentApp->RenderTextureToScreen(ImmediateContext, pRTV, new D3DXVECTOR4(0.7f,-0.1f,0.6f,- (0.6f)), mGBufferSRV[3]); //normal
	//parentApp->RenderTextureToScreen(ImmediateContext, pRTV, new D3DXVECTOR4(0.7f,-0.71f,0.6f,- (0.6f)), mGBufferSRV[4]); //position
	

	parentApp->textToRender << "CPU MS per Frame: " << totalTime << " \n";
	//DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"Stats" );
	//parentApp->textToRender << DXUTGetFrameStats( DXUTIsVsyncEnabled()) << " \n";
	//DXUT_EndPerfEvent();
	parentApp->GetTimeSinceLastCall();
	//	DEBUG
	//parentApp->GetPhotonMapper()->RenderVertexGrid(ImmediateContext, DXUTGetD3D11DepthStencilView(), DXUTGetD3D11RenderTargetView());
	//RenderDirectOptixRaycast(ImmediateContext, mainViewPort);
}

// ================================================================================
optix::Group Scene::GetRoot()
{
	return _Root;
}

// ================================================================================
optix::GeometryGroup Scene::GetStaticGroup()
{
	return _StaticGroup;
}

// ================================================================================
optix::GeometryGroup Scene::GetDynamicGroup()
{
	return _DynamicGroup;
}

// ================================================================================
const std::vector<SpotLight*>& Scene::GetSpotLights() const
{
	return mSpotLights;
}

void Scene::HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	viewerCamera->HandleMessages(hWnd, uMsg, wParam, lParam);
}

void Scene::Move( float elapsedTime )
{
	viewerCamera->FrameMove(elapsedTime);

	// rotate some cubes --> update optix transform matrix and rotation matrix for rendering.
	for (std::vector<RebelGeometryInstance*>::iterator it = _GeometryInstances.begin(); it != _GeometryInstances.end(); ++it)
	{
		if((*it)->GetGeometry()->IsStatic())
			continue;
		
		D3DXMATRIX oldTransform = (*it)->GetTransform();
		D3DXMATRIX newTransform = D3DXMATRIX();
		float angle = (*it)->GetAngle();
		//angle += (D3DX_PI / 128);
		//angle = fmodf(angle, D3DX_PI*2.0f);
		//D3DXMatrixRotationX(&newTransform, angle * elapsedTime);

		angle = angle + 0.01f;
		D3DXMatrixTranslation(&newTransform, 0.0f, 0.0f, sin(angle) * 200.0f);
		(*it)->SetTransform(&newTransform);
		(*it)->SetAngle(angle);

		//float determinant = D3DXMatrixDeterminant(&newTransform);
		_Transform->setMatrix(false, newTransform, NULL);
	}

	
}

OptixViewerCamera* Scene::GetViewerCamera()
{
	return viewerCamera;
}

void Scene::RenderForward( ID3D11DeviceContext* d3dDeviceContext, const D3D11_VIEWPORT* mainViewPort, LightConstants* lightConstData, bool sun)
{
	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();

	D3DXMatrixInverse(&lightConstData->shadowToScreen, NULL, &lightConstData->lightViewProj);
	lightConstData->shadowToScreen *= (*viewerCamera->GetViewMatrix());
	
	d3dDeviceContext->UpdateSubresource(lightCBuffer, 0, NULL, lightConstData, 0, 0);

	d3dDeviceContext->OMSetDepthStencilState(mDepthState, 0);
	d3dDeviceContext->OMSetBlendState(mDeferredShadeBlend, 0, 0xFFFFFFFF);
	d3dDeviceContext->OMSetRenderTargets(1, &splattingRTV, NULL);

	UINT stride = sizeof(PositionTexCoordVertex);
	UINT offset = 0;
	d3dDeviceContext->IASetVertexBuffers(0,1, &screenQuadVertices, &stride, &offset);
	d3dDeviceContext->IASetInputLayout(parentApp->mPositionTexCoordLayout);
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// set shaders
	d3dDeviceContext->VSSetShader(DeferredShadeVS->GetShader(), 0, 0);
	d3dDeviceContext->GSSetShader(NULL, 0, 0);
	if(sun)
		d3dDeviceContext->PSSetShader(DeferredSunShadePS->GetShader(), 0, 0);
	else
		d3dDeviceContext->PSSetShader(DeferredShadePS->GetShader(), 0, 0);

	d3dDeviceContext->RSSetState(mRasterizerState);
	d3dDeviceContext->RSSetViewports(1, mainViewPort);

	// set shader resources
	// light worldposition map (need for shadows)
	d3dDeviceContext->PSSetShaderResources(3, 1, &mLightGBufferSRV[4]);
	//viewer camera gbuffer
	d3dDeviceContext->PSSetShaderResources(4, static_cast<UINT>(mGBufferSRV.size()), &mGBufferSRV.front());

	// set constant buffers
	// camera is already set in Render function
	d3dDeviceContext->PSSetConstantBuffers(4, 1, &lightCBuffer);


	// draw
	d3dDeviceContext->Draw(4,0);

	d3dDeviceContext->OMSetRenderTargets(0, 0, 0);
	d3dDeviceContext->OMSetBlendState(NULL, NULL, 0xffffffff);

	ID3D11ShaderResourceView *const pSRV[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
	d3dDeviceContext->PSSetShaderResources(3, 6, pSRV);
	//d3dDeviceContext->PSSetSamplers(1, 0, 0);
}

void Scene::RenderGBuffer( ID3D11DeviceContext* d3dDeviceContext, const D3D11_VIEWPORT* givenViewPort)
{
	//render with CraftMyGBuffer
#ifdef PROFILING
	ProfileBlock profileBlock(L"GBuffer");
#endif

	//float ClearColor[4] = { 0.2f, 0.1f, 0.8f, 1.0f };
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	d3dDeviceContext->IASetInputLayout(mMeshVertexLayout);

	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	d3dDeviceContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//d3dDeviceContext->ClearDepthStencilView(mDepthBufferView, D3D11_CLEAR_DEPTH, 1.0f, NULL);
	for (std::size_t i = 0; i < mGBufferRTV.size(); ++i) {
		d3dDeviceContext->ClearRenderTargetView(mGBufferRTV[i], ClearColor);
	}
	d3dDeviceContext->OMSetRenderTargets(static_cast<UINT>(mGBufferRTV.size()), &mGBufferRTV.front(), pDSV);
	
	d3dDeviceContext->VSSetShader(craftMyGBufferVS->GetShader(), 0, 0);
	d3dDeviceContext->GSSetShader(0, 0, 0);
	d3dDeviceContext->PSSetShader(craftMyGBufferPS->GetShader(), 0, 0);

	d3dDeviceContext->RSSetState(mRasterizerState);
	d3dDeviceContext->RSSetViewports(1, givenViewPort);

	ID3D11ShaderResourceView *const pSRV[4] = {NULL, NULL, NULL, NULL};
	d3dDeviceContext->PSSetShaderResources(0, 4, pSRV);
	//d3dDeviceContext->PSSetSamplers(0, 1, &mDiffuseSampler);

	d3dDeviceContext->OMSetDepthStencilState(mDepthState, 0);
	d3dDeviceContext->OMSetBlendState(mGeometryBlendState, 0, 0xFFFFFFFF);
	
	// TODO ### RENDER ###
	for (std::vector<RebelGeometryInstance*>::iterator it = _GeometryInstances.begin(); it != _GeometryInstances.end(); ++it)
	{
		if((*it)->GetGeometry()->IsStatic())
			(*it)->RenderToGBuffer(d3dDeviceContext);
		else
		{
			//save old matrix
			D3DXMATRIX oldWorldMatrix = 	viewerCamera->GetCameraCBuffer()->Data.World;
			D3DXMATRIX oldWVPMatrix = 	viewerCamera->GetCameraCBuffer()->Data.WorldViewProjection;

			viewerCamera->GetCameraCBuffer()->Data.World = (*it)->GetTransform();
			viewerCamera->GetCameraCBuffer()->Data.WorldViewProjection = viewerCamera->GetCameraCBuffer()->Data.World * viewerCamera->GetCameraCBuffer()->Data.View * (*viewerCamera->GetProjMatrix());
			viewerCamera->GetCameraCBuffer()->UpdateBuffer(d3dDeviceContext);
			ID3D11Buffer* tempCameraCBuffer = viewerCamera->GetCameraCBuffer()->GetBuffer();

			d3dDeviceContext->VSSetConstantBuffers(0, 1, &tempCameraCBuffer);
			d3dDeviceContext->GSSetConstantBuffers(0, 1, &tempCameraCBuffer);
			d3dDeviceContext->PSSetConstantBuffers(0, 1, &tempCameraCBuffer);
			
			(*it)->RenderToGBuffer(d3dDeviceContext);

			// set to old matrix
			viewerCamera->GetCameraCBuffer()->Data.World = oldWorldMatrix;
			viewerCamera->GetCameraCBuffer()->Data.WorldViewProjection = oldWVPMatrix;
			viewerCamera->GetCameraCBuffer()->UpdateBuffer(d3dDeviceContext);
			tempCameraCBuffer = viewerCamera->GetCameraCBuffer()->GetBuffer();

			d3dDeviceContext->VSSetConstantBuffers(0, 1, &tempCameraCBuffer);
			d3dDeviceContext->GSSetConstantBuffers(0, 1, &tempCameraCBuffer);
			d3dDeviceContext->PSSetConstantBuffers(0, 1, &tempCameraCBuffer);
		}
	}

	// Cleanup (aka make the runtime happy)
	d3dDeviceContext->OMSetRenderTargets(1, &pRTV, pDSV); 
}

void Scene::RenderLightGBuffer( ID3D11DeviceContext* d3dDeviceContext, const D3D11_VIEWPORT* givenViewPort, StaticCamera* lightCam)
{
	//render with CraftMyGBuffer

	//float ClearColor[4] = { 0.2f, 0.1f, 0.8f, 1.0f };
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	d3dDeviceContext->IASetInputLayout(mMeshVertexLayout);

	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	d3dDeviceContext->ClearDepthStencilView(lightDepthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//d3dDeviceContext->ClearDepthStencilView(mDepthBufferView, D3D11_CLEAR_DEPTH, 1.0f, NULL);
	for (std::size_t i = 0; i < mLightGBufferRTV.size(); ++i) {
		d3dDeviceContext->ClearRenderTargetView(mLightGBufferRTV[i], ClearColor);
	}
	d3dDeviceContext->OMSetRenderTargets(static_cast<UINT>(mLightGBufferRTV.size()), &mLightGBufferRTV.front(), lightDepthView);

	d3dDeviceContext->VSSetShader(craftMyLightGBufferVS->GetShader(), 0, 0);
	d3dDeviceContext->GSSetShader(0, 0, 0);
	d3dDeviceContext->PSSetShader(craftMyLightGBufferPS->GetShader(), 0, 0);

	d3dDeviceContext->RSSetState(mRasterizerState);
	d3dDeviceContext->RSSetViewports(1, givenViewPort);

	ID3D11ShaderResourceView *const pSRV[4] = {NULL, NULL, NULL, NULL};
	d3dDeviceContext->PSSetShaderResources(0, 4, pSRV);
	//d3dDeviceContext->PSSetSamplers(0, 1, &mDiffuseSampler);

	d3dDeviceContext->OMSetDepthStencilState(mDepthState, 0);
	d3dDeviceContext->OMSetBlendState(mGeometryBlendState, 0, 0xFFFFFFFF);

	// TODO ### RENDER ###
	for (std::vector<RebelGeometryInstance*>::iterator it = _GeometryInstances.begin(); it != _GeometryInstances.end(); ++it)
	{
		if((*it)->GetGeometry()->IsStatic())
			(*it)->RenderToGBuffer(d3dDeviceContext);
		else
		{
			//save old matrix
			D3DXMATRIX oldWorldMatrix = 	lightCam->GetCameraCBuffer()->Data.World;
			D3DXMATRIX oldWVPMatrix = 	lightCam->GetCameraCBuffer()->Data.WorldViewProjection;

			lightCam->GetCameraCBuffer()->Data.World = (*it)->GetTransform();
			lightCam->GetCameraCBuffer()->Data.WorldViewProjection = lightCam->GetCameraCBuffer()->Data.World * lightCam->GetCameraCBuffer()->Data.View * (*lightCam->GetProjMatrix());
			lightCam->GetCameraCBuffer()->UpdateBuffer(d3dDeviceContext);
			ID3D11Buffer* tempCameraCBuffer = lightCam->GetCameraCBuffer()->GetBuffer();

			d3dDeviceContext->VSSetConstantBuffers(1, 1, &tempCameraCBuffer);
			d3dDeviceContext->GSSetConstantBuffers(1, 1, &tempCameraCBuffer);
			d3dDeviceContext->PSSetConstantBuffers(1, 1, &tempCameraCBuffer);

			(*it)->RenderToGBuffer(d3dDeviceContext);

			// set to old matrix
			lightCam->GetCameraCBuffer()->Data.World = oldWorldMatrix;
			lightCam->GetCameraCBuffer()->Data.WorldViewProjection = oldWVPMatrix;
			lightCam->GetCameraCBuffer()->UpdateBuffer(d3dDeviceContext);
			tempCameraCBuffer = lightCam->GetCameraCBuffer()->GetBuffer();

			d3dDeviceContext->VSSetConstantBuffers(1, 1, &tempCameraCBuffer);
			d3dDeviceContext->GSSetConstantBuffers(1, 1, &tempCameraCBuffer);
			d3dDeviceContext->PSSetConstantBuffers(1, 1, &tempCameraCBuffer);
		}
	}

	// Cleanup (aka make the runtime happy)
	d3dDeviceContext->OMSetRenderTargets(0, NULL, NULL); 
}

void Scene::RenderDirectOptixRaycast( ID3D11DeviceContext* d3dDeviceContext, const D3D11_VIEWPORT* mainViewPort )
{	
	// Clear render target and the depth stencil 
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	d3dDeviceContext->ClearRenderTargetView(pRTV, ClearColor );
	d3dDeviceContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	d3dDeviceContext->OMSetRenderTargets(1, &pRTV, pDSV);

	d3dDeviceContext->RSSetViewports(1, mainViewPort);

	const DXGI_SURFACE_DESC* backBufferDesc = DXUTGetDXGIBackBufferSurfaceDesc();
	parentApp->GetOptixPhotonMapping()->GetContext()->launch(EntryType_OptixViewerCamera, backBufferDesc->Width, backBufferDesc->Height);

// Copy output from optix to d3d readable buffer.
//	d3dDeviceContext->CopyResource(d3dRaycastOutput, d3dRaycastOutputStaging);
	UINT stride = 12;
	UINT offset = 0;
	d3dDeviceContext->IASetVertexBuffers(0,1, &optixViewportQuadVertices, &stride, &offset);
	d3dDeviceContext->IASetInputLayout(optixInputLayoutViewportQuad);

	d3dDeviceContext->VSSetShader(optixDebugOutputVS->GetShader(), NULL, 0);
	d3dDeviceContext->PSSetShader(optixDebugOutputPS->GetShader(), NULL, 0);
	d3dDeviceContext->PSSetShaderResources(0, 1, &d3dRaycastOutputSRV);
	
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	d3dDeviceContext->Draw(4, 0);
}

void Scene::SetSunLight( SunLight* givenSun)
{
	mSunLight = givenSun;
}

const SunLight* Scene::GetSunLight() const
{
	return mSunLight;
}

void Scene::ChangeValue( int change )
{
	ParticleSystem.ChangeGravity(change);
}

