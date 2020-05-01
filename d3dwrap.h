#pragma once
#include <d3d9.h>
#include <d3dx9.h>

//*********************************************************************************************************************

IDirect3DVertexBuffer9* CreateVertexBuffer(IDirect3DDevice9* pDevice, const void* vertices, const unsigned int vertexSize, const unsigned int count, const unsigned long vertexFVF);
IDirect3DIndexBuffer9* CreateIndexBuffer(IDirect3DDevice9* pDevice, const short* indices, const unsigned int count);
IDirect3DTexture9* CreateTexture(IDirect3DDevice9* pDevice, const wchar_t* const filename);
ID3DXEffect* CreateEffect(IDirect3DDevice9* pDevice, const wchar_t* const filename);

auto vertexDeleter = [](IDirect3DVertexBuffer9* pVertexBuffer)
{
	pVertexBuffer->Release();
};

auto indexDeleter = [](IDirect3DIndexBuffer9* pIndexBuffer)
{
	pIndexBuffer->Release();
};

auto textureDeleter = [](IDirect3DTexture9* pTexture)
{
	pTexture->Release();
};

auto effectDeleter = [](ID3DXEffect* pEffect)
{
	pEffect->Release();
};

//*********************************************************************************************************************
