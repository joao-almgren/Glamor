#include "cube.h"

//*********************************************************************************************************************

namespace
{
	const unsigned long vertexFVF{ D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_TEXCOORDSIZE2(0) };
	struct Vertex
	{
		float x, y, z;
		float nx, ny, nz;
		unsigned long c;
		float u, v;
	};

	const Vertex vertex[]
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

	const short index[]
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

//*********************************************************************************************************************

Cube::Cube(IDirect3DDevice9* pDevice)
	: iMesh(pDevice)
	, pVertexBuffer(nullptr, vertexDeleter)
	, pIndexBuffer(nullptr, indexDeleter)
	, pTexture(nullptr, textureDeleter)
	, pEffect(nullptr, effectDeleter)
	, angle(0.0f)
{
}

//*********************************************************************************************************************

bool Cube::init()
{
	pVertexBuffer.reset(CreateVertexBuffer(pDevice, vertex, sizeof(Vertex), 24, vertexFVF));
	if (!pVertexBuffer)
		return false;

	pIndexBuffer.reset(CreateIndexBuffer(pDevice, index, 36));
	if (!pIndexBuffer)
		return false;

	pTexture.reset(CreateTexture(pDevice, L"smiley.bmp"));
	if (!pTexture)
		return false;

	pEffect.reset(CreateEffect(pDevice, L"cube.fx"));
	if (!pEffect)
		return false;

	pEffect->SetTechnique("Technique0");
	pEffect->SetTexture("myTexture", pTexture.get());

	return true;
}

//*********************************************************************************************************************

void Cube::update(const float tick)
{
	angle += 0.015f * tick;
}

//*********************************************************************************************************************

void Cube::draw()
{
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

	D3DXMATRIX worldViewProjection = matWorld * matView * matProjection;
	D3DXMatrixTranspose(&worldViewProjection, &worldViewProjection);
	pEffect->SetMatrix("worldViewProj", &worldViewProjection);

	pDevice->SetFVF(vertexFVF);
	pDevice->SetStreamSource(0, pVertexBuffer.get(), 0, sizeof(Vertex));
	pDevice->SetIndices(pIndexBuffer.get());

	RenderEffect(pEffect.get(), [this]()
	{
		pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 24, 0, 12);
	});

	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

//*********************************************************************************************************************
