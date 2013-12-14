#include "DXUT.h"
#include "BoxGeometry.h"
#include "Constants.h"
#include "ContentLoader.h"
#include "Util.h"

bool BoxGeometry::mInitialized = false;
optix::Program BoxGeometry::mProgBoundingBox = NULL;
optix::Program BoxGeometry::mProgIntersection = NULL;

BoxGeometry::BoxGeometry( const D3DXVECTOR3& position, const D3DXVECTOR3& scale, bool isStatic )
	: RebelGeometry(isStatic, true),
	mPosition(position),
	mScale(scale)
{

}



BoxGeometry::~BoxGeometry(void)
{
	D3DReleaseDevice();
	OptixReleaseDevice();
}

bool BoxGeometry::D3DCreateDevice( ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc )
{

	// ### Box vertices and indices
	PositionNormalTexCoordVertex vertices[] =
	{
		//FRONT
		{ mPosition + D3DXVECTOR3( 0.0f, 0.0f, 0.0f),			D3DXVECTOR3( 0.0f, 0.0f, -1.0f), D3DXVECTOR2(0.0f, 0.0f) }, // Bottom Left,	0
		{ mPosition + D3DXVECTOR3( 0.0f, mScale.y, 0.0f),		D3DXVECTOR3( 0.0f, 0.0f, -1.0f), D3DXVECTOR2(1.0f, 0.0f) }, // Top Left,	1 
		{ mPosition + D3DXVECTOR3( mScale.x, 0.0f, 0.0f),		D3DXVECTOR3( 0.0f, 0.0f, -1.0f), D3DXVECTOR2(0.0f, 1.0f) }, // Bottom Right,2
		{ mPosition + D3DXVECTOR3( mScale.x, mScale.y, 0.0f),	D3DXVECTOR3( 0.0f, 0.0f, -1.0f), D3DXVECTOR2(1.0f, 1.0f) }, // Top Right,	3

		//RIGHT
		{ mPosition + D3DXVECTOR3( mScale.x, 0.0f, 0.0f),		D3DXVECTOR3( 1.0f, 0.0f, 0.0f),	D3DXVECTOR2(0.0f, 0.0f) }, // Bottom Left,	4
		{ mPosition + D3DXVECTOR3( mScale.x, mScale.y, 0.0f),	D3DXVECTOR3( 1.0f, 0.0f, 0.0f),	D3DXVECTOR2(1.0f, 0.0f) }, // Top Left,		5 
		{ mPosition + D3DXVECTOR3( mScale.x, 0.0f, mScale.z),	D3DXVECTOR3( 1.0f, 0.0f, 0.0f),	D3DXVECTOR2(0.0f, 1.0f) }, // Bottom Right,	6
		{ mPosition + D3DXVECTOR3( mScale.x, mScale.y, mScale.z), D3DXVECTOR3( 1.0f, 0.0f, 0.0f),	D3DXVECTOR2(1.0f, 1.0f) }, // Top Right,	7

		//LEFT
		{ mPosition + D3DXVECTOR3( 0.0f, 0.0f, mScale.z),		D3DXVECTOR3( -1.0f, 0.0f, 0.0f), D3DXVECTOR2(0.0f, 0.0f) }, // Bottom Left,	8
		{ mPosition + D3DXVECTOR3( 0.0f, mScale.y, mScale.z),	D3DXVECTOR3( -1.0f, 0.0f, 0.0f), D3DXVECTOR2(1.0f, 0.0f) }, // Top Left,	9 
		{ mPosition + D3DXVECTOR3( 0.0f, 0.0f, 0.0f),			D3DXVECTOR3( -1.0f, 0.0f, 0.0f), D3DXVECTOR2(0.0f, 1.0f) }, // Bottom Right,10
		{ mPosition + D3DXVECTOR3( 0.0f, mScale.y, 0.0f),		D3DXVECTOR3( -1.0f, 0.0f, 0.0f), D3DXVECTOR2(1.0f, 1.0f) }, // Top Right,	11

		//TOP
		{ mPosition + D3DXVECTOR3( 0.0f, mScale.y, 0.0f),		D3DXVECTOR3( 0.0f, 1.0f, 0.0f),	D3DXVECTOR2(0.0f, 0.0f) }, // Bottom Left,	12
		{ mPosition + D3DXVECTOR3( 0.0f, mScale.y, mScale.z),	D3DXVECTOR3( 0.0f, 1.0f, 0.0f),	D3DXVECTOR2(1.0f, 0.0f) }, // Top Left,		13 
		{ mPosition + D3DXVECTOR3( mScale.x, mScale.y, 0.0f),	D3DXVECTOR3( 0.0f, 1.0f, 0.0f),	D3DXVECTOR2(0.0f, 1.0f) }, // Bottom Right,	14
		{ mPosition + D3DXVECTOR3( mScale.x, mScale.y, mScale.z), D3DXVECTOR3( 0.0f, 1.0f, 0.0f),	D3DXVECTOR2(1.0f, 1.0f) }, // Top Right,	15

		//BACK
		{ mPosition + D3DXVECTOR3(mScale.x, 0.0f, mScale.z),	D3DXVECTOR3( 0.0f, 0.0f, 1.0f),	D3DXVECTOR2(0.0f, 0.0f) }, // Bottom Left,	16
		{ mPosition + D3DXVECTOR3(mScale.x, mScale.y, mScale.z),D3DXVECTOR3( 0.0f, 0.0f, 1.0f),	D3DXVECTOR2(1.0f, 0.0f) }, // Top Left,		17
		{ mPosition + D3DXVECTOR3( 0.0f, 0.0f, mScale.z),		D3DXVECTOR3( 0.0f, 0.0f, 1.0f),	D3DXVECTOR2(0.0f, 1.0f) }, // Bottom Right,	18
		{ mPosition + D3DXVECTOR3( 0.0f, mScale.y, mScale.z),	D3DXVECTOR3( 0.0f, 0.0f, 1.0f),	D3DXVECTOR2(1.0f, 1.0f) }, // Top Right,	19

		//GROUND / BOTTOM
		{ mPosition + D3DXVECTOR3( 0.0f, 0.0f, 0.0f),			D3DXVECTOR3( 0.0f, -1.0f, 0.0f), D3DXVECTOR2(1.0f, 0.0f) },	// Top Left,	20
		{ mPosition + D3DXVECTOR3( mScale.x, 0.0f, 0.0f),		D3DXVECTOR3( 0.0f, -1.0f, 0.0f), D3DXVECTOR2(1.0f, 1.0f) }, // Top Right,	21
		{ mPosition + D3DXVECTOR3( 0.0f, 0.0f, mScale.z),		D3DXVECTOR3( 0.0f, -1.0f, 0.0f), D3DXVECTOR2(0.0f, 0.0f) }, // Bottom Left,	22
		{ mPosition + D3DXVECTOR3( mScale.x, 0.0f, mScale.z),	D3DXVECTOR3( 0.0f, -1.0f, 0.0f), D3DXVECTOR2(0.0f, 1.0f) } // Bottom Right,	23
	};


	short indices16[] = 
	{
		// FRONT
		0, 1, 2,		2, 1, 3,
		
		// RIGHT
		4, 5, 6,		6, 5, 7,
		
		//LEFT
		8, 9, 10,		10, 9, 11,

		//TOP
		12, 13, 14,		14, 13, 15,

		//BACK
		16, 17, 18,		18, 17, 19,

		//GROUND / BOTTOM
		20, 21, 22,		22, 21, 23
	};

	D3D11_BUFFER_DESC boxDesc;
	ZeroMemory( &boxDesc, sizeof(D3D11_BUFFER_DESC) );
	boxDesc.Usage = D3D11_USAGE_DEFAULT;
	boxDesc.ByteWidth = sizeof( PositionNormalTexCoordVertex ) * 24;
	boxDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	boxDesc.CPUAccessFlags = 0;
	boxDesc.MiscFlags = 0;
	boxDesc.StructureByteStride = sizeof( PositionNormalTexCoordVertex );
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory( &InitData, sizeof(InitData) );
	InitData.pSysMem = vertices;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;
	if (S_OK != Device->CreateBuffer(&boxDesc, &InitData, &_VertexBuffer))	
	{
		//delete[] vertices;
		//delete[] indices16;
		return false;
	}

	DXUT_SetDebugName(_VertexBuffer, "Box_VB");

	UINT numIndices = 36; //ARRAYSIZE(indices16);

	//create index Buffer
	ZeroMemory(&boxDesc, sizeof(D3D11_BUFFER_DESC));
	boxDesc.Usage = D3D11_USAGE_DEFAULT;
	boxDesc.ByteWidth = numIndices * ( sizeof(short));
	boxDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	boxDesc.CPUAccessFlags = 0;
	boxDesc.MiscFlags = 0;		

	ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
	InitData.pSysMem = indices16;

	if (S_OK != Device->CreateBuffer(&boxDesc, &InitData, &_IndexBuffer))
	{
		//delete[] vertices;
		//delete[] indices16;
		return false;
	}
	DXUT_SetDebugName(_IndexBuffer, "Sphere_IB");

	//delete[] vertices;
	//delete[] indices16;

	_Stride = sizeof(PositionNormalTexCoordVertex);
	_IndexFormat = DXGI_FORMAT_R16_UINT;
	_NumElements = numIndices;
	_Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	return true; 
}

void BoxGeometry::D3DReleaseDevice()
{
	SAFE_RELEASE(_VertexBuffer);
	SAFE_RELEASE(_IndexBuffer);
}

bool BoxGeometry::OptixCreateDevice( optix::Context& Context )
{
	if (!mInitialized)
	{
		if (!ContentLoader::LoadOptixProgramFromFile("CUDA\\BoxGeometry.cu.ptx", "bounds", Context, &mProgBoundingBox)) return false;
		if (!ContentLoader::LoadOptixProgramFromFile("CUDA\\BoxGeometry.cu.ptx", "intersect", Context, &mProgIntersection)) return false;		
		mInitialized = true;
	} // TODO: else increment ref counter of the programs!
	_Geometry = Context->createGeometry();
	_Geometry->setPrimitiveCount( 1u );
	_Geometry->setIntersectionProgram( mProgIntersection );
	_Geometry->setBoundingBoxProgram( mProgBoundingBox );	

	optix::float3 position = Util::Float3FromD3DXVECTOR3(&mPosition);
	_Geometry["position"]->setFloat( position );
	optix::float3 scale = Util::Float3FromD3DXVECTOR3(&mScale);
	_Geometry["scale"]->setFloat(scale);
	return true;
}

void BoxGeometry::OptixReleaseDevice()
{
	if (_Geometry)
		_Geometry->destroy();
}

void BoxGeometry::SetPosition( D3DXVECTOR3 position )
{
	mPosition = position;
}

void BoxGeometry::SetScale( D3DXVECTOR3 scale)
{
	mScale = scale;
}

