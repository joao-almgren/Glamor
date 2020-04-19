#include "cube.h"

namespace
{
	const unsigned long cubeVertexFVF{ D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_TEXCOORDSIZE2(0) };
	struct CubeVertex
	{
		float x, y, z;
		float nx, ny, nz;
		unsigned long c;
		float u, v;
	};

	const CubeVertex cubeVertex[]
	{
		{ -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(0, 255, 0), 0, 0 },
		{ 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(0, 255, 0), 1, 0 },
		{ -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(0, 255, 0), 0, 1 },
		{ 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(0, 255, 0), 1, 1 },
		{ -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, D3DCOLOR_XRGB(255, 0, 0), 0, 0 },
		{ -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, D3DCOLOR_XRGB(255, 0, 0), 0, 1 },
		{ 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, D3DCOLOR_XRGB(255, 0, 0), 1, 0 },
		{ 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, D3DCOLOR_XRGB(255, 0, 0), 1, 1 },
		{ -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(0, 0, 255), 0, 0 },
		{ -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(0, 0, 255), 0, 1 },
		{ 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(0, 0, 255), 1, 0 },
		{ 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(0, 0, 255), 1, 1 },
		{ -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 0), 0, 0 },
		{ 1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 0), 1, 0 },
		{ -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 0), 0, 1 },
		{ 1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 0), 1, 1 },
		{ 1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 255), 0, 0 },
		{ 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 255), 1, 0 },
		{ 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 255), 0, 1 },
		{ 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 255), 1, 1 },
		{ -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 255), 0, 0 },
		{ -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 255), 0, 1 },
		{ -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 255), 1, 0 },
		{ -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 255), 1, 1 }
	};

	const short cubeIndex[]
	{
		0, 1, 2,
		2, 1, 3,
		4, 5, 6,
		6, 5, 7,
		8, 9, 10,
		10, 9, 11,
		12, 13, 14,
		14, 13, 15,
		16, 17, 18,
		18, 17, 19,
		20, 21, 22,
		22, 21, 23
	};
}

Cube::Cube()
	: pVertexBufferCube(nullptr, vertexDeleter)
	, pIndexBufferCube(nullptr, indexDeleter)
	, pEffect(nullptr, effectDeleter)
	, pTextureCube(nullptr, textureDeleter)
{
}

bool Cube::init(IDirect3DDevice9* pDevice)
{
	pVertexBufferCube.reset(CreateVertexBuffer(pDevice, cubeVertex, 24, cubeVertexFVF));
	if (!pVertexBufferCube)
		return false;

	pIndexBufferCube.reset(CreateIndexBuffer(pDevice, cubeIndex, 36));
	if (!pIndexBufferCube)
		return false;

	pEffect.reset(CreateEffect(pDevice, L"main.fx"));
	if (!pEffect)
		return false;

	pTextureCube.reset(CreateTexture(pDevice, L"smiley.bmp"));
	if (!pTextureCube)
		return false;

	return true;
}

void Cube::draw(IDirect3DDevice9* pDevice)
{
	static auto angle = 0.0f;
	angle += 0.015f;

	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	D3DXMATRIX matProjection;
	pDevice->GetTransform(D3DTS_PROJECTION, &matProjection);

	D3DXMATRIX matView;
	const D3DXVECTOR3 eye(5.0f, 0.0f, 0.0f);
	const D3DXVECTOR3 at(0.0f, 0.0f, 0.0f);
	const D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&matView, &eye, &at, &up);
	pDevice->SetTransform(D3DTS_VIEW, &matView);

	D3DXMATRIX matRotZ, matRotY, matRotX;
	D3DXMatrixRotationZ(&matRotZ, angle);
	D3DXMatrixRotationY(&matRotY, angle);
	D3DXMatrixRotationX(&matRotX, angle);
	D3DXMATRIX matWorld = matRotZ * matRotY * matRotX;
	pDevice->SetTransform(D3DTS_WORLD, &matWorld);

	pEffect->SetTechnique("Technique0");

	D3DXMATRIX worldViewProjection = matWorld * matView * matProjection;
	D3DXMatrixTranspose(&worldViewProjection, &worldViewProjection);
	pEffect->SetMatrix("worldViewProj", &worldViewProjection);

	pEffect->SetTexture("mytex", pTextureCube.get());

	pDevice->SetFVF(cubeVertexFVF);
	pDevice->SetStreamSource(0, pVertexBufferCube.get(), 0, sizeof(CubeVertex));
	pDevice->SetIndices(pIndexBufferCube.get());

	unsigned int uPasses;
	if (SUCCEEDED(pEffect->Begin(&uPasses, 0)))
	{
		for (unsigned int uPass = 0; uPass < uPasses; uPass++)
		{
			pEffect->BeginPass(uPass);
			pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 24, 0, 12);
			pEffect->EndPass();
		}

		pEffect->End();
	}
}
