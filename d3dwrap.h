#pragma once
#include <d3dx9.h>
#include <functional>
#include <memory>
#include <string>

typedef std::unique_ptr<IDirect3DVertexBuffer9, std::function<void(IDirect3DVertexBuffer9*)>> VertexBuffer;
[[nodiscard]] VertexBuffer MakeVertexBuffer();
[[nodiscard]] IDirect3DVertexBuffer9* LoadVertexBuffer(IDirect3DDevice9* pDevice, const void* vertices, const unsigned int vertexSize, const unsigned int count, const unsigned long vertexFVF);

typedef std::unique_ptr<IDirect3DIndexBuffer9, std::function<void(IDirect3DIndexBuffer9*)>> IndexBuffer;
[[nodiscard]] IndexBuffer MakeIndexBuffer();
[[nodiscard]] IDirect3DIndexBuffer9* LoadIndexBuffer(IDirect3DDevice9* pDevice, const short* indices, const unsigned int count);

typedef std::unique_ptr<IDirect3DTexture9, std::function<void(IDirect3DTexture9*)>> Texture;
[[nodiscard]] Texture MakeTexture();
[[nodiscard]] IDirect3DTexture9* LoadTexture(IDirect3DDevice9* pDevice, const wchar_t* const filename);

typedef std::unique_ptr<ID3DXEffect, std::function<void(ID3DXEffect*)>> Effect;
[[nodiscard]] Effect MakeEffect();
[[nodiscard]] ID3DXEffect* LoadEffect(IDirect3DDevice9* pDevice, const wchar_t* const filename);

typedef std::unique_ptr<IDirect3DSurface9, std::function<void(IDirect3DSurface9*)>> Surface;
[[nodiscard]] Surface MakeSurface();

typedef std::unique_ptr<IDirect3DVertexDeclaration9, std::function<void(IDirect3DVertexDeclaration9*)>> VertexDeclaration;
[[nodiscard]] VertexDeclaration MakeVertexDeclaration();
[[nodiscard]] IDirect3DVertexDeclaration9* LoadVertexDeclaration(IDirect3DDevice9* pDevice, const D3DVERTEXELEMENT9* element);

void RenderEffect(ID3DXEffect* pEffect, std::function<void(void)> renderFunction);

struct TbnVertex
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	D3DXVECTOR3 tangent;
	D3DXVECTOR3 bitangent;
	D3DXVECTOR2 texcoord;
};

bool LoadTbnObject(IDirect3DDevice9* pDevice, const std::string& filename, VertexBuffer& vertexbuffer, IndexBuffer& indexbuffer, int& indexCount, D3DXVECTOR4& sphere);
