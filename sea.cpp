#include "sea.h"

//*********************************************************************************************************************

namespace
{
	constexpr auto uv = 16.0f;

	const auto vertexFVF{ D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) };
	struct Vertex
	{
		float x{}, y{}, z{};
		float u{}, v{};
	};

	const Vertex sea[6]
	{
		{ -1, 0, -1,  0,  0 },
		{  1, 0, -1, uv,  0 },
		{ -1, 0,  1,  0, uv },
		{ -1, 0,  1,  0, uv },
		{  1, 0, -1, uv,  0 },
		{  1, 0,  1, uv, uv }
	};
}

//*********************************************************************************************************************

Sea::Sea(IDirect3DDevice9* pDevice)
	: iMesh(pDevice)
	, mVertexBuffer(nullptr, vertexDeleter)
	, mTexture(nullptr, textureDeleter)
	, mEffect(nullptr, effectDeleter)
{
}

//*********************************************************************************************************************

bool Sea::init()
{
	mVertexBuffer.reset(CreateVertexBuffer(mDevice, sea, sizeof(Vertex), 6, vertexFVF));
	if (!mVertexBuffer)
		return false;

	mTexture.reset(CreateTexture(mDevice, L"WaterPlain0012_1_500.tga"));
	if (!mTexture)
		return false;

	mEffect.reset(CreateEffect(mDevice, L"sea.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTechnique("Technique0");
	mEffect->SetTexture("Texture0", mTexture.get());

	return true;
}

//*********************************************************************************************************************

void Sea::update(const float /*tick*/)
{
}

//*********************************************************************************************************************

void Sea::draw()
{
	D3DXMATRIX matProjection;
	mDevice->GetTransform(D3DTS_PROJECTION, &matProjection);

	D3DXMATRIX matView;
	mDevice->GetTransform(D3DTS_VIEW, &matView);

	D3DXMATRIX matWorld, matTrans, matScale;
	D3DXMatrixTranslation(&matTrans, -32, 4.15f, -32);
	D3DXMatrixScaling(&matScale, 128, 1, 128);
	matWorld = matScale * matTrans;
	mDevice->SetTransform(D3DTS_WORLD, &matWorld);

	D3DXMATRIX worldViewProjection = matWorld * matView * matProjection;
	D3DXMatrixTranspose(&worldViewProjection, &worldViewProjection);
	mEffect->SetMatrix("WorldViewProj", &worldViewProjection);

	mDevice->SetFVF(vertexFVF);
	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(Vertex));

	RenderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
	});
}

//*********************************************************************************************************************
