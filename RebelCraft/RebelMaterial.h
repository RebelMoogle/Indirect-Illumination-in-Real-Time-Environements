#pragma once

#include "ID3DResource.h"
#include "IOptixResource.h"
#include "Shader.h"

class Scene;

class RebelMaterial : public ID3DDeviceResource, public IOptixDeviceResource
{
	public:
		// Constructor. Creates an empty material.	
		RebelMaterial();
		// Destructor. Deletes entity.
		virtual ~RebelMaterial();

		// Binds the shader.
		void Bind(ID3D11DeviceContext* ImmediateContext, Scene* Scene);
		
		// Binds effect parameters.
		virtual void BindParams(ID3D11DeviceContext* ImmediateContext) = 0;

		// Optix material
		optix::Material GetMaterial();

	protected:
		
		bool LoadEffectFromFile(const std::string& path, ID3D11Device* Device);

		// Optix material
		optix::Material _Material;

		// D3D input layout (assumes pos/normal layout...)
		ID3D11InputLayout* _InputLayout;

		// TODO: Shader	
		VertexShader* materialVS;
		PixelShader* materialPS;
};
