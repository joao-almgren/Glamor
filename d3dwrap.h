#pragma once
#include <d3dx9.h>
#include <functional>

//*********************************************************************************************************************

auto gVertexDeleter = [](IDirect3DVertexBuffer9* pVertexBuffer) { pVertexBuffer->Release(); };
typedef std::unique_ptr<IDirect3DVertexBuffer9, decltype(gVertexDeleter)> VertexBuffer;
auto MakeVertexBuffer() { return VertexBuffer{ nullptr, gVertexDeleter }; }

auto gIndexDeleter = [](IDirect3DIndexBuffer9* pIndexBuffer) { pIndexBuffer->Release(); };
typedef std::unique_ptr<IDirect3DIndexBuffer9, decltype(gIndexDeleter)> IndexBuffer;
auto MakeIndexBuffer() { return IndexBuffer{ nullptr, gIndexDeleter }; }

auto gTextureDeleter = [](IDirect3DTexture9* pTexture) { pTexture->Release(); };
typedef std::unique_ptr<IDirect3DTexture9, decltype(gTextureDeleter)> Texture;
auto MakeTexture() { return Texture{ nullptr, gTextureDeleter }; }

auto gEffectDeleter = [](ID3DXEffect* pEffect) { pEffect->Release(); };
typedef std::unique_ptr<ID3DXEffect, decltype(gEffectDeleter)> Effect;
auto MakeEffect() { return Effect{ nullptr, gEffectDeleter }; }

auto gSurfaceDeleter = [](IDirect3DSurface9* pSurface) { pSurface->Release(); };
typedef std::unique_ptr<IDirect3DSurface9, decltype(gSurfaceDeleter)> Surface;
auto MakeSurface() { return Surface{ nullptr, gSurfaceDeleter }; }

auto gVertexDeclarationDeleter = [](IDirect3DVertexDeclaration9* pDeclaration) { pDeclaration->Release(); };
typedef std::unique_ptr<IDirect3DVertexDeclaration9, decltype(gVertexDeclarationDeleter)> VertexDeclaration;
auto MakeVertexDeclaration() { return VertexDeclaration{ nullptr, gVertexDeclarationDeleter }; }

//*********************************************************************************************************************

IDirect3DVertexBuffer9* LoadVertexBuffer(IDirect3DDevice9* pDevice, const void* vertices, const unsigned int vertexSize, const unsigned int count, const unsigned long vertexFVF);
IDirect3DIndexBuffer9* LoadIndexBuffer(IDirect3DDevice9* pDevice, const short* indices, const unsigned int count);
IDirect3DTexture9* LoadTexture(IDirect3DDevice9* pDevice, const wchar_t* const filename);
ID3DXEffect* LoadEffect(IDirect3DDevice9* pDevice, const wchar_t* const filename);
IDirect3DVertexDeclaration9* LoadVertexDeclaration(IDirect3DDevice9* pDevice, const D3DVERTEXELEMENT9* element);

//*********************************************************************************************************************

void RenderEffect(ID3DXEffect* pEffect, std::function<void(void)> renderFunction);

//*********************************************************************************************************************
