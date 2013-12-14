#pragma once

#include "ID3DResource.h"
#include "IOptixResource.h"
#include "Shader.h"
#include "Constants.h"
#include "ConstantBuffer.h"

class App;
class RebelGeometry;
class RebelMaterial;
class RebelGeometryInstance;
class OptixViewerCamera;
class Light;
class SpotLight;
class SunLight;
class StaticCamera;


// Stores all information about the scene, including geometry, instances and materials.
class Scene : public ID3DDeviceResource, public IOptixDeviceResource, public ID3DSwapChainResource, public IOptixSwapChainResource
{
	public:

		// Constructor. Creates an empty scene.
		explicit Scene(App*);
		~Scene();

		// Gets the parent App.
		App* GetApp();

		// Adds a geometry object to the scene.
		void AddGeometry(RebelGeometry* Geometry);
		// Adds a material object to the scene.
		void AddMaterial(RebelMaterial* Material);
		// Creates a geometry instance.
		void CreateGeometryInstance(RebelGeometry* Geometry, RebelMaterial* Material);
		// Adds a camera to the scene.
		void SetCamera(OptixViewerCamera*);
		// Adds a light to the scene.
		void AddSpotLight(SpotLight*);
		void SetSunLight(SunLight*);

		bool D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc);
		void D3DReleaseDevice();
		bool OptixCreateDevice(optix::Context& Context);
		void OptixReleaseDevice();

		bool D3DCreateSwapChain(ID3D11Device* Device, IDXGISwapChain* SwapChain, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc);
		void D3DReleaseSwapChain();
		bool OptixCreateSwapChain(optix::Context& Context, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc);
		void OptixReleaseSwapChain();

		void Render(ID3D11DeviceContext*, const D3D11_VIEWPORT*, float);
		
		optix::Group GetRoot();
		optix::GeometryGroup GetStaticGroup();
		optix::GeometryGroup GetDynamicGroup();

		OptixViewerCamera* GetViewerCamera();

		// Gets the vector of lights.
		const std::vector<SpotLight*>& GetSpotLights() const;
		const SunLight*	GetSunLight() const;
		void HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
		void Move( float elapsedTime );
		void ChangeValue( int change );
private:

		void RenderForward(ID3D11DeviceContext*, const D3D11_VIEWPORT*, LightConstants* lightConstData, bool sun = false);

		// !camera constant buffer needs to be set before calling RenderGBuffer!
		void RenderGBuffer(ID3D11DeviceContext* d3dDeviceContext, const D3D11_VIEWPORT* givenViewport);
		void RenderLightGBuffer(ID3D11DeviceContext*, const D3D11_VIEWPORT* lightViewPort, StaticCamera* lightCam);
		void RenderDirectOptixRaycast(ID3D11DeviceContext*, const D3D11_VIEWPORT*);


		// Parent App.
		App* const parentApp;

		float mSumLightImportance;

		std::vector<RebelGeometry*> _Geometries;
		std::vector<RebelMaterial*> _Materials;
		std::vector<RebelGeometryInstance*> _GeometryInstances;
		OptixViewerCamera* viewerCamera;
		std::vector<SpotLight*> mSpotLights;
		SunLight* mSunLight; // only one sun. :)

		optix::Group _Root;
		optix::GeometryGroup _StaticGroup;
		optix::GeometryGroup _DynamicGroup;
		optix::Transform _Transform;

		// ### GBuffer stuff ###
		VertexShader* craftMyGBufferVS;
		PixelShader* craftMyGBufferPS;

		VertexShader* craftMyLightGBufferVS;
		PixelShader* craftMyLightGBufferPS;
		
		ID3D11InputLayout* mMeshVertexLayout;

		std::vector<ID3D11Texture2D* > mGBuffer;
		//cache of list of RT pointers for GBuffer
		std::vector<ID3D11RenderTargetView*> mGBufferRTV;
		//cache of list of SRV pointers for Gbuffer
		std::vector<ID3D11ShaderResourceView*> mGBufferSRV;
		unsigned int mGBufferWidth;
		unsigned int mGBufferHeight;

		//Light GBuffer
		//light GBuffer
		std::vector< ID3D11Texture2D* > mLightGBuffer;
		//cache of list of RT pointers for GBuffer
		std::vector<ID3D11RenderTargetView*> mLightGBufferRTV;
		//cache of list of SRV pointers for Gbuffer
		std::vector<ID3D11ShaderResourceView*> mLightGBufferSRV;
		unsigned int mLightGBufferWidth;
		unsigned int mLightGBufferHeight;
		ID3D11DepthStencilView* lightDepthView;

		// final Splatting render target
		ID3D11Texture2D* splattingTexture;
		ID3D11RenderTargetView* splattingRTV;
		ID3D11ShaderResourceView* splattingSRV;

		ID3D11RasterizerState* mRasterizerState;
		ID3D11SamplerState* mDiffuseSampler;
		ID3D11DepthStencilState* mDepthState;
		ID3D11BlendState* mGeometryBlendState;
		ID3D11BlendState* mDeferredShadeBlend;
	
		// ### optix stuff ###
		optix::Buffer optixRaycastOutput;
		ID3D11Buffer* d3dRaycastOutput;
		ID3D11Buffer* d3dRaycastOutputStaging;
		ID3D11ShaderResourceView* d3dRaycastOutputSRV;
		
		VertexShader* optixDebugOutputVS;
		PixelShader* optixDebugOutputPS;

		// ### optix viewport quad ###
		ID3D11Buffer* optixViewportQuadVertices;
		ID3D11InputLayout* optixInputLayoutViewportQuad;

		// ### normal fullscreen quad (position and texcoord) ###
		ID3D11Buffer* screenQuadVertices;
		//ID3D11InputLayout* screenQuadLayout;

		// ### direct light shader ###
		ID3D11Buffer* lightCBuffer;
		VertexShader* DeferredShadeVS;
		PixelShader*  DeferredShadePS;
		PixelShader*  DeferredSunShadePS;
		ID3D11SamplerState* shadowSampler;


};
