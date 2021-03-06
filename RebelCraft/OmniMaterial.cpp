#include "DXUT.h"
#include "OmniMaterial.h"
#include "Util.h"
#include "ContentLoader.h"

bool OmniMaterial::isInitialized = false;
optix::Program OmniMaterial::closestHit_DirectDiffuse = NULL;
optix::Program OmniMaterial::closestHit_PhotonRay = NULL;

OmniMaterial::OmniMaterial(const optix::float3& Diffuse, const optix::float3& Specular, const optix::float3& Transmissive, float refractiveETA, float specularExponent)
{
	omniCBuffer = new ConstantBuffer<OmniConstants>();
	omniCBuffer->Data.diffuseColor = D3DXVECTOR4(Diffuse.x, Diffuse.y, Diffuse.z, 1.0f);
	omniCBuffer->Data.specularColor = D3DXVECTOR4(Specular.x, Specular.y, Specular.z, specularExponent);
	omniCBuffer->Data.transmissive = D3DXVECTOR4(Transmissive.x, Transmissive.y, Transmissive.z, refractiveETA);
}

OmniMaterial::~OmniMaterial()
{

	D3DReleaseDevice();
	OptixReleaseDevice();
	SAFE_DELETE(omniCBuffer);
}

void OmniMaterial::BindParams(ID3D11DeviceContext* ImmediateContext)
{
	// TODO: Material Constant buffer setzen...
	//_Effect->GetVariableByName("Omni")->AsVector()->SetFloatVector((float*)Util::conv(_Omni).m128_f32);

	ID3D11Buffer* tempCBuffer = omniCBuffer->GetBuffer();
	ImmediateContext->PSSetConstantBuffers(2, 1, &tempCBuffer);
}

bool OmniMaterial::D3DCreateDevice(ID3D11Device* d3dDevice, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc)
{	

	materialVS = new VertexShader(d3dDevice, L"Shaders\\OmniMaterial.hlsl", "OmniMaterialVS");
	materialPS = new PixelShader(d3dDevice, L"Shaders\\OmniMaterial.hlsl", "OmniMaterialPS");

	if(!omniCBuffer->D3DCreateDevice(d3dDevice, BackBufferSurfaceDesc)) return false;

	// Create  mesh input layout
	// isn't there an easier and better way to do this???

	// We need the vertex shader bytecode for this... rather than try to wire that all through the
	// shader interface, just recompile the vertex shader.
	UINT shaderFlags = D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_PACK_MATRIX_ROW_MAJOR;
	ID3D10Blob *bytecode = 0;
	HRESULT hr = D3DX11CompileFromFile(L"Shaders\\OmniMaterial.hlsl", NULL, 0, "OmniMaterialVS", "vs_5_0", shaderFlags, 0, 0, &bytecode, 0, 0);
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

	DXUT_SetDebugName(_InputLayout, "OmniMaterial_InputLayout"); 
	return hr == S_OK;
}

void OmniMaterial::D3DReleaseDevice()
{
	SAFE_DELETE(materialVS);
	SAFE_DELETE(materialPS);
	omniCBuffer->D3DReleaseDevice();

	SAFE_RELEASE(_InputLayout);

}

bool OmniMaterial::OptixCreateDevice(optix::Context& Context)
{
	if (!isInitialized)
	{
		if (!ContentLoader::LoadOptixProgramFromFile("CUDA\\DiffuseMaterial.cu.ptx", "DirectDiffuse_closest_hit", Context, &closestHit_DirectDiffuse)) return false;
		if (!ContentLoader::LoadOptixProgramFromFile("CUDA\\OmniMaterial.cu.ptx", "PhotonRay_closest_hit", Context, &closestHit_PhotonRay)) return false;
		isInitialized = true;
	}
	_Material = Context->createMaterial();
	_Material->setClosestHitProgram(RayType_DirectDiffuse, closestHit_DirectDiffuse);
	_Material->setClosestHitProgram(RayType_PhotonRay, closestHit_PhotonRay);	

	_Material["Diffuse"]->setFloat(omniCBuffer->Data.diffuseColor.x, omniCBuffer->Data.diffuseColor.y, omniCBuffer->Data.diffuseColor.z);
	_Material["Specular"]->setFloat(omniCBuffer->Data.specularColor.x, omniCBuffer->Data.specularColor.y, omniCBuffer->Data.specularColor.z, omniCBuffer->Data.specularColor.w);
	_Material["Transmissive"]->setFloat(omniCBuffer->Data.transmissive.x, omniCBuffer->Data.transmissive.y, omniCBuffer->Data.transmissive.z, omniCBuffer->Data.transmissive.w);
	return true;
}

void OmniMaterial::OptixReleaseDevice()
{

}
