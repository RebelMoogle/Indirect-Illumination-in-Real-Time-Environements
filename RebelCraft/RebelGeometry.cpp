#include "DXUT.h"
#include "RebelGeometry.h"

// ================================================================================
RebelGeometry::RebelGeometry(bool isStatic, bool hasOptixResource) : 
_HasOptixResource(hasOptixResource),
_Geometry(NULL),
_Buffer(NULL),
_VertexBuffer(NULL),
_IndexBuffer(NULL),
_NumElements(0),
_IsStatic(isStatic),
_Stride(0),
_IndexFormat(DXGI_FORMAT_R16_UINT),
_Topology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
{

}

// ================================================================================
RebelGeometry::~RebelGeometry()
{
	SAFE_RELEASE(_VertexBuffer);
	SAFE_RELEASE(_IndexBuffer);
}

// ================================================================================
bool RebelGeometry::IsStatic() const
{
	return _IsStatic;
}

// ================================================================================
void RebelGeometry::Bind(ID3D11DeviceContext* ImmediateContext)
{
	ImmediateContext->IASetIndexBuffer(_IndexBuffer, _IndexFormat, 0);
	UINT offset = 0;
	ImmediateContext->IASetVertexBuffers(0, 1, &_VertexBuffer, &_Stride, &offset);
	ImmediateContext->IASetPrimitiveTopology(_Topology);
}

// ================================================================================
void RebelGeometry::Render(ID3D11DeviceContext* ImmediateContext)
{
	if (_IndexBuffer)
		ImmediateContext->DrawIndexed(_NumElements, 0, 0);
	else ImmediateContext->Draw(_NumElements, 0);
}

// ================================================================================
optix::Geometry RebelGeometry::GetGeometry()
{
	return _Geometry;
}

// ================================================================================
bool RebelGeometry::HasOptixResource() const
{
	return _HasOptixResource;
}