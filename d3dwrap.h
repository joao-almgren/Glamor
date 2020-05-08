#pragma once
#include <d3dx9.h>
#include <functional>

//*********************************************************************************************************************

auto vertexDeleter = [](IDirect3DVertexBuffer9* pVertexBuffer)
{
	pVertexBuffer->Release();
};

typedef std::unique_ptr<IDirect3DVertexBuffer9, decltype(vertexDeleter)> VertexBuffer;

auto indexDeleter = [](IDirect3DIndexBuffer9* pIndexBuffer)
{
	pIndexBuffer->Release();
};

typedef std::unique_ptr<IDirect3DIndexBuffer9, decltype(indexDeleter)> IndexBuffer;

auto textureDeleter = [](IDirect3DTexture9* pTexture)
{
	pTexture->Release();
};

typedef std::unique_ptr<IDirect3DTexture9, decltype(textureDeleter)> Texture;

auto effectDeleter = [](ID3DXEffect* pEffect)
{
	pEffect->Release();
};

typedef std::unique_ptr<ID3DXEffect, decltype(effectDeleter)> Effect;

//*********************************************************************************************************************

IDirect3DVertexBuffer9* CreateVertexBuffer(IDirect3DDevice9* pDevice, const void* vertices, const unsigned int vertexSize, const unsigned int count, const unsigned long vertexFVF);
IDirect3DIndexBuffer9* CreateIndexBuffer(IDirect3DDevice9* pDevice, const short* indices, const unsigned int count);
IDirect3DTexture9* CreateTexture(IDirect3DDevice9* pDevice, const wchar_t* const filename);
ID3DXEffect* CreateEffect(IDirect3DDevice9* pDevice, const wchar_t* const filename);

//*********************************************************************************************************************

void RenderEffect(ID3DXEffect* pEffect, std::function<void(void)> renderFunction);

//*********************************************************************************************************************
