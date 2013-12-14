#include "DXUT.h"
#include "RebelGeometryInstance.h"
#include "RebelMaterial.h"
#include "RebelGeometry.h"
#include "Scene.h"
#include "Debug.h"

// ================================================================================
RebelGeometryInstance::RebelGeometryInstance(RebelGeometry* Geometry, RebelMaterial* Material, Scene* Scene) : 
_Geometry(Geometry),
_Material(Material),
_Scene(Scene),
_GeometryInstance(NULL),
_GeometryGroup(NULL),
_Acceleration(NULL)
{
	mAngle = 0.0f;
}

// ================================================================================
RebelGeometryInstance::~RebelGeometryInstance()
{
	OptixReleaseDevice();
}

// ================================================================================
void RebelGeometryInstance::Render(ID3D11DeviceContext* ImmediateContext)
{
	// Bind Shader and Input Layout
	_Material->Bind(ImmediateContext, _Scene);

	// Bind VBO and IBO
	_Geometry->Bind(ImmediateContext);	

	// Submit draw call
	_Geometry->Render(ImmediateContext);
}

// ================================================================================
bool RebelGeometryInstance::OptixCreateDevice(optix::Context& Context)
{
	if (!_Geometry->HasOptixResource()) return true;

	optix::Material mat = _Material->GetMaterial();
	_GeometryInstance = Context->createGeometryInstance(_Geometry->GetGeometry(), &mat, &mat+1);

	optix::GeometryGroup parent;
	if (_Geometry->IsStatic())
		parent = _Scene->GetStaticGroup();
	else 
	{
		// Create an own geometry group for each dynamic object and use a fast accelleration builder.
		parent = _Scene->GetDynamicGroup();
	}

	UINT index = parent->getChildCount();
	parent->setChildCount(index+1);
	parent->setChild(index, _GeometryInstance);

	return true;
}

// ================================================================================
void RebelGeometryInstance::OptixReleaseDevice()
{
	if (_GeometryInstance)
	{
		_GeometryInstance->destroy();
	}
	//if(_Geometry)
	//{
	//	_Geometry->OptixReleaseDevice();
	//	SAFE_DELETE(_Geometry);
	//}
}

void RebelGeometryInstance::RenderToGBuffer( ID3D11DeviceContext* d3dDeviceContext )
{
	// Bind VBO and IBO
	_Geometry->Bind(d3dDeviceContext);
	_Material->BindParams(d3dDeviceContext);

	// Submit draw call
	_Geometry->Render(d3dDeviceContext);
}

const RebelGeometry* RebelGeometryInstance::GetGeometry() const
{
	return _Geometry;
}

const D3DXMATRIX& RebelGeometryInstance::GetTransform() const
{
	return mTransform;
}

void RebelGeometryInstance::SetTransform( D3DXMATRIX* newTransform )
{
	mTransform = *newTransform;
}

float RebelGeometryInstance::GetAngle()
{
	return mAngle;
}

void RebelGeometryInstance::SetAngle( float angle)
{
	mAngle = angle;
}

