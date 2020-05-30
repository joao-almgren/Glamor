#include "d3dwrap.h"
#include <fstream>

//*********************************************************************************************************************

IDirect3DIndexBuffer9* CreateIndexBuffer(IDirect3DDevice9* pDevice, const short* indices, const unsigned int count)
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
	if (FAILED(pIndexBuffer->Lock(0, 0, (void**)&pData, 0)))
	{
		pIndexBuffer->Release();
		return nullptr;
	}

	memcpy(pData, indices, bufferSize);
	pIndexBuffer->Unlock();

	return pIndexBuffer;
}

//*********************************************************************************************************************

ID3DXEffect* CreateEffect(IDirect3DDevice9* pDevice, const wchar_t* const filename)
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

//*********************************************************************************************************************

IDirect3DTexture9* LoadTexture(IDirect3DDevice9* pDevice, const wchar_t* const filename)
{
	IDirect3DTexture9* pTexture{};
	if (FAILED(D3DXCreateTextureFromFile(pDevice, filename, &pTexture)))
		return nullptr;
	return pTexture;
}

//*********************************************************************************************************************

IDirect3DVertexBuffer9* CreateVertexBuffer(IDirect3DDevice9* pDevice, const void* vertices, const unsigned int vertexSize, const unsigned int count, const unsigned long vertexFVF)
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

//*********************************************************************************************************************

void RenderEffect(ID3DXEffect* pEffect, std::function<void(void)> renderFunction)
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

//*********************************************************************************************************************
