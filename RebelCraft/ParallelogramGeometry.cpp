#include "DXUT.h"
#include "ParallelogramGeometry.h"
#include "ContentLoader.h"

bool ParallelogramGeometry::_Initialized = false;
optix::Program ParallelogramGeometry::_ProgBoundingBox = NULL;
optix::Program ParallelogramGeometry::_ProgIntersection = NULL;

ParallelogramGeometry::ParallelogramGeometry(const optix::float3& anchor, const optix::float3& offset1, const optix::float3& offset2):
_Anchor(anchor),
_Offset1(offset1),
_Offset2(offset2)
{

}

ParallelogramGeometry::~ParallelogramGeometry()
{
	D3DReleaseDevice();
	OptixReleaseDevice();
}

static struct VertexPosNormalTexCoord
{
	optix::float3 Position;
	optix::float3 Normal;
	optix::float2 TexCoord;

	VertexPosNormalTexCoord(const optix::float3& pos, const optix::float3& normal, const optix::float2& tex) : Position(pos), Normal(normal), TexCoord(tex) {}
};

bool ParallelogramGeometry::D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc)
{
	_Stride = 32;
	unsigned int numVerts = 4;
	unsigned int sizeInBytes = _Stride * numVerts;

	optix::float3 normal = normalize(cross(_Offset1, _Offset2));
	VertexPosNormalTexCoord verts[] =
	{		
		VertexPosNormalTexCoord(_Anchor, normal, optix::make_float2(0,0)),
		VertexPosNormalTexCoord(_Anchor + _Offset1, normal, optix::make_float2(0,1)),
		VertexPosNormalTexCoord(_Anchor + _Offset2, normal, optix::make_float2(1,0)),
		VertexPosNormalTexCoord(_Anchor + _Offset1 + _Offset2, normal, optix::make_float2(1,1))
	};

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeInBytes;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;	

	D3D11_SUBRESOURCE_DATA init;
	ZeroMemory(&init, sizeof(D3D11_SUBRESOURCE_DATA));
	init.pSysMem = verts;
	
	if (S_OK != Device->CreateBuffer(&desc, &init, &_VertexBuffer))		
		return false;

	DXUT_SetDebugName(_VertexBuffer, "Parallelogram_VB");

	_NumElements = numVerts;
	_Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	
	return true;
}

void ParallelogramGeometry::D3DReleaseDevice()
{
	SAFE_RELEASE(_VertexBuffer);
}

bool ParallelogramGeometry::OptixCreateDevice(optix::Context& Context)
{
	if (!_Initialized)
	{
		if (!ContentLoader::LoadOptixProgramFromFile("CUDA\\ParallelogramGeometry.cu.ptx", "intersect", Context, &_ProgIntersection )) return false;
		if (!ContentLoader::LoadOptixProgramFromFile("CUDA\\ParallelogramGeometry.cu.ptx", "bounds", Context, &_ProgBoundingBox )) return false;		
		_Initialized = true;
	} // TODO: else increment ref counter of the programs!
	_Geometry = Context->createGeometry();
	_Geometry->setPrimitiveCount( 1u );
	_Geometry->setIntersectionProgram( _ProgIntersection );
	_Geometry->setBoundingBoxProgram( _ProgBoundingBox );

	optix::float3 normal = normalize( cross( _Offset1, _Offset2 ) );
	float d       = dot( normal, _Anchor );
	optix::float4 plane  = make_float4( normal, d );

	optix::float3 v1 = _Offset1 / dot( _Offset1, _Offset1 );
	optix::float3 v2 = _Offset2 / dot( _Offset2, _Offset2 );

	_Geometry["plane"]->setFloat( plane );
	_Geometry["anchor"]->setFloat( _Anchor );
	_Geometry["v1"]->setFloat( v1 );
	_Geometry["v2"]->setFloat( v2 );
	return true;
}

void ParallelogramGeometry::OptixReleaseDevice()
{
	if (_Geometry)
		_Geometry->destroy();
	// TODO: Release a ref of the intersection and bounding box program.
}
