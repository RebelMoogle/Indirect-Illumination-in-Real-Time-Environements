#pragma  once

#include "ID3DResource.h"
#include "IOptixResource.h"

class RebelGeometry : public ID3DDeviceResource, public IOptixDeviceResource
{
	public:
		// Constructor. Creates an empty geometry.	
		explicit RebelGeometry(bool isStatic = true, bool hasOptixResource = true);
		// Destructor. Deletes entity.
		virtual ~RebelGeometry();

		void Bind(ID3D11DeviceContext* ImmediateContext);
		void Render(ID3D11DeviceContext* ImmediateContext);

		//Gets whether this geometry is static.
		bool IsStatic() const;

		// Optix Geometry
		optix::Geometry GetGeometry();

		// Checks whether this geometry has an optix representation.
		bool HasOptixResource() const;

	protected:
		
		// Optix Geometry
		optix::Geometry _Geometry;
		// Stores optix primitives.
		optix::Buffer _Buffer;

		// Vertex buffer used for rendering with D3D.
		ID3D11Buffer* _VertexBuffer;
		// Index buffer used for rendering with D3D.
		ID3D11Buffer* _IndexBuffer;
		// Number of elements in the D3D buffers. (if IndexBuffer=null then NumVertices else NumIndices)
		UINT _NumElements;
		// Stride in the vertex buffer.
		UINT _Stride;
		// Topology of the primitive.
		D3D11_PRIMITIVE_TOPOLOGY _Topology;
		// Format of the elements in the index buffer.
		DXGI_FORMAT _IndexFormat;

		// Flag that determines whether this geometry is static.
		bool _IsStatic;

		// Flag that determines whether this geometry has an optix representation.
		bool _HasOptixResource;
		
};
