#include "DXUT.h"
#include "PhotonMapping.h"
#include "ContentLoader.h"
#include "random.h"
#include "OptixTypes.h"
#include "OptixAPI.h"
#ifdef PROFILING
#include "Profiler.h"
#endif // PROFILING


PhotonMapping::PhotonMapping( void ) : photonResultBufferStaging(NULL),
	mPhotonCount(0)
{
	mBounceCBuffer = new ConstantBuffer<bounceInfo>;
}

PhotonMapping::~PhotonMapping( void )
{

}

//creates a Stream Out Buffer with Photons, can be bound directly for further use
//TODO: create multiple Photons per Input Vertex, Amount set by User
void PhotonMapping::CreateBounceBuffer(	ID3D11DeviceContext* d3dDeviceContext, std::vector<ID3D11ShaderResourceView*> gBufferSRV, ID3D11DepthStencilView* depthBufferView, 
										SpotLight* spotlight, float sumLightImportance, bool isFirstLight)
{
	ID3D11Buffer* pBuffers[1];
	pBuffers[0] = d3dPhotonInitialBuffer;
	//update light data
	// Spotlight constant buffer
	LightConstants lightConstData;
	lightConstData.lightViewProj = (*spotlight->GetCamera()->GetViewMatrix()) * (*spotlight->GetCamera()->GetProjMatrix());;
	lightConstData.lightPositionAngle = D3DXVECTOR4(*spotlight->GetPosition(), spotlight->GetAngle());
	lightConstData.lightPower = spotlight->GetPower();
	lightConstData.lightDirectionDistance = D3DXVECTOR4(spotlight->GetDirection(), spotlight->GetDistance());

	d3dDeviceContext->UpdateSubresource(bounceLightConstants, 0, NULL, &lightConstData, 0, 0);
	float test = PHOTONS_PER_SPOTLIGHT_QUOTA ;
	mBounceCBuffer->Data.killPercentage = (1 - clamp(spotlight->GetImportance()/ sumLightImportance, 0.0f, test));
	mBounceCBuffer->UpdateBuffer(d3dDeviceContext);

	ID3D11Buffer* tempBounceBuffer = mBounceCBuffer->GetBuffer();
	d3dDeviceContext->GSSetConstantBuffers(3, 1, &tempBounceBuffer);


	d3dDeviceContext->IASetInputLayout(positionTexCoordLayout);
	// Set vertex buffer
	UINT stride[1] = { sizeof( PositionTexCoordVertex ) };
	UINT offset[1] = { 0 };

	d3dDeviceContext->IASetVertexBuffers( 0, 1, &vertexGrid, stride, offset );
	d3dDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );

	d3dDeviceContext->VSSetShader(bounceBufferVS->GetShader(), NULL, 0 );

	d3dDeviceContext->GSSetShader(bounceBufferGS, NULL, 0);
	d3dDeviceContext->PSSetShader(NULL, NULL, 0);
	// is already set in scene.render
	d3dDeviceContext->GSSetConstantBuffers(4, 1, &bounceLightConstants);
	d3dDeviceContext->GSSetShaderResources(4, static_cast<UINT>(gBufferSRV.size()), &gBufferSRV.front());
	
	//if not 1st spotlight, need to append to SO buffer (all lights in one Buffer)
	if(!isFirstLight)
		offset[0] = -1;
	d3dDeviceContext->SOSetTargets(1, pBuffers, offset);

	//d3dDeviceContext->Begin(photonCountQuery);

	//d3dDeviceContext->PSSetShader(NULL,NULL,NULL);
	//d3dDeviceContext->RSSetState(mRasterizerState);
	d3dDeviceContext->Draw((UINT)PHOTONS_PER_SCENE, 0); // draw the whole buffer ( vertexGrid currently contains as many vertices as photons are to be used)

	//d3dDeviceContext->End(photonCountQuery);

	pBuffers[0] = NULL;
	d3dDeviceContext->SOSetTargets(1, pBuffers, 0);
	ID3D11ShaderResourceView *const pSRV[5] = {NULL, NULL, NULL, NULL, NULL};
	d3dDeviceContext->GSSetShaderResources(4, 5, pSRV);
	SAFE_RELEASE(tempBounceBuffer);

}

void PhotonMapping::CreateBounceBuffer( ID3D11DeviceContext* d3dDeviceContext, std::vector<ID3D11ShaderResourceView*> gBufferSRV, ID3D11DepthStencilView* depthBufferView, 
										SunLight* sunlight, float sumLightImportance )
{
#ifdef PROFILING
	ProfileBlock profileBlockGB(L"Create Bounce Buffer Sun");
#endif // PROFILING

	ID3D11Buffer* pBuffers[1];
	pBuffers[0] = d3dPhotonInitialBuffer;
	//update light data
	// light constant buffer
	LightConstants lightConstData;
	lightConstData.lightViewProj = (*sunlight->GetCamera()->GetViewMatrix()) * (*sunlight->GetCamera()->GetProjMatrix());;
	lightConstData.lightPositionAngle = D3DXVECTOR4(sunlight->GetPosition(), 0.0f);
	lightConstData.lightPower = sunlight->GetPower();
	lightConstData.lightDirectionDistance = D3DXVECTOR4(sunlight->GetDirection(), 1.0f);

	d3dDeviceContext->UpdateSubresource(bounceLightConstants, 0, NULL, &lightConstData, 0, 0);

	mBounceCBuffer->Data.killPercentage = (1 - sunlight->GetImportance() / sumLightImportance);
	mBounceCBuffer->UpdateBuffer(d3dDeviceContext);
	ID3D11Buffer* tempBounceBuffer = mBounceCBuffer->GetBuffer();
	d3dDeviceContext->GSSetConstantBuffers(3, 1, &tempBounceBuffer);

	d3dDeviceContext->IASetInputLayout(positionTexCoordLayout);
	// Set vertex buffer
	UINT stride[1] = { sizeof( PositionTexCoordVertex ) };
	UINT offset[1] = { 0 };

	d3dDeviceContext->IASetVertexBuffers( 0, 1, &vertexGrid, stride, offset );
	d3dDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );

	d3dDeviceContext->VSSetShader(bounceBufferVS->GetShader(), NULL, 0 );

	d3dDeviceContext->GSSetShader(bounceBufferGS, NULL, 0);
	d3dDeviceContext->PSSetShader(NULL, NULL, 0);
	// is already set in scene.render
	d3dDeviceContext->GSSetConstantBuffers(4, 1, &bounceLightConstants);
	d3dDeviceContext->GSSetShaderResources(4, static_cast<UINT>(gBufferSRV.size()), &gBufferSRV.front());

	// SunLight is always first!
	d3dDeviceContext->SOSetTargets(1, pBuffers, offset);

	//d3dDeviceContext->Begin(photonCountQuery);

	//d3dDeviceContext->PSSetShader(NULL,NULL,NULL);
	//d3dDeviceContext->RSSetState(mRasterizerState);
	d3dDeviceContext->Draw((UINT)PHOTONS_PER_SCENE, 0); // draw the whole buffer ( vertexGrid currently contains as many vertices as photons are to be used)

	//d3dDeviceContext->End(photonCountQuery);

	pBuffers[0] = NULL;
	d3dDeviceContext->SOSetTargets(1, pBuffers, 0);
	ID3D11ShaderResourceView *const pSRV[5] = {NULL, NULL, NULL, NULL, NULL};
	d3dDeviceContext->GSSetShaderResources(4, 5, pSRV);
	SAFE_RELEASE(tempBounceBuffer);
}



void PhotonMapping::SplatPhotons(ID3D11DeviceContext* d3dDeviceContext, std::vector<ID3D11ShaderResourceView*> gBufferSRV, ID3D11RenderTargetView* splattingRTV)
{
	// TEST
	//RenderVertexGrid(d3dDeviceContext, depthBufferView, backBuffer);
	//return;

	// use shader to splat photons in photon buffer
	// input:	GBuffer from camera
	//			Photon Buffer as Vertex Buffer
	//
	// output:	Render onto BackBuffer

#ifdef PROFILING
	ProfileBlock profileBlockGB(L"SplatPhotons");
#endif // PROFILING

	//clear rendertarget
	d3dDeviceContext->IASetInputLayout(photonLayout);
	UINT stride = sizeof(PhotonRecord);
	UINT offset = 0;
	d3dDeviceContext->IASetVertexBuffers(0, 1, &d3dPhotonResultBuffer, &stride, &offset);
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	d3dDeviceContext->VSSetShader(splatPhotonsVS->GetShader(), NULL, 0);
	d3dDeviceContext->GSSetShader(splatPhotonsGS->GetShader(), NULL, 0);
	d3dDeviceContext->PSSetShader(splatPhotonsPS->GetShader(), NULL, 0);
	d3dDeviceContext->PSSetShaderResources(4, static_cast<UINT>(gBufferSRV.size()), &gBufferSRV.front());
	d3dDeviceContext->GSSetShaderResources(4, static_cast<UINT>(gBufferSRV.size()), &gBufferSRV.front());


	float blendFactor[] = {1,1,1,1}; 
	ID3D11RenderTargetView* tempRTV[] = {splattingRTV};
	d3dDeviceContext->OMSetRenderTargets(1, tempRTV, DXUTGetD3D11DepthStencilView());
	d3dDeviceContext->OMSetBlendState(splatPhotonsBlendState, blendFactor, 0xffffffff);
	d3dDeviceContext->OMSetDepthStencilState(splatPhotonsDepthState, 0);
	//d3dDeviceContext->RSSetState(splatPhotonsRasterizerState);

	//d3dDeviceContext->Draw(PHOTONS_PER_SCENE * DIFFUSE_PHOTONS_PER_RAY, 0);

	for (int i = 0; i < DIFFUSE_PHOTONS_PER_RAY*2; i++)
	{
		d3dDeviceContext->Draw(PHOTONS_PER_SCENE/2, (PHOTONS_PER_SCENE/2) * i);
	}
	 //mPhotonCount

	d3dDeviceContext->OMSetRenderTargets(0, 0, 0);
	d3dDeviceContext->OMSetBlendState(NULL, NULL, 0xffffffff);
	d3dDeviceContext->OMSetDepthStencilState(NULL, 0);

	//d3dDeviceContext->RSSetState(NULL);

	ID3D11ShaderResourceView *const pSRV[5] = {NULL, NULL, NULL, NULL, NULL};
	d3dDeviceContext->PSSetShaderResources(4, 5, pSRV);
	d3dDeviceContext->GSSetShaderResources(4, 5, pSRV);

}

void PhotonMapping::RenderVertexGrid( ID3D11DeviceContext* d3dDeviceContext, ID3D11DepthStencilView* depthBufferView, ID3D11RenderTargetView* backbuffer)
{
	d3dDeviceContext->IASetInputLayout(positionTexCoordLayout);
	//d3dDeviceContext->IASetInputLayout(photonLayout);
	UINT stride = sizeof(PositionTexCoordVertex);
	//UINT stride = sizeof(Photon);
	UINT offset = 0;
	d3dDeviceContext->IASetVertexBuffers(0, 1, &vertexGrid, &stride, &offset);
	//d3dDeviceContext->IASetVertexBuffers(0, 1, &d3dPhotonInitialBuffer, &stride, &offset);
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	d3dDeviceContext->VSSetShader(renderVertexVS->GetShader(), NULL, 0);
	d3dDeviceContext->GSSetShader(NULL, NULL, 0);
	d3dDeviceContext->PSSetShader(renderVertexPS->GetShader(), NULL, 0);

	d3dDeviceContext->OMSetRenderTargets(1, &backbuffer, NULL);

	d3dDeviceContext->Draw(PHOTONS_PER_SCENE, 0);
	//d3dDeviceContext->DrawAuto();
	d3dDeviceContext->OMSetRenderTargets(0, 0, 0);

}

void PhotonMapping::UpdatePhotonCount(ID3D11DeviceContext* immediateContext)
{
	D3D11_QUERY_DATA_SO_STATISTICS queryDataStats;
	ZeroMemory(&queryDataStats, sizeof(D3D11_QUERY_DATA_SO_STATISTICS));
	while (S_OK != immediateContext->GetData(photonCountQuery, &queryDataStats, sizeof(D3D11_QUERY_DATA_SO_STATISTICS), 0))
	{} // wait

	mPhotonCount = (UINT) queryDataStats.NumPrimitivesWritten;
}

bool PhotonMapping::D3DCreateDevice( ID3D11Device* d3dDevice, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc, ID3D11InputLayout* vertexLayout)
{
	positionTexCoordLayout = vertexLayout;

	//render vertex shaders
	renderVertexVS = new VertexShader(d3dDevice, L"Shaders\\RenderVertex.hlsl", "RenderVertexVS");
	renderVertexPS = new PixelShader(d3dDevice, L"Shaders\\RenderVertex.hlsl", "RenderVertexPS");

	bounceBufferVS = new VertexShader(d3dDevice, L"Shaders\\BounceMap.hlsl", "BounceBufferVS");
	bounceBufferGS = NULL;
	//bounceBufferGS = new GeometryShader(d3dDevice, L"Shaders\\BounceMap.hlsl", "BounceBufferGS");
	//create Geometry Shader with stream output
	{
		// We need the vertex shader bytecode for this... rather than try to wire that all through the
		// shader interface, just recompile the vertex shader.
		UINT shaderFlags = D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_PACK_MATRIX_ROW_MAJOR;

#if defined( DEBUG ) || defined( _DEBUG )
		shaderFlags |= D3D10_SHADER_DEBUG;
#endif

		ID3D10Blob *bytecode = 0;
		ID3D10Blob *errors = 0;
		HRESULT hr = D3DX11CompileFromFile( L"Shaders\\BounceMap.hlsl", NULL, 0, "BounceBufferGS", "gs_5_0", shaderFlags, 0, 0, &bytecode, &errors, 0);
		if (errors) {
			OutputDebugStringA(static_cast<const char *>(errors->GetBufferPointer()));
		}

		if (FAILED(hr)) {
			DXUT_ERR_MSGBOX(L"failed to compile Bounce Buffer Geometry Shader", hr);
		}
		D3D11_SO_DECLARATION_ENTRY BounceBufferOutputDecl[] =
		{
			// stream, sem name, sem index, start component, component count, output slot
			{ 0, "SV_POSITION", 0, 0, 4, 0},
			{ 0, "TEXCOORD", 0, 0, 4, 0},
			{ 0, "POWER", 0, 0, 4, 0},
			{ 0, "DIRECTION", 0, 0, 3, 0},
			{ 0, "DISTANCE", 0, 0, 1, 0},
			{ 0, "NORMAL", 0, 0, 4, 0},
		};
		UINT strides[] = { (4 + 4 + 4 + 3 + 1 + 4)  * sizeof(float) };

		hr = d3dDevice->CreateGeometryShaderWithStreamOutput(	bytecode->GetBufferPointer(), 
			bytecode->GetBufferSize(), 
			BounceBufferOutputDecl, 
			ARRAYSIZE(BounceBufferOutputDecl), 
			strides, 
			1, 
			D3D11_SO_NO_RASTERIZED_STREAM, 
			NULL, 
			&bounceBufferGS );

		if (FAILED(hr)) {
			DXUT_ERR_MSGBOX(L"failed to create Bounce Buffer Geometry Shader with Stream Output", hr);
		}

		bytecode->Release();
	}

	mBounceCBuffer->D3DCreateDevice(d3dDevice, BackBufferSurfaceDesc);

	// ### splat photons shader ###

	// load shaders
	splatPhotonsVS = new VertexShader(d3dDevice, L"Shaders\\SplatPhotons.hlsl", "SplatVertexShader");
	splatPhotonsGS = new GeometryShader(d3dDevice, L"Shaders\\SplatPhotons.hlsl", "SplatGeometryShader");
	splatPhotonsPS = new PixelShader(d3dDevice, L"Shaders\\SplatPhotons.hlsl", "SplatPixelShader");

	// create photon record layout
	{
		const D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"POWER",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"DIRECTION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},			
			{"DISTANCE",	0, DXGI_FORMAT_R32_FLOAT,		0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},

		};
		ID3D10Blob* bytecode = splatPhotonsVS->GetByteCode();
		HRESULT hr = d3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), bytecode->GetBufferPointer(), bytecode->GetBufferSize(), &photonLayout);
		if (FAILED(hr)) {
			DXUT_ERR_MSGBOX(L"failed to create photon input layout (splatphotons)", hr);
		}
	}

	// splat photon blend state
	CD3D11_BLEND_DESC splatBlendDesc = CD3D11_BLEND_DESC();
	splatBlendDesc.AlphaToCoverageEnable = false;
	splatBlendDesc.IndependentBlendEnable = false;

	D3D11_RENDER_TARGET_BLEND_DESC splatRTBlendDesc = D3D11_RENDER_TARGET_BLEND_DESC();
	splatRTBlendDesc.BlendEnable = true;
	splatRTBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
	splatRTBlendDesc.SrcBlend = D3D11_BLEND_ONE;
	splatRTBlendDesc.DestBlend = D3D11_BLEND_ONE;
	splatRTBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	splatRTBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	splatRTBlendDesc.DestBlendAlpha = D3D11_BLEND_ONE;
	splatRTBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	splatBlendDesc.RenderTarget[0] = splatRTBlendDesc;

	HRESULT hr = d3dDevice->CreateBlendState(&splatBlendDesc, &splatPhotonsBlendState);
	if (FAILED(hr)) {
		DXUT_ERR_MSGBOX(L"failed to create splat blend state", hr);
	}

	CD3D11_DEPTH_STENCIL_DESC splatDepthDesc = CD3D11_DEPTH_STENCIL_DESC();
	splatDepthDesc.StencilEnable = false;
	splatDepthDesc.DepthEnable = true;
	splatDepthDesc.DepthFunc = D3D11_COMPARISON_LESS;
	splatDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

	hr = d3dDevice->CreateDepthStencilState(&splatDepthDesc, &splatPhotonsDepthState);

	if (FAILED(hr)) {
		DXUT_ERR_MSGBOX(L"failed to create splat depth state", hr);
	}


	D3D11_RASTERIZER_DESC rasterDesc = D3D11_RASTERIZER_DESC();
	ZeroMemory(&rasterDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthClipEnable = TRUE;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = FALSE;

	hr = d3dDevice->CreateRasterizerState(&rasterDesc, &splatPhotonsRasterizerState);

	if (FAILED(hr)) {
		DXUT_ERR_MSGBOX(L"failed to create splat rasterizer state", hr);
	}

	// ### tone mapping shader
	toneMappingVS = new VertexShader(d3dDevice, L"Shaders\\ToneMapping.hlsl", "ToneMappingVS");
	toneMappingPS = new PixelShader(d3dDevice, L"Shaders\\ToneMapping.hlsl", "ToneMappingPS");

	// ### QUERY object ###

	CD3D11_QUERY_DESC queryDesc = CD3D11_QUERY_DESC(D3D11_QUERY_SO_STATISTICS_STREAM0);
	hr = d3dDevice->CreateQuery(&queryDesc, &photonCountQuery);
	if (FAILED(hr)) {
		DXUT_ERR_MSGBOX(L"failed to create photon count query", hr);
	}

	// ### Bouncemaps ###
	// TODO DELETE
	CD3D11_TEXTURE2D_DESC texDesc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R32G32B32A32_FLOAT, BOUNCEMAP_HEIGHT, BOUNCEMAP_WIDTH, 1, 1, D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT);
	hr = d3dDevice->CreateTexture2D(&texDesc, 0, &powerDistance);
	if (FAILED(hr)) {
		DXUT_ERR_MSGBOX(L"failed to create power Distance Tex2D", hr);
	}
	d3dDevice->CreateTexture2D(&texDesc, 0, &outgoingDensity);
	if (FAILED(hr)) {
		DXUT_ERR_MSGBOX(L"failed to create Outgoing Density Tex2D", hr);
	}

	D3D11_RENDER_TARGET_VIEW_DESC* rtViewDesc = &D3D11_RENDER_TARGET_VIEW_DESC();
	rtViewDesc->ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtViewDesc->Format = texDesc.Format;

	D3D11_SHADER_RESOURCE_VIEW_DESC* srViewDesc = &D3D11_SHADER_RESOURCE_VIEW_DESC();
	srViewDesc->ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srViewDesc->Texture2D.MipLevels = 1;
	srViewDesc->Texture2D.MostDetailedMip = 0;
	srViewDesc->Format = texDesc.Format;

	//power and distance RT
	d3dDevice->CreateRenderTargetView(powerDistance, rtViewDesc, &powerDistanceRTV);
	d3dDevice->CreateShaderResourceView(powerDistance, srViewDesc,&powerDistanceSRV);

	// outgoing dir and path density
	d3dDevice->CreateRenderTargetView(outgoingDensity, rtViewDesc, &outgoingDensityRTV);
	d3dDevice->CreateShaderResourceView(outgoingDensity, srViewDesc, &outgoingDensitySRV);

	// ## destination Rectangle constant data ##
	TextureToScreenConstants GSConstData;
	GSConstData.mDestRect = D3DXVECTOR4(0.0f, 0.0f, 2.0f, -2.0f);//0, -1.0f, 2, 2);

	D3D11_BUFFER_DESC constantbd;
	ZeroMemory( &constantbd, sizeof(D3D11_BUFFER_DESC) );
	constantbd.Usage = D3D11_USAGE_DEFAULT;
	constantbd.ByteWidth = sizeof( TextureToScreenConstants );
	constantbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantbd.CPUAccessFlags = 0;
	constantbd.MiscFlags = 0;
	constantbd.StructureByteStride = 0;

	// Fill in the subresource data.
	D3D11_SUBRESOURCE_DATA ConstInitData;
	ConstInitData.pSysMem = &GSConstData;
	ConstInitData.SysMemPitch = 0;
	ConstInitData.SysMemSlicePitch = 0;

	d3dDevice->CreateBuffer(&constantbd, &ConstInitData, &quadConstants);


	// ## Spotlight constant buffer ##
	{
		D3DXMATRIX identityMatrix;
		D3DXMatrixIdentity(&identityMatrix);
		LightConstants lightConstData;
		lightConstData.lightViewProj = identityMatrix;
		lightConstData.lightPositionAngle = D3DXVECTOR4(0,0,0,D3DX_PI / 4);
		lightConstData.lightPower = D3DXVECTOR4(1,1,1,1);
		lightConstData.lightDirectionDistance = D3DXVECTOR4(0,1,0,100.0f);
	
		ZeroMemory( &constantbd, sizeof(D3D11_BUFFER_DESC) );
		constantbd.Usage = D3D11_USAGE_DEFAULT;
		constantbd.ByteWidth = sizeof( LightConstants );
		constantbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantbd.CPUAccessFlags = 0;
		constantbd.MiscFlags = 0;
		constantbd.StructureByteStride = 0;
	
		// Fill in the subresource data.
		ConstInitData.pSysMem = &lightConstData;
		ConstInitData.SysMemPitch = 0;
		ConstInitData.SysMemSlicePitch = 0;
	
		d3dDevice->CreateBuffer(&constantbd, &ConstInitData, &bounceLightConstants);
	}

	// ### Create Vertex Grid for BounceBuffer ###

	// fill vertex array with regularly spaced vertices
	PositionTexCoordVertex tempVertex;

	// photons per width = sqrt(photonCount * aspectRatio)
	// photons per height = photonsPerWidth/aspectRatio

	// step size = width / photonsPerWIDTH; from 0 to 1
	UINT photonsPerWidth = sqrtf(PHOTONS_PER_SCENE * (BOUNCEMAP_WIDTH / BOUNCEMAP_HEIGHT));
	UINT photonsPerHeight = photonsPerWidth / (BOUNCEMAP_WIDTH / BOUNCEMAP_HEIGHT);
	//float stepSize =  BOUNCEMAP_WIDTH / photonsPerWidth; // aspect Ratio is 1.0f atm, since all CONSTANT, should be replaced by compiler
	float stepSizeW = 2.0f / (float)photonsPerWidth;
	float stepSizeH = 2.0f / (float)photonsPerHeight;// Screen Space width is always 2: [-1, 1]
	PositionTexCoordVertex* gridVertices = new PositionTexCoordVertex[PHOTONS_PER_SCENE];

	for (UINT i = 0; i < PHOTONS_PER_SCENE; i++)
	{
		float wStep = (i % photonsPerWidth);
		float hStep = floorf(i / photonsPerWidth);

		tempVertex.position = D3DXVECTOR3( -1.0f + (wStep * stepSizeW) + 0.01f, 1.0f - (hStep * stepSizeH)+ 0.01f, 0.0f) ; // screen coordinates are [-1,1]: stepsize * 2.0f (width)
		tempVertex.TexCoord = D3DXVECTOR2( wStep * stepSizeW * 0.5, hStep * stepSizeH * 0.5); //texCoords should go from (0,0) to (1,1)
		gridVertices[i] =  tempVertex;
	}
	PositionTexCoordVertex temp = gridVertices[PHOTONS_PER_SCENE - 48];

	D3D11_BUFFER_DESC bd = D3D11_BUFFER_DESC();
	ZeroMemory( &bd, sizeof(D3D11_BUFFER_DESC) );
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(PositionTexCoordVertex) * PHOTONS_PER_SCENE;//sizeof( gridVertices );
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA InitData = D3D11_SUBRESOURCE_DATA();
	ZeroMemory( &InitData, sizeof(D3D11_SUBRESOURCE_DATA) );
	InitData.pSysMem = gridVertices;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;
	d3dDevice->CreateBuffer( &bd, &InitData, &vertexGrid );

	SAFE_DELETE_ARRAY(gridVertices);


	/// ### create Photon Buffer ###
	ZeroMemory( &bd, sizeof(D3D11_BUFFER_DESC) );
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(PhotonInitialBounce) * PHOTONS_PER_SCENE;//sizeof( gridVertices );
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	// has no INIT DATA, is filled in Geometry Shader
	// used as input for OPTIX
	hr = d3dDevice->CreateBuffer(&bd, NULL, &d3dPhotonInitialBuffer);
	if (FAILED(hr)) {
		DXUT_ERR_MSGBOX(L"failed to create photon output buffer (initial bounce)", hr);
	}

	bd.ByteWidth = sizeof(PhotonRecord) * DIFFUSE_PHOTONS_PER_RAY * PHOTONS_PER_SCENE;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	hr = d3dDevice->CreateBuffer(&bd, NULL, &d3dPhotonResultBuffer);
	if (FAILED(hr)) {
		DXUT_ERR_MSGBOX(L"failed to create photon output buffer (result bounce)", hr);
		return false;
	}

	// RESULT buffer clearing buffer
	ZeroMemory( &bd, sizeof(D3D11_BUFFER_DESC) );
	bd.ByteWidth = sizeof(PhotonRecord) * PHOTONS_PER_SCENE * DIFFUSE_PHOTONS_PER_RAY;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	bd.Usage = D3D11_USAGE_STAGING;
	PhotonRecord* noData = new PhotonRecord[PHOTONS_PER_SCENE*DIFFUSE_PHOTONS_PER_RAY];
	ZeroMemory(noData, bd.ByteWidth);

	ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
	InitData.pSysMem = noData;
	hr = d3dDevice->CreateBuffer(&bd, &InitData, &photonResultBufferStaging);
	if (FAILED(hr)) {
		DXUT_ERR_MSGBOX(L"failed to create photon output buffer (result bounce staging)", hr);
		return false;
	}

	SAFE_DELETE_ARRAY(noData);


	// INITIAL buffer clearing buffer
	ZeroMemory( &bd, sizeof(D3D11_BUFFER_DESC) );
	bd.ByteWidth = sizeof(PhotonInitialBounce) * PHOTONS_PER_SCENE;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	bd.Usage = D3D11_USAGE_STAGING;
	PhotonInitialBounce* noInitData = new PhotonInitialBounce[PHOTONS_PER_SCENE];
	ZeroMemory(noInitData, bd.ByteWidth);

	ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
	InitData.pSysMem = noInitData;
	hr = d3dDevice->CreateBuffer(&bd, &InitData, &photonInitialBufferStaging);
	if (FAILED(hr)) {
		DXUT_ERR_MSGBOX(L"failed to create photon output buffer (initial bounce staging)", hr);
		return false;
	}
	SAFE_DELETE_ARRAY(noInitData);
	
	return true;
}

void PhotonMapping::D3DReleaseDevice()
{
	mBounceCBuffer->D3DReleaseDevice();

	SAFE_RELEASE(powerDistance);
	SAFE_RELEASE(powerDistanceRTV);
	SAFE_RELEASE(powerDistanceSRV);

	SAFE_RELEASE(outgoingDensity);
	SAFE_RELEASE(outgoingDensityRTV);
	SAFE_RELEASE(outgoingDensitySRV);

	SAFE_RELEASE(bounceBufferGS);
	SAFE_RELEASE(vertexGrid);
	
	SAFE_RELEASE(quadConstants);
	SAFE_RELEASE(bounceLightConstants);

	SAFE_DELETE(splatPhotonsVS);
	SAFE_DELETE(splatPhotonsGS);
	SAFE_DELETE(splatPhotonsPS);
	SAFE_RELEASE(splatPhotonsBlendState);
	SAFE_RELEASE(splatPhotonsDepthState);
	SAFE_RELEASE(splatPhotonsRasterizerState);

	SAFE_DELETE(renderVertexVS);
	SAFE_DELETE(renderVertexPS);

	SAFE_DELETE(bounceBufferVS);

	SAFE_RELEASE(photonCountQuery);

	SAFE_RELEASE(d3dPhotonResultBuffer);
	SAFE_RELEASE(d3dPhotonInitialBuffer);
	SAFE_RELEASE(photonResultBufferStaging);
}

bool PhotonMapping::OptixCreateDevice( optix::Context& Context )
{
	if(!ContentLoader::LoadOptixProgramFromFile("CUDA\\PhotonEmitter.cu.ptx", "photon_emitter", Context, &optixPhotonEmitter)) return false;
	Context->setRayGenerationProgram(EntryType_PhotonEmitter, optixPhotonEmitter);

	optixPhotonBuffer = Context->createBufferFromD3D11Resource(RT_BUFFER_INPUT_OUTPUT, d3dPhotonInitialBuffer); // photonResultBufferStaging falls das schief geht
	optixPhotonBuffer->setFormat(RT_FORMAT_USER);
	optixPhotonBuffer->setElementSize(sizeof(PhotonInitialBounce));
	optixPhotonBuffer->setSize(PHOTONS_PER_SCENE);
	optixPhotonBuffer->validate();
	optixPhotonEmitter["photonInitialBuffer"]->set(optixPhotonBuffer);

	optixPhotonResultBuffer = Context->createBufferFromD3D11Resource(RT_BUFFER_OUTPUT, d3dPhotonResultBuffer);
	optixPhotonResultBuffer->setFormat(RT_FORMAT_USER);
	optixPhotonResultBuffer->setElementSize(sizeof(PhotonRecord));
	optixPhotonResultBuffer->setSize(DIFFUSE_PHOTONS_PER_RAY * PHOTONS_PER_SCENE);
	optixPhotonResultBuffer->validate();
	Context["photonResultBuffer"]->set(optixPhotonResultBuffer);

	
	// Buffer with random values.
	optixRandomSeeds = Context->createBuffer( RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_UNSIGNED_INT2, PHOTONS_PER_SCENE );
	optixPhotonEmitter["randomSeeds"]->set( optixRandomSeeds );
	optix::uint2* seeds = reinterpret_cast<optix::uint2*>( optixRandomSeeds->map() );
	for ( unsigned int i = 0; i < PHOTONS_PER_SCENE; ++i )  
		seeds[i] = random2u();
	optixRandomSeeds->unmap();
	optixRandomSeeds->validate();

	return true;
}

void PhotonMapping::OptixReleaseDevice()
{
	optixPhotonResultBuffer->destroy();
	optixPhotonBuffer->destroy();
	optixRandomSeeds->destroy();
}

void PhotonMapping::LaunchPhotons(ID3D11DeviceContext* d3dDeviceContext, optix::Context &Context )
{
//	d3dDeviceContext->Flush();
//	d3dDeviceContext->CopyResource(photonResultBufferStaging, d3dPhotonInitialBuffer);

	// TODO: Query Photon Count
#ifdef PROFILING
	ProfileBlock profileBlockGB(L"launch photons");
#endif // PROFILING

	d3dDeviceContext->CopyResource(d3dPhotonResultBuffer, photonResultBufferStaging);
	Context->launch(EntryType_PhotonEmitter, PHOTONS_PER_SCENE);//mPhotonCount);
	d3dDeviceContext->CopyResource(d3dPhotonInitialBuffer, photonInitialBufferStaging);
}

void PhotonMapping::ToneMapping( ID3D11DeviceContext* d3dDeviceContext, ID3D11ShaderResourceView* inputRender, ID3D11Buffer* quadVertices)
{
	d3dDeviceContext->IASetInputLayout(positionTexCoordLayout);
	UINT stride = sizeof(PositionTexCoordVertex);
	UINT offset = 0;
	d3dDeviceContext->IASetVertexBuffers(0, 1, &quadVertices, &stride, &offset);
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	d3dDeviceContext->VSSetShader(toneMappingVS->GetShader(), NULL, 0);
	d3dDeviceContext->GSSetShader(NULL, NULL, 0);
	d3dDeviceContext->PSSetShader(toneMappingPS->GetShader(), NULL, 0);
	d3dDeviceContext->PSSetShaderResources(0, 1, &inputRender);

	ID3D11RenderTargetView* tempRTV[] = {DXUTGetD3D11RenderTargetView()};
	d3dDeviceContext->OMSetRenderTargets(1, tempRTV, NULL);
	d3dDeviceContext->OMSetBlendState(NULL, NULL, 0xffffffff);

	d3dDeviceContext->Draw(4, 0); //quad

	d3dDeviceContext->OMSetRenderTargets(0, 0, 0);
	d3dDeviceContext->OMSetBlendState(NULL, NULL, 0xffffffff);

	ID3D11ShaderResourceView* pSRV[] = {NULL};
	d3dDeviceContext->PSSetShaderResources(0, 1, pSRV);
}



