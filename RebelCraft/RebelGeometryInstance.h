#pragma once

#include "ID3DResource.h"
#include "IOptixResource.h"

class RebelGeometry;
class RebelMaterial;
class Scene;

class RebelGeometryInstance : public IOptixDeviceResource
{
	public:
		// Constructor. Creates a geometry instance.
		RebelGeometryInstance(RebelGeometry* Geometry, RebelMaterial* Material, Scene* Scene);
		// Destructor. Deletes the geometry instance.
		virtual ~RebelGeometryInstance();

		void Render(ID3D11DeviceContext* ImmediateContext);
		
		bool OptixCreateDevice(optix::Context& Context);
		void OptixReleaseDevice();
		void RenderToGBuffer( ID3D11DeviceContext* d3dDeviceContext );
		const RebelGeometry* GetGeometry() const;
		const D3DXMATRIX& GetTransform() const;
		float GetAngle();

		void SetTransform( D3DXMATRIX* newTransform );
		void SetAngle(float);

private:
		
		RebelGeometry* _Geometry;
		RebelMaterial* _Material;
		Scene* _Scene;

		optix::GeometryInstance _GeometryInstance;
		optix::GeometryGroup _GeometryGroup;
		optix::Acceleration _Acceleration;
		D3DXMATRIX mTransform;
		float mAngle;
		
};
