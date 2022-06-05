#pragma once
#include <d3dx9.h>
#include <functional>
#include <memory>
#include <string>

typedef std::unique_ptr<IDirect3DVertexBuffer9, std::function<void(IDirect3DVertexBuffer9*)>> VertexBuffer;
[[nodiscard]] VertexBuffer makeVertexBuffer() noexcept;
[[nodiscard]] IDirect3DVertexBuffer9* loadVertexBuffer(IDirect3DDevice9* pDevice, const void* vertices, unsigned int vertexSize, unsigned int count, unsigned long vertexFVF) noexcept;

typedef std::unique_ptr<IDirect3DIndexBuffer9, std::function<void(IDirect3DIndexBuffer9*)>> IndexBuffer;
[[nodiscard]] IndexBuffer makeIndexBuffer() noexcept;
[[nodiscard]] IDirect3DIndexBuffer9* loadIndexBuffer(IDirect3DDevice9* pDevice, const short* indices, unsigned int count) noexcept;

typedef std::unique_ptr<IDirect3DTexture9, std::function<void(IDirect3DTexture9*)>> Texture;
[[nodiscard]] Texture makeTexture() noexcept;
[[nodiscard]] IDirect3DTexture9* loadTexture(IDirect3DDevice9* pDevice, const wchar_t* filename) noexcept;

typedef std::unique_ptr<ID3DXEffect, std::function<void(ID3DXEffect*)>> Effect;
[[nodiscard]] Effect makeEffect() noexcept;
[[nodiscard]] ID3DXEffect* loadEffect(IDirect3DDevice9* pDevice, const wchar_t* filename);

typedef std::unique_ptr<IDirect3DSurface9, std::function<void(IDirect3DSurface9*)>> Surface;
[[nodiscard]] Surface makeSurface() noexcept;

typedef std::unique_ptr<IDirect3DVertexDeclaration9, std::function<void(IDirect3DVertexDeclaration9*)>> VertexDeclaration;
[[nodiscard]] VertexDeclaration makeVertexDeclaration() noexcept;
[[nodiscard]] IDirect3DVertexDeclaration9* loadVertexDeclaration(IDirect3DDevice9* pDevice, const D3DVERTEXELEMENT9* element) noexcept;

void renderEffect(ID3DXEffect* pEffect, const std::function<void()>& renderFunction);

struct TbnVertex
{
	[[maybe_unused]] D3DXVECTOR3 position;
	[[maybe_unused]] D3DXVECTOR3 normal;
	[[maybe_unused]] D3DXVECTOR3 tangent;
	[[maybe_unused]] D3DXVECTOR3 bitangent;
	[[maybe_unused]] D3DXVECTOR2 texcoord;
};

bool loadTbnObject(IDirect3DDevice9* pDevice, const std::string& filename, VertexBuffer& vertexbuffer, IndexBuffer& indexbuffer, int& indexCount, D3DXVECTOR4& sphere);
