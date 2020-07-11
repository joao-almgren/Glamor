#pragma once
#include "wavefront.h"
#include <d3dx9.h>
#include <functional>
#include <memory>
#include <string>

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

struct TbnVertex
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	D3DXVECTOR3 tangent;
	D3DXVECTOR3 bitangent;
	D3DXVECTOR2 texcoord;
};

void CalculateTangents(TbnVertex& a, TbnVertex& b, TbnVertex& c);

bool LoadTbnObject(IDirect3DDevice9* pDevice, std::string filename, VertexBuffer& vertexbuffer, IndexBuffer& indexbuffer, int& indexCount)
{
	std::vector<WFOVertex> vertex;
	std::vector<short> index;

	if (!LoadWFObject(filename, vertex, index))
		return false;

	int vertexCount = static_cast<int>(vertex.size());
	TbnVertex* vertex_buffer = new TbnVertex[vertexCount];
	for (int i = 0; i < vertexCount; i++)
		vertex_buffer[i] =
	{
		.position = vertex[i].p,
		.normal = vertex[i].n,
		.tangent = { 0, 0, 0 },
		.bitangent = { 0, 0, 0 },
		.texcoord = vertex[i].t,
	};

	indexCount = static_cast<int>(index.size());
	short* index_buffer = new short[indexCount];
	for (int i = 0; i < indexCount; i++)
		index_buffer[i] = index[i];

	for (int i = 0; i < indexCount; i += 3)
	{
		TbnVertex& a = vertex_buffer[index_buffer[i]];
		TbnVertex& b = vertex_buffer[index_buffer[i + 1]];
		TbnVertex& c = vertex_buffer[index_buffer[i + 2]];

		CalculateTangents(a, b, c);
	}

	vertexbuffer.reset(LoadVertexBuffer(pDevice, vertex_buffer, sizeof(TbnVertex), vertexCount, 0));
	delete[] vertex_buffer;

	indexbuffer.reset(LoadIndexBuffer(pDevice, index_buffer, indexCount));
	delete[] index_buffer;

	if (!vertexbuffer || !indexbuffer)
		return false;

	return true;
}

//*********************************************************************************************************************
