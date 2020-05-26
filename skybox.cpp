#include "skybox.h"

//*********************************************************************************************************************

namespace
{
	constexpr auto epsilon = 1.0f / 1024.0f;

	const auto vertexFVF{ D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) };
	struct Vertex
	{
		float x{}, y{}, z{};
		float u{}, v{};
	};

	const Vertex sky[30]
	{
		{ -1, 1, -1, epsilon, epsilon },
		{ 1, 1, -1, 1 - epsilon, epsilon },
		{ 1, 1, 1, 1 - epsilon, 1 - epsilon },
		{ -1, 1, -1, epsilon, epsilon },
		{ 1, 1, 1, 1 - epsilon, 1 - epsilon },
		{ -1, 1, 1, epsilon, 1 - epsilon },

		{ -1, 1, 1, epsilon, epsilon },
		{ 1, 1, 1, 1 - epsilon, epsilon },
		{ 1, -1, 1, 1 - epsilon, 1 - epsilon },
		{ -1, 1, 1, epsilon, epsilon },
		{ 1, -1, 1, 1 - epsilon, 1 - epsilon },
		{ -1, -1, 1, epsilon, 1 - epsilon },

		{ 1, 1, 1, epsilon, epsilon },
		{ 1, 1, -1, 1 - epsilon, epsilon },
		{ 1, -1, -1, 1 - epsilon, 1 - epsilon },
		{ 1, 1, 1, epsilon, epsilon },
		{ 1, -1, -1, 1 - epsilon, 1 - epsilon },
		{ 1, -1, 1, epsilon, 1 - epsilon },

		{ 1, 1, -1, epsilon, epsilon },
		{ -1, 1, -1, 1 - epsilon, epsilon },
		{ -1, -1, -1, 1 - epsilon, 1 - epsilon },
		{ 1, 1, -1, epsilon, epsilon },
		{ -1, -1, -1, 1 - epsilon, 1 - epsilon },
		{ 1, -1, -1, epsilon, 1 - epsilon },

		{ -1, 1, -1, epsilon, epsilon },
		{ -1, 1, 1, 1 - epsilon, epsilon },
		{ -1, -1, 1, 1 - epsilon, 1 - epsilon },
		{ -1, 1, -1, epsilon, epsilon },
		{ -1, -1, 1, 1 - epsilon, 1 - epsilon },
		{ -1, -1, -1, epsilon, 1 - epsilon }
	};
}

//*********************************************************************************************************************

Skybox::Skybox(IDirect3DDevice9* pDevice)
	: mDevice(pDevice)
	, mVertexBuffer(nullptr, vertexDeleter)
	, mTexture{ { nullptr, textureDeleter }, { nullptr, textureDeleter }, { nullptr, textureDeleter }, { nullptr, textureDeleter },  { nullptr, textureDeleter } }
{
}

//*********************************************************************************************************************

bool Skybox::init()
{
	mVertexBuffer.reset(CreateVertexBuffer(mDevice, sky, sizeof(Vertex), 30, vertexFVF));
	if (!mVertexBuffer)
		return false;

	mTexture[0].reset(LoadTexture(mDevice, L"envmap_miramar\\miramar_up.tga"));
	mTexture[1].reset(LoadTexture(mDevice, L"envmap_miramar\\miramar_rt.tga"));
	mTexture[2].reset(LoadTexture(mDevice, L"envmap_miramar\\miramar_ft.tga"));
	mTexture[3].reset(LoadTexture(mDevice, L"envmap_miramar\\miramar_lf.tga"));
	mTexture[4].reset(LoadTexture(mDevice, L"envmap_miramar\\miramar_bk.tga"));
	if (!mTexture[0] || !mTexture[1] || !mTexture[2] || !mTexture[3] || !mTexture[4])
		return false;

	return true;
}

//*********************************************************************************************************************

void Skybox::update(const float /*tick*/)
{
}

//*********************************************************************************************************************

void Skybox::draw()
{
	D3DXMATRIX matWorld, matScale, matTrans;
	D3DXMatrixScaling(&matScale, 500, 500, 500);
	D3DXMatrixTranslation(&matTrans, mPos.x, mPos.y, mPos.z);
	matWorld = matScale * matTrans;
	mDevice->SetTransform(D3DTS_WORLD, &matWorld);

	mDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	mDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	mDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	mDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	mDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

	mDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	mDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	mDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	mDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	mDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	mDevice->SetFVF(vertexFVF);
	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(Vertex));

	for (int s = 0; s < 5; s++)
	{
		mDevice->SetTexture(0, mTexture[s].get());
		mDevice->DrawPrimitive(D3DPT_TRIANGLELIST, s * 6, 2);
	}

	mDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

//*********************************************************************************************************************

void Skybox::setPos(const D3DXVECTOR3& pos)
{
	mPos = pos;
}

//*********************************************************************************************************************
