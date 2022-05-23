#include "d3dwrap.h"
#include "wavefront.h"
#include <fstream>

std::function fVertexDeleter = [](IDirect3DVertexBuffer9* pVertexBuffer) -> void { pVertexBuffer->Release(); };
VertexBuffer makeVertexBuffer() { return VertexBuffer{ nullptr, fVertexDeleter }; }

std::function fIndexDeleter = [](IDirect3DIndexBuffer9* pIndexBuffer) -> void { pIndexBuffer->Release(); };
IndexBuffer makeIndexBuffer() { return IndexBuffer{ nullptr, fIndexDeleter }; }

std::function fTextureDeleter = [](IDirect3DTexture9* pTexture) -> void { pTexture->Release(); };
Texture makeTexture() { return Texture{ nullptr, fTextureDeleter }; }

std::function fEffectDeleter = [](ID3DXEffect* pEffect) -> void { pEffect->Release(); };
Effect makeEffect() { return Effect{ nullptr, fEffectDeleter }; }

std::function fSurfaceDeleter = [](IDirect3DSurface9* pSurface) -> void { pSurface->Release(); };
Surface makeSurface() { return Surface{ nullptr, fSurfaceDeleter }; }

std::function fVertexDeclarationDeleter = [](IDirect3DVertexDeclaration9* pDeclaration) -> void { pDeclaration->Release(); };
VertexDeclaration makeVertexDeclaration() { return VertexDeclaration{ nullptr, fVertexDeclarationDeleter }; }

IDirect3DIndexBuffer9* loadIndexBuffer(IDirect3DDevice9* pDevice, const short* indices, const unsigned int count)
{
	const unsigned int bufferSize = count * sizeof(short);
	IDirect3DIndexBuffer9* pIndexBuffer{};
	if (FAILED(pDevice->CreateIndexBuffer
	(
		bufferSize,
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&pIndexBuffer,
		nullptr
	)))
		return nullptr;

	short* pData{};
	if (FAILED(pIndexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pData), 0)))
	{
		pIndexBuffer->Release();
		return nullptr;
	}

	memcpy(pData, indices, bufferSize);
	pIndexBuffer->Unlock();

	return pIndexBuffer;
}

ID3DXEffect* loadEffect(IDirect3DDevice9* pDevice, const wchar_t* const filename)
{
	ID3DXEffect* pEffect{};
	ID3DXBuffer* pBufferErrors{};
	if (FAILED(D3DXCreateEffectFromFile
	(
		pDevice,
		filename,
		nullptr,
		nullptr,
		0,
		nullptr,
		&pEffect,
		&pBufferErrors
	)))
	{
		if (pBufferErrors != nullptr)
		{
			void* pErrors = pBufferErrors->GetBufferPointer();
			std::ofstream fout(L"fxlog.txt", std::ios_base::app);
			fout << static_cast<char*>(pErrors) << std::endl;
			fout.close();
		}
		return nullptr;
	}

	return pEffect;
}

IDirect3DTexture9* loadTexture(IDirect3DDevice9* pDevice, const wchar_t* const filename)
{
	IDirect3DTexture9* pTexture{};
	if (FAILED(D3DXCreateTextureFromFile(pDevice, filename, &pTexture)))
		return nullptr;
	return pTexture;
}

IDirect3DVertexBuffer9* loadVertexBuffer(IDirect3DDevice9* pDevice, const void* vertices, const unsigned int vertexSize, const unsigned int count, const unsigned long vertexFVF)
{
	const unsigned int bufferSize = count * vertexSize;
	IDirect3DVertexBuffer9* pVertexBuffer{};
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

	void* pData{};
	if (FAILED(pVertexBuffer->Lock(0, 0, &pData, 0)))
	{
		pVertexBuffer->Release();
		return nullptr;
	}

	memcpy(pData, vertices, bufferSize);
	pVertexBuffer->Unlock();

	return pVertexBuffer;
}

void renderEffect(ID3DXEffect* pEffect, const std::function<void()>& renderFunction)
{
	unsigned int uPasses;
	if (SUCCEEDED(pEffect->Begin(&uPasses, 0)))
	{
		for (unsigned int uPass = 0; uPass < uPasses; uPass++)
		{
			pEffect->BeginPass(uPass);
			renderFunction();
			pEffect->EndPass();
		}

		pEffect->End();
	}
}

IDirect3DVertexDeclaration9* loadVertexDeclaration(IDirect3DDevice9* pDevice, const D3DVERTEXELEMENT9* element)
{
	IDirect3DVertexDeclaration9* pDeclaration{};
	if (FAILED(pDevice->CreateVertexDeclaration(element, &pDeclaration)))
		return nullptr;

	return pDeclaration;
}

void calculateTangents(TbnVertex& a, TbnVertex& b, TbnVertex& c)
{
	D3DXVECTOR3 v = b.position - a.position, w = c.position - a.position;
	float sx = b.texcoord.x - a.texcoord.x, sy = b.texcoord.y - a.texcoord.y;
	float tx = c.texcoord.x - a.texcoord.x, ty = c.texcoord.y - a.texcoord.y;

	const float dirCorrection = (tx * sy - ty * sx) < 0.0f ? -1.0f : 1.0f;

	if (sx * ty == sy * tx) // when does this happen?
	{
		sx = 0.0;
		sy = 1.0;
		tx = 1.0;
		ty = 0.0;
	}

	D3DXVECTOR3 tangent, bitangent;
	tangent.x = (w.x * sy - v.x * ty) * dirCorrection;
	tangent.y = (w.y * sy - v.y * ty) * dirCorrection;
	tangent.z = (w.z * sy - v.z * ty) * dirCorrection;
	bitangent.x = (w.x * sx - v.x * tx) * dirCorrection;
	bitangent.y = (w.y * sx - v.y * tx) * dirCorrection;
	bitangent.z = (w.z * sx - v.z * tx) * dirCorrection;

	D3DXVECTOR3 localTangent = tangent - a.normal * D3DXVec3Dot(&tangent, &a.normal);
	D3DXVECTOR3 localBitangent = bitangent - a.normal * D3DXVec3Dot(&bitangent, &a.normal);

	D3DXVec3Normalize(&localTangent, &localTangent);
	D3DXVec3Normalize(&localBitangent, &localBitangent);

	a.tangent = localTangent;
	a.bitangent = localBitangent;

	localTangent = tangent - b.normal * D3DXVec3Dot(&tangent, &b.normal);
	localBitangent = bitangent - b.normal * D3DXVec3Dot(&bitangent, &b.normal);

	D3DXVec3Normalize(&localTangent, &localTangent);
	D3DXVec3Normalize(&localBitangent, &localBitangent);

	b.tangent = localTangent;
	b.bitangent = localBitangent;

	localTangent = tangent - c.normal * D3DXVec3Dot(&tangent, &c.normal);
	localBitangent = bitangent - c.normal * D3DXVec3Dot(&bitangent, &c.normal);

	D3DXVec3Normalize(&localTangent, &localTangent);
	D3DXVec3Normalize(&localBitangent, &localBitangent);

	c.tangent = localTangent;
	c.bitangent = localBitangent;
}

bool loadTbnObject(IDirect3DDevice9* pDevice, const std::string& filename, VertexBuffer& vertexbuffer, IndexBuffer& indexbuffer, int& indexCount, D3DXVECTOR4& sphere)
{
	std::vector<WfoVertex> vertex;
	std::vector<short> index;

	if (!loadWfObject(filename, vertex, index, sphere))
		return false;

	const int vertexCount = static_cast<int>(vertex.size());
	auto vertexBuffer = new TbnVertex[vertexCount];
	for (int i = 0; i < vertexCount; i++)
		vertexBuffer[i] =
		{
			.position = vertex[i].p,
			.normal = vertex[i].n,
			.tangent = { 0, 0, 0 },
			.bitangent = { 0, 0, 0 },
			.texcoord = vertex[i].t,
		};

	indexCount = static_cast<int>(index.size());
	auto indexBuffer = new short[indexCount];
	for (int i = 0; i < indexCount; i++)
		indexBuffer[i] = index[i];

	for (int i = 0; i < indexCount - 2; i += 3)
	{
		TbnVertex& a = vertexBuffer[indexBuffer[i]];
		TbnVertex& b = vertexBuffer[indexBuffer[i + 1]];
		TbnVertex& c = vertexBuffer[indexBuffer[i + 2]];

		calculateTangents(a, b, c);
	}

	vertexbuffer.reset(loadVertexBuffer(pDevice, vertexBuffer, sizeof(TbnVertex), vertexCount, 0));
	delete[] vertexBuffer;

	indexbuffer.reset(loadIndexBuffer(pDevice, indexBuffer, indexCount));
	delete[] indexBuffer;

	if (!vertexbuffer || !indexbuffer)
		return false;

	return true;
}
