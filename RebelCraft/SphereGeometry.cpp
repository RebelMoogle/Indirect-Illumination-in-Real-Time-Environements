#include "DXUT.h"
#include "SphereGeometry.h"
#include "ContentLoader.h"
#include "Util.h"

bool SphereGeometry::_Initialized = false;
optix::Program SphereGeometry::_ProgBoundingBox = NULL;
optix::Program SphereGeometry::_ProgIntersection = NULL;

SphereGeometry::SphereGeometry(const optix::float3& center, const float& radius):
RebelGeometry(true, true),
_Center(center),
_Radius(radius)
{

}

SphereGeometry::~SphereGeometry()
{
	D3DReleaseDevice();
	OptixReleaseDevice();
}

bool SphereGeometry::D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc)
{
	int Slices = 20;
	int Stacks = 15;
	if (Slices < 3)
        Slices = 3;
    if (Stacks < 2)
        Stacks = 2;            

    int numVertices = (Stacks + 1) * (Slices + 1);
    int numIndices = 6 * Stacks * (Slices + 1);

	bool useShort = numIndices < SHRT_MAX;

	int floatsPerVert = 3*2; // float3 * 2 (pos, normal)
	int sizeInBytes = numVertices * floatsPerVert * sizeof(float);
    float* verts = new float[numVertices * floatsPerVert]; 
    int* indices = NULL; 
    short* indices16 = NULL;
    if (useShort)
        indices16 = new short[numIndices];
    else
        indices = new int[numIndices];            

	float deltaRingAngle = (float)D3DX_PI / Stacks;
    float deltaSegmentAngle = 2.0f * (float)D3DX_PI / Slices;

    // Generate the group of rings for the sphere
    int vertexIndex = 0;
    int indexIndex = 0;
    for (int i = 0; i <= Stacks; i++)
    {
		float ring = sinf(i * deltaRingAngle);
        float y = cosf(i * deltaRingAngle);

        // Generate the group of segments for the current ring
        for (int j = 0; j <= Slices; j++)
        {
            float x = ring * sinf(j * deltaSegmentAngle);
            float z = ring * cosf(j * deltaSegmentAngle);

            // Add one vertex to the strip which makes up the sphere
            optix::float3 position = optix::make_float3(x, y, z);
			optix::float3 normal = optix::normalize(position);
    //        float2 textureCoordinate = make_float2((float)j / Slices, (float)i / Stacks);

			verts[vertexIndex * floatsPerVert + 0] = position.x * _Radius + _Center.x;
			verts[vertexIndex * floatsPerVert + 1] = position.y * _Radius + _Center.y;
			verts[vertexIndex * floatsPerVert + 2] = position.z * _Radius + _Center.z;
			verts[vertexIndex * floatsPerVert + 3] = normal.x;
			verts[vertexIndex * floatsPerVert + 4] = normal.y;
			verts[vertexIndex * floatsPerVert + 5] = normal.z;

            if (i != Stacks)
            {
                if (useShort)
                {
                    // each vertex (except the last) has six indices pointing to it
                    indices16[indexIndex++] = (short)(vertexIndex + Slices);
					indices16[indexIndex++] = (short)(vertexIndex + Slices + 1);
                    indices16[indexIndex++] = (short)(vertexIndex);
                    
                    indices16[indexIndex++] = (short)(vertexIndex);
					indices16[indexIndex++] = (short)(vertexIndex + Slices + 1);
                    indices16[indexIndex++] = (short)(vertexIndex + 1);
                    
                }
                else
                {
                    // each vertex (except the last) has six indices pointing to it
                    indices[indexIndex++] = vertexIndex + Slices;
					indices[indexIndex++] = vertexIndex + Slices + 1;
                    indices[indexIndex++] = vertexIndex;

                    indices[indexIndex++] = vertexIndex;
					indices[indexIndex++] = vertexIndex + Slices + 1;
                    indices[indexIndex++] = vertexIndex + 1;
                    
                }
            }

            vertexIndex++;
        }
    }

	_Stride = 24;

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
	{
		SAFE_DELETE_ARRAY(verts);
		SAFE_DELETE_ARRAY(indices);
		SAFE_DELETE_ARRAY(indices16);
		return false;
	}

	DXUT_SetDebugName(_VertexBuffer, "Sphere_VB");

	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = numIndices * ( useShort ? sizeof(short) : sizeof(int) );
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;		
	
	ZeroMemory(&init, sizeof(D3D11_SUBRESOURCE_DATA));
	init.pSysMem = useShort ? (void*)indices16 : (void*)indices;
	
	if (S_OK != Device->CreateBuffer(&desc, &init, &_IndexBuffer))
	{
		SAFE_DELETE_ARRAY(verts);
		SAFE_DELETE_ARRAY(indices);
		SAFE_DELETE_ARRAY(indices16);
		return false;
	}
	DXUT_SetDebugName(_IndexBuffer, "Sphere_IB");

	SAFE_DELETE_ARRAY(verts);
	SAFE_DELETE_ARRAY(indices);
	SAFE_DELETE_ARRAY(indices16);

	_IndexFormat = useShort ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
	_NumElements = numIndices;
	_Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	
	return true; 
}

void SphereGeometry::D3DReleaseDevice()
{
	SAFE_RELEASE(_VertexBuffer);
	SAFE_RELEASE(_IndexBuffer);
}

bool SphereGeometry::OptixCreateDevice(optix::Context& Context)
{
	if (!_Initialized)
	{
		if (!ContentLoader::LoadOptixProgramFromFile("CUDA\\SphereGeometry.cu.ptx", "bounds", Context, &_ProgBoundingBox)) return false;
		if (!ContentLoader::LoadOptixProgramFromFile("CUDA\\SphereGeometry.cu.ptx", "intersect", Context, &_ProgIntersection)) return false;		
		_Initialized = true;
	} // TODO: else increment ref counter of the programs!
	_Geometry = Context->createGeometry();
	_Geometry->setPrimitiveCount( 1u );
	_Geometry->setIntersectionProgram( _ProgIntersection );
	_Geometry->setBoundingBoxProgram( _ProgBoundingBox );	
	
	optix::float4 sphere = optix::make_float4( _Center, _Radius);
	_Geometry["sphere"]->setFloat( sphere );		
	return true;
}

void SphereGeometry::OptixReleaseDevice()
{
	_Geometry->destroy();
	// TODO: Release a ref of the intersection and bounding box program.
}
