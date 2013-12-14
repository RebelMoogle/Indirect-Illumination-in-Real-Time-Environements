#include "DXUT.h"
#include "DiffuseMaterial.h"
#include "Util.h"
#include "OptixTypes.h"
#include "ContentLoader.h"

bool DiffuseMaterial::isInitialized = false;
optix::Program DiffuseMaterial::closestHit_DirectDiffuse = NULL;
optix::Program DiffuseMaterial::closestHit_PhotonRay = NULL;

DiffuseMaterial::DiffuseMaterial(const optix::float3& Diffuse):
_Diffuse(Diffuse)
{
	diffuseCBuffer = new ConstantBuffer<diffuseConstants>();
	diffuseCBuffer->Data.diffuseColor = D3DXVECTOR4(Diffuse.x, Diffuse.y, Diffuse.z, 1.0f);
}

DiffuseMaterial::~DiffuseMaterial()
{

	D3DReleaseDevice();
	OptixReleaseDevice();
	SAFE_DELETE(diffuseCBuffer);
}

void DiffuseMaterial::BindParams(ID3D11DeviceContext* ImmediateContext)
{
	// TODO: Material Constant buffer setzen...
	//_Effect->GetVariableByName("Diffuse")->AsVector()->SetFloatVector((float*)Util::conv(_Diffuse).m128_f32);

	ID3D11Buffer* tempCBuffer = diffuseCBuffer->GetBuffer();
	ImmediateContext->PSSetConstantBuffers(2, 1, &tempCBuffer);
}

bool DiffuseMaterial::D3DCreateDevice(ID3D11Device* d3dDevice, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc)
{	

	materialVS = new VertexShader(d3dDevice, L"Shaders\\DiffuseMaterial.hlsl", "DiffuseMaterialVS");
	materialPS = new PixelShader(d3dDevice, L"Shaders\\DiffuseMaterial.hlsl", "DiffuseMaterialPS");

	if(!diffuseCBuffer->D3DCreateDevice(d3dDevice, BackBufferSurfaceDesc)) return false;
	
	// Create  mesh input layout
	// isn't there an easier and better way to do this???
	
		// We need the vertex shader bytecode for this... rather than try to wire that all through the
		// shader interface, just recompile the vertex shader.
		UINT shaderFlags = D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_PACK_MATRIX_ROW_MAJOR;
		ID3D10Blob *bytecode = 0;
		HRESULT hr = D3DX11CompileFromFile(L"Shaders\\DiffuseMaterial.hlsl", NULL, 0, "DiffuseMaterialVS", "vs_5_0", shaderFlags, 0, 0, &bytecode, 0, 0);
		if (FAILED(hr)) 
		{
			assert(false);
		}

		const D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		d3dDevice->CreateInputLayout( 
			layout, ARRAYSIZE(layout), 
			bytecode->GetBufferPointer(),
			bytecode->GetBufferSize(), 
			&_InputLayout);

		bytecode->Release();
	
	DXUT_SetDebugName(_InputLayout, "DiffuseMaterial_InputLayout"); 
	return hr == S_OK;
}

void DiffuseMaterial::D3DReleaseDevice()
{
	SAFE_DELETE(materialVS);
	SAFE_DELETE(materialPS);
	diffuseCBuffer->D3DReleaseDevice();

	SAFE_RELEASE(_InputLayout);

}

bool DiffuseMaterial::OptixCreateDevice(optix::Context& Context)
{
	if (!isInitialized)
	{
		if (!ContentLoader::LoadOptixProgramFromFile("CUDA\\DiffuseMaterial.cu.ptx", "DirectDiffuse_closest_hit", Context, &closestHit_DirectDiffuse)) return false;
	//	if (!ContentLoader::LoadOptixProgramFromFile("CUDA\\DiffuseMaterial.cu.ptx", "PhotonRay_closest_hit", Context, &closestHit_PhotonRay)) return false;
		isInitialized = true;
	}
	_Material = Context->createMaterial();
	_Material->setClosestHitProgram(RayType_DirectDiffuse, closestHit_DirectDiffuse);
//	_Material->setClosestHitProgram(RayType_PhotonRay, closestHit_PhotonRay);	
	_Material["Diffuse"]->setFloat(_Diffuse);
	return true;
}

void DiffuseMaterial::OptixReleaseDevice()
{
	
}
