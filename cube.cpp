#include "cube.h"

//*********************************************************************************************************************

namespace
{
	const unsigned long vertexFVF{ D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_TEXCOORDSIZE2(0) };
	struct Vertex
	{
		float x{}, y{}, z{};
		float nx{}, ny{}, nz{};
		unsigned long c{};
		float u{}, v{};
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
	: mDevice(pDevice)
	, mVertexBuffer(nullptr, vertexDeleter)
	, mIndexBuffer(nullptr, indexDeleter)
	, mTexture(nullptr, textureDeleter)
	, mEffect(nullptr, effectDeleter)
	, mAngle(0.0f)
	, mPos{}
{
}

//*********************************************************************************************************************

bool Cube::init()
{
	mVertexBuffer.reset(CreateVertexBuffer(mDevice, vertex, sizeof(Vertex), 24, vertexFVF));
	if (!mVertexBuffer)
		return false;

	mIndexBuffer.reset(CreateIndexBuffer(mDevice, index, 36));
	if (!mIndexBuffer)
		return false;

	mTexture.reset(LoadTexture(mDevice, L"smiley.bmp"));
	if (!mTexture)
		return false;

	mEffect.reset(CreateEffect(mDevice, L"cube.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTechnique("Technique0");
	mEffect->SetTexture("myTexture", mTexture.get());

	return true;
}

//*********************************************************************************************************************

void Cube::update(const float tick)
{
	mAngle += 0.015f * tick;
}

//*********************************************************************************************************************

void Cube::draw()
{
	D3DXMATRIX matProjection;
	mDevice->GetTransform(D3DTS_PROJECTION, &matProjection);

	D3DXMATRIX matView;
	mDevice->GetTransform(D3DTS_VIEW, &matView);

	D3DXMATRIX matRotZ, matRotY, matRotX, matTrans, matScale;
	D3DXMatrixScaling(&matScale, 2.0f, 2.0f, 2.0f);
	D3DXMatrixRotationZ(&matRotZ, mAngle);
	D3DXMatrixRotationY(&matRotY, mAngle);
	D3DXMatrixRotationX(&matRotX, mAngle);
	D3DXMatrixTranslation(&matTrans, mPos.x, mPos.y, mPos.z);
	D3DXMATRIX matWorld = matScale * matRotZ * matRotY * matRotX * matTrans;
	mDevice->SetTransform(D3DTS_WORLD, &matWorld);

	D3DXMATRIX worldViewProjection = matWorld * matView * matProjection;
	D3DXMatrixTranspose(&worldViewProjection, &worldViewProjection);
	mEffect->SetMatrix("worldViewProj", &worldViewProjection);

	mDevice->SetFVF(vertexFVF);
	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(Vertex));
	mDevice->SetIndices(mIndexBuffer.get());

	RenderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 24, 0, 12);
	});
}

//*********************************************************************************************************************

void Cube::setPos(const D3DXVECTOR3& pos)
{
	mPos = pos;
}

//*********************************************************************************************************************
