#include "sea.h"

//*********************************************************************************************************************

namespace
{
	const auto vertexFVF{ D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) };
	struct Vertex
	{
		float x{}, y{}, z{};
		float u{}, v{};
	};

	const Vertex sea[6]
	{
		{ -1, 4, -1, 0, 0 },
		{  1, 4, -1, 16, 0 },
		{ -1, 4,  1, 0, 16 },
		{ -1, 4,  1, 0, 16 },
		{  1, 4, -1, 16, 0 },
		{  1, 4,  1, 16, 16 }
	};
}

//*********************************************************************************************************************

Sea::Sea(IDirect3DDevice9* pDevice)
	: iMesh(pDevice)
	, mVertexBuffer(nullptr, vertexDeleter)
	, mTexture(nullptr, textureDeleter)
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

	return true;
}

//*********************************************************************************************************************

void Sea::update(const float /*tick*/)
{
}

//*********************************************************************************************************************

void Sea::draw()
{
	D3DXMATRIX matWorld;
	D3DXMatrixScaling(&matWorld, 128, 1, 128);
	mDevice->SetTransform(D3DTS_WORLD, &matWorld);

	//mDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	mDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	mDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	mDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	mDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

	mDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	mDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	mDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);
	mDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	mDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	mDevice->SetTexture(0, mTexture.get());

	mDevice->SetFVF(vertexFVF);
	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(Vertex));
	mDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);

	mDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	//mDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

//*********************************************************************************************************************
