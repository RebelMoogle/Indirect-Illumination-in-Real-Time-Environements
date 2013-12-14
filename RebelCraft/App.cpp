#include "DynamicTest.h"
// Copyright John McLaughlin
// have fun with it, and credit me.
// sending me a note you used this, would be awesome. Would be great to know people actually use this!

#include "DXUT.h"
#include <optix_world.h>
#include "App.h"
#include "ColorUtil.h"
#include "ShaderDefines.h"
#include <limits>
#include <sstream>
#include <random>
#include <algorithm>
#include "OptixAPI.h"
#include "CornellSceneBuilder.h"
#include "BoxCitySceneBuilder.h"
#include "DynamicTest.h"
#include "Scene.h"
#include "OptixViewerCamera.h"
#ifdef PROFILING
	#include "Profiler.h"
#endif


using std::tr1::shared_ptr;

App::App():
	mDepthBufferLightCamView(0),
	photonMapper(new PhotonMapping())
{

	mSpotLight = NULL;

	//set camera stuff
	gAspectRatio = 1024.0f / 768.0f; // TODO set differently, (constants) default size

	InitScene();

	//init optix
	optixPhotonMapping = new OptixAPI(this);

	//counter stuff
	{
		LARGE_INTEGER li;
		if(!QueryPerformanceFrequency(&li))
			DXUT_ERR_MSGBOX(L"QueryPerformanceFrequency failed!\n", NULL);

		mCounterFreq = double(li.QuadPart)/1000.0;

		QueryPerformanceCounter(&li);
		mCounterLast = li.QuadPart;
	}

	//init dialogs / text
	//	mSettings.Init(&mDialogMgr);

}

App::~App(void)
{
	D3DReleaseDevice();
	D3DReleaseSwapChain();
	//mSkybox.Destroy();
	//SAFE_RELEASE(mDepthBufferView);
	SAFE_RELEASE(mDepthBufferLightCamView);
	//delete mLightBuffer;

	//SAFE_RELEASE(mPerFrameConstants);

	SAFE_RELEASE(mTextureToScreenCBuffer);
	SAFE_RELEASE(mTextureToScreenVertexBuffer);

	//SAFE_RELEASE(craftMyGBufferChangesCamera);

	//SAFE_RELEASE(mLightingBlendState);
	//SAFE_RELEASE(mGeometryBlendState);
	//SAFE_RELEASE(mDepthState);
	SAFE_RELEASE(mDefaultDepthState);
	SAFE_RELEASE(mNoDepthRasterizerState);
	SAFE_RELEASE(mWireRasterizerState);
	//SAFE_RELEASE(mRasterizerState);
//	SAFE_RELEASE(mMeshVertexLayout);
	SAFE_RELEASE(mPositionTexCoordLayout);

	SAFE_DELETE(mTextureToScreenVS);
	SAFE_DELETE(mTextureToScreenGS);
	SAFE_DELETE(mTextureToScreenPS);

	/*SAFE_DELETE(craftMyGBufferPS);
	SAFE_DELETE(craftMyGBufferVS);*/

	SAFE_DELETE(mSpotLight);
	SAFE_DELETE(photonMapper);
	SAFE_DELETE(optixPhotonMapping);

	//text and stuff
	//SAFE_DELETE(mDialogMgr);
	//SAFE_DELETE(mSettings);
	//SAFE_RELEASE(mText);
}

void App::Move(float elapsedTime)
{

	// Update camera
	sceneBuilder->GetScene()->Move(elapsedTime);

	//if(!mSpotLights.empty())
	//	mSpotLights.front().GetCamera()->FrameMove(elapsedTime);
	//  CAMERA IS STATIC, NO MOVEMENT! (could be set directly, or changed)

	//mSpotLights.front().SetDirection(*gViewerCamera.GetLookAtPt() - *gViewerCamera.GetEyePt());
	//mSpotLights.front().SetPosition(*gViewerCamera.GetEyePt());
	//mTotalTime += elapsedTime;
}

void App::Render(ID3D11Device* pd3dDevice, ID3D11DeviceContext* d3dDeviceContext, 
	const D3D11_VIEWPORT* viewport, float fElapsedTime)
{
	//d3dDeviceContext->OMSetDepthStencilState(mDepthState, 0);
	d3dDeviceContext->RSSetState(mNoDepthRasterizerState);
	d3dDeviceContext->OMSetDepthStencilState(mDefaultDepthState, 0);

	
	sceneBuilder->GetScene()->Render(d3dDeviceContext, viewport, fElapsedTime);

	//DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"Stats");
	//// RENDER TEXT HERE (make RenderText(String)) function
	// TEXT OUTPUT


	//textToRender << L"";
#ifdef PROFILING
	Profiler::GlobalProfiler.EndFrame(this);

	ID3D11RenderTargetView* tempRTV[] = {DXUTGetD3D11RenderTargetView()};
	d3dDeviceContext->OMSetRenderTargets(1, tempRTV, NULL);
	mText->Begin();
	//draw text here
		mText->SetInsertionPos(2,0);
		mText->SetForegroundColor(D3DXCOLOR(0.2f,0.9f,0.4f,1.0f));
		mText->DrawTextLine(textToRender.str().data());
	mText->End();

	d3dDeviceContext->OMSetRenderTargets(0, 0, 0);
#endif // PROFILING
	textToRender.str(L"");
}

void App::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* noFurtherProcessing)
{
	mDialogMgr.MsgProc(hWnd,uMsg,wParam,lParam);

	sceneBuilder->GetScene()->HandleMessages(hWnd, uMsg, wParam, lParam);
}

void App::RenderTextureToScreen(	ID3D11DeviceContext* d3dDeviceContext, ID3D11RenderTargetView* backBuffer, D3DXVECTOR4* destRectangle, //const UIConstants* ui, 
									ID3D11ShaderResourceView* textureSRV )
{
	

	D3D11_MAPPED_SUBRESOURCE mappedConstantBuffer;
	TextureToScreenConstants* constData;
	//d3dDeviceContext->UpdateSubresource( mTextureToScreenCBuffer, 0, NULL, &constData, 0, 0 );
	
	d3dDeviceContext->Map(mTextureToScreenCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedConstantBuffer);
		constData = (TextureToScreenConstants*)mappedConstantBuffer.pData;
	
		constData->mDestRect = D3DXVECTOR4(destRectangle->x, destRectangle->y, destRectangle->z, destRectangle->w);

	d3dDeviceContext->Unmap(mTextureToScreenCBuffer, 0);
	

	d3dDeviceContext->IASetInputLayout(mPositionTexCoordLayout);
	// Set vertex buffer
	UINT stride = sizeof( PositionTexCoordVertex );
	UINT offset = 0;
	d3dDeviceContext->IASetVertexBuffers( 0, 1, &mTextureToScreenVertexBuffer, &stride, &offset );
	d3dDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );

	d3dDeviceContext->VSSetShader(mTextureToScreenVS->GetShader(), NULL, 0 );
	d3dDeviceContext->VSSetConstantBuffers(2, 1, &mTextureToScreenCBuffer);

	d3dDeviceContext->GSSetShader(mTextureToScreenGS->GetShader(), NULL, 0);
	d3dDeviceContext->GSSetConstantBuffers(2, 1, &mTextureToScreenCBuffer);

	d3dDeviceContext->PSSetShader( mTextureToScreenPS->GetShader(), NULL, 0 );
	d3dDeviceContext->PSSetShaderResources( 0, 1, &textureSRV );
	//d3dDeviceContext->PSSetSamplers( 0, 1, &mDiffuseSampler );

	//d3dDeviceContext->OMSetDepthStencilState(mDepthState, 0);
	d3dDeviceContext->OMSetRenderTargets(1, &backBuffer, NULL);
	//d3dDeviceContext->RSSetState(mRasterizerState);
	d3dDeviceContext->Draw(1, 0);

	d3dDeviceContext->OMSetRenderTargets(0, 0, 0);
	ID3D11ShaderResourceView *const pSRV[1] = {NULL};
	d3dDeviceContext->PSSetShaderResources(0, 1, pSRV);
}

void App::InitScene()
{
	//sceneBuilder = new CornellSceneBuilder(this);
	//sceneBuilder = new BoxCitySceneBuilder(this);
	sceneBuilder = new DynamicTest(this);
	if (!sceneBuilder->Init()) return;
}

bool App::OptixCreateDevice( optix::Context& optixContext )
{
	if(!photonMapper->OptixCreateDevice(optixContext))
		return false;

	return sceneBuilder->GetScene()->OptixCreateDevice(optixContext);	
}

void App::OptixReleaseDevice()
{
	sceneBuilder->GetScene()->OptixReleaseDevice();
	photonMapper->OptixReleaseDevice();
}

bool App::OptixCreateSwapChain( optix::Context& optixContext, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc )
{
	return sceneBuilder->GetScene()->OptixCreateSwapChain(optixContext, BackBufferSurfaceDesc);
}

void App::OptixReleaseSwapChain()
{
	sceneBuilder->GetScene()->OptixReleaseSwapChain();
}

bool App::D3DCreateDevice( ID3D11Device* d3dDevice, const DXGI_SURFACE_DESC* backBufferDesc )
{

	//render Texture To Screen
	mTextureToScreenVS = new VertexShader(d3dDevice, L"Shaders\\TextureToScreen.hlsl", "TextureToScreenVS");
	mTextureToScreenGS = new GeometryShader(d3dDevice, L"Shaders\\TextureToScreen.hlsl", "TextureToScreenGS");
	mTextureToScreenPS = new PixelShader(d3dDevice, L"Shaders\\TextureToScreen.hlsl", "TextureToScreenPS");

	// Create  texture to screen input layout
	{
		ID3D10Blob *bytecode = mTextureToScreenVS->GetByteCode();


		const D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{"POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		d3dDevice->CreateInputLayout( 
			layout, ARRAYSIZE(layout), 
			bytecode->GetBufferPointer(),
			bytecode->GetBufferSize(), 
			&mPositionTexCoordLayout);
	}

	// Create standard rasterizer state
	{
		CD3D11_RASTERIZER_DESC desc(D3D11_DEFAULT);

		desc.CullMode = D3D11_CULL_NONE;
		desc.DepthClipEnable = FALSE;
		d3dDevice->CreateRasterizerState(&desc, &mNoDepthRasterizerState);

		desc.CullMode = D3D11_CULL_NONE;
		desc.FillMode = D3D11_FILL_WIREFRAME;
		d3dDevice->CreateRasterizerState(&desc, &mWireRasterizerState);


	}

	{
		CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);

		desc.DepthEnable = true;
		desc.StencilEnable = false;
		desc.DepthFunc = D3D11_COMPARISON_LESS;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		d3dDevice->CreateDepthStencilState(&desc, &mDefaultDepthState);
	}

	
	// ### Render Texture To Screen Buffer and Vertex Buffer
	{
		TextureToScreenConstants GSConstData;
		GSConstData.mDestRect = D3DXVECTOR4(0, 0, 1, 1);

		D3D11_BUFFER_DESC constantbd;
		ZeroMemory( &constantbd, sizeof(constantbd) );
		constantbd.Usage = D3D11_USAGE_DYNAMIC;
		constantbd.ByteWidth = sizeof( TextureToScreenConstants );
		constantbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		constantbd.MiscFlags = 0;
		constantbd.StructureByteStride = 0;

		// Fill in the subresource data.
		D3D11_SUBRESOURCE_DATA ConstInitData;
		ConstInitData.pSysMem = &GSConstData;
		ConstInitData.SysMemPitch = 0;
		ConstInitData.SysMemSlicePitch = 0;

		d3dDevice->CreateBuffer(&constantbd, &ConstInitData, &mTextureToScreenCBuffer);

		// create vertices
		// the full QUAD will be created in the Geometry shader, we only need the upper left corner.
		PositionTexCoordVertex vertices[] =
		{
			{ D3DXVECTOR3( -1.0f, 1.0f, 0.5f), D3DXVECTOR2(0.0f, 0.0f) },
		};

		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( PositionTexCoordVertex );
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		bd.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory( &InitData, sizeof(InitData) );
		InitData.pSysMem = vertices;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;
		d3dDevice->CreateBuffer( &bd, &InitData, &mTextureToScreenVertexBuffer );
	}

	//D3D11_SAMPLER_DESC defaultDesc = D3D11_SAMPLER_DESC();
	//ZeroMemory(&defaultDesc, sizeof(D3D11_SAMPLER_DESC));


	// init photon mapper
	photonMapper->D3DCreateDevice(d3dDevice, backBufferDesc, mPositionTexCoordLayout);

	if(!sceneBuilder->GetScene()->D3DCreateDevice(d3dDevice, backBufferDesc))
		DXUT_ERR_MSGBOX(L"Scene D3D device not created", S_FALSE);

	optixPhotonMapping->Init(d3dDevice);
	if (!OptixCreateDevice(optixPhotonMapping->GetContext())) return false;

	// text helper
	ID3D11DeviceContext* ImmediateContext = DXUTGetD3D11DeviceContext();
	HRESULT hr = mDialogMgr.OnD3D11CreateDevice(d3dDevice, ImmediateContext);
	if (FAILED(hr)) 
	{
		DXUT_ERR_MSGBOX(L"failed to create dialog manager", hr);
		return false;
	}

	//hr = mSettings.OnD3D11CreateDevice(d3dDevice);
	//if (FAILED(hr)) 
	//{
	//	DXUT_ERR_MSGBOX(L"failed to create settings dialog", hr);
	//	return false;
	//}

	mText = new CDXUTTextHelper(d3dDevice, ImmediateContext, &mDialogMgr, 12);
	
	//Query
#ifdef PROFILING
	Profiler::GlobalProfiler.Initialize(d3dDevice,ImmediateContext);
#endif


	return true;
}

void App::D3DReleaseDevice()
{
	//SAFE_RELEASE(defaultSampler);
	OptixReleaseDevice();
	sceneBuilder->GetScene()->D3DReleaseDevice();
	photonMapper->D3DReleaseDevice();
	mDialogMgr.OnD3D11DestroyDevice();
	//mSettings.OnD3D11DestroyDevice();
	SAFE_DELETE(mText);
}

bool App::D3DCreateSwapChain( ID3D11Device* d3dDevice, IDXGISwapChain* SwapChain, const DXGI_SURFACE_DESC* backBufferDesc )
{
	//camera settings
	gAspectRatio = backBufferDesc->Width / (float)backBufferDesc->Height;
	

	//SAFE_RELEASE(mDepthBufferView);
	SAFE_RELEASE(mDepthBufferLightCamView);

	DXGI_SAMPLE_DESC sampleDesc;
	sampleDesc.Count = 1;
	sampleDesc.Quality = 0;


	// random texture
	//create shader resource view from file
	D3DX11_IMAGE_LOAD_INFO loadInfo;
	ZeroMemory( &loadInfo, sizeof(D3DX11_IMAGE_LOAD_INFO) );
	loadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	loadInfo.Format = DXGI_FORMAT_BC1_UNORM;

	ID3D11ShaderResourceView *pSRView = NULL;
	/*D3DX11CreateShaderResourceViewFromFile( d3dDevice, L"Media\\ColorNoiseSpot.png", &loadInfo, NULL, &pSRView, NULL );*/
	D3DX11CreateShaderResourceViewFromFile( d3dDevice, L"Media\\ColorNoise.png", &loadInfo, NULL, &pSRView, NULL );

	//set to context
	ID3D11DeviceContext* pd3dImmediateContext;
	d3dDevice->GetImmediateContext(&pd3dImmediateContext);
	pd3dImmediateContext->VSSetShaderResources( 9, 1, &pSRView);
	pd3dImmediateContext->PSSetShaderResources( 9, 1, &pSRView);
	pd3dImmediateContext->GSSetShaderResources( 9, 1, &pSRView);

	pSRView->Release();
	pd3dImmediateContext->Release(); // decrease use count

	sceneBuilder->GetScene()->D3DCreateSwapChain(d3dDevice, SwapChain, backBufferDesc);

	if (!OptixCreateSwapChain(optixPhotonMapping->GetContext(), backBufferDesc)) return false;

	HRESULT hr = mDialogMgr.OnD3D11ResizedSwapChain(d3dDevice, backBufferDesc);
	if (FAILED(hr)) 
	{
		DXUT_ERR_MSGBOX(L"failed to resize swap chain on dialog manager", hr);
		return false;
	}
	//hr = mSettings.OnD3D11ResizedSwapChain(d3dDevice,backBufferDesc);
	//if (FAILED(hr)) 
	//{
	//	DXUT_ERR_MSGBOX(L"failed to resize swap chain on settings dialog", hr);
	//	return false;
	//}

	return true;
}

void App::D3DReleaseSwapChain()
{
	sceneBuilder->GetScene()->D3DReleaseSwapChain();
	OptixReleaseSwapChain();
	mDialogMgr.OnD3D11ReleasingSwapChain();
}

OptixAPI* App::GetOptixPhotonMapping()
{
	return optixPhotonMapping;
}

PhotonMapping* App::GetPhotonMapper()
{
	return photonMapper;
}

CDXUTTextHelper* App::GetTextHelper()
{
	return mText;
}

double App::GetTimeSinceLastCall()
{
	LARGE_INTEGER li;
	double result = 0.0;
	
	QueryPerformanceCounter(&li);
	result = double(li.QuadPart-mCounterLast)/mCounterFreq;

	mCounterLast = li.QuadPart;

	return result;
}


//void App::RenderText(const WCHAR* textToDisplay, int x, int y, D3DXCOLOR textColor)
//{
//	mText->Begin();
//	mText->SetInsertionPos(x,y);
//	mText->SetForegroundColor(textColor);
//	mText->DrawTextLine(textToDisplay);
//	mText->End();
//	// draw the text you wanted
//}




