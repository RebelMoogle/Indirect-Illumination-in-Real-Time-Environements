#include "DXUT.h"
#include "RebelMaterial.h"
#include "File.h"
#include "Debug.h"
//#include "BaseCamera.h"

// ================================================================================
RebelMaterial::RebelMaterial() : 
//_Effect(NULL),
_Material(NULL)
{

}

// ================================================================================
RebelMaterial::~RebelMaterial()
{
	
}

// ================================================================================
void RebelMaterial::Bind(ID3D11DeviceContext* ImmediateContext, Scene* Scene)
{	
	ImmediateContext->VSSetShader(materialVS->GetShader(), NULL, 0);
	ImmediateContext->GSSetShader(NULL, NULL, 0);
	ImmediateContext->PSSetShader(materialPS->GetShader(), NULL, 0);
/*	ID3DX11EffectTechnique* technique = _Effect->GetTechniqueByName("Render");
	ID3DX11EffectPass* pass = technique->GetPassByIndex(0);
	pass->Apply(0, ImmediateContext);
	
	const CbViewProjection& cb = Scene->GetMainCamera()->GetCbViewProjection();
	_Effect->GetVariableByName("View")->AsMatrix()->SetMatrix((float*)cb.View.m);
	_Effect->GetVariableByName("Projection")->AsMatrix()->SetMatrix((float*)cb.Projection.m);
	*/
	BindParams(ImmediateContext);

	ImmediateContext->IASetInputLayout(_InputLayout);
}

// ================================================================================
optix::Material RebelMaterial::GetMaterial()
{
	return _Material;
}