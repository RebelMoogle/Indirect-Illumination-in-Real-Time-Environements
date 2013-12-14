#pragma once

#include <D3D11.h>
#include "ID3DResource.h"

template <typename T>
class ConstantBuffer : public ID3DDeviceResource
{
	public:

		ConstantBuffer() : Data(), _Buffer(NULL) {}
		ConstantBuffer(const T& Initial) : Data(Initial), _Buffer(NULL) {}

		~ConstantBuffer() { D3DReleaseDevice(); }

		T Data;

		ID3D11Buffer* GetBuffer() const { return _Buffer; }

		bool D3DCreateDevice(ID3D11Device* Device, const DXGI_SURFACE_DESC* BackBufferSurfaceDesc)
		{
			int size = sizeof(T);
			if ((size & 15) != 0)
			{
				size >>= 4;
				size++;
				size <<= 4;
			}

			D3D11_BUFFER_DESC desc;	
			ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.ByteWidth = size;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA init;
			ZeroMemory(&init, sizeof(D3D11_SUBRESOURCE_DATA));
			init.pSysMem = &Data;
			if (S_OK != Device->CreateBuffer(&desc, &init, &_Buffer))
				return false;

			return true;
		}

		void D3DReleaseDevice()
		{
			if (_Buffer) { _Buffer->Release(); _Buffer = NULL; }
		}

		void UpdateBuffer(ID3D11DeviceContext* ImmediateContext)
		{
			if (!_Buffer) return;
			D3D11_MAPPED_SUBRESOURCE MappedSubResource;		
			ImmediateContext->Map(_Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubResource );			
			*(T*)MappedSubResource.pData = Data;
			ImmediateContext->Unmap(_Buffer, 0 );		
		}

		bool IsLoaded() { return _Buffer != NULL; }

	private:

		ID3D11Buffer* _Buffer;
		
};