#pragma once
#include <d3d9.h>
#include <d3dx9.h>

auto vertexDeleter = [](IDirect3DVertexBuffer9* pVertexBuffer)
{
	pVertexBuffer->Release();
};

auto textureDeleter = [](IDirect3DTexture9* pTexture)
{
	pTexture->Release();
};

IDirect3DIndexBuffer9* CreateIndexBuffer(IDirect3DDevice9* pDevice, const short* indices, const unsigned int count);

ID3DXEffect* CreateEffect(IDirect3DDevice9* pDevice, const wchar_t* const filename);

IDirect3DTexture9* CreateTexture(IDirect3DDevice9* pDevice, const wchar_t* const filename);

template<typename VertexType>
IDirect3DVertexBuffer9* CreateVertexBuffer(IDirect3DDevice9* pDevice, const VertexType* vertices, const unsigned int count, const unsigned long vertexFVF)
{
	const unsigned int bufferSize = count * sizeof(VertexType);
	IDirect3DVertexBuffer9* pVertexBuffer;
	if (FAILED(pDevice->CreateVertexBuffer
	(
		bufferSize,
		D3DUSAGE_WRITEONLY,
		vertexFVF,
		D3DPOOL_MANAGED,
		&pVertexBuffer,
		nullptr
	)))
		return nullptr;

	void* pData;
	if (FAILED(pVertexBuffer->Lock(0, 0, &pData, 0)))
	{
		pVertexBuffer->Release();
		return nullptr;
	}

	memcpy(pData, vertices, bufferSize);
	pVertexBuffer->Unlock();

	return pVertexBuffer;
}
