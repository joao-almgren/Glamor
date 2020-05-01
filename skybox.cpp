#include "skybox.h"

//*********************************************************************************************************************

namespace
{
	constexpr auto epsilon = 1.0f / 1024.0f;

	const auto skyVertexFVF{ D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) };
	struct SkyVertex
	{
		float x, y, z;
		float u, v;
	};

	const SkyVertex sky[30]
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
	: iMesh(pDevice)
	, pVertexBufferSky(nullptr, vertexDeleter)
	, pTextureSky{ { nullptr, textureDeleter }, { nullptr, textureDeleter }, { nullptr, textureDeleter }, { nullptr, textureDeleter },  { nullptr, textureDeleter } }
	, angle(0.0f)
{
}

//*********************************************************************************************************************

bool Skybox::init()
{
	pVertexBufferSky.reset(CreateVertexBuffer(pDevice, sky, sizeof(SkyVertex), 30, skyVertexFVF));
	if (!pVertexBufferSky)
		return false;

	pTextureSky[0].reset(CreateTexture(pDevice, L"envmap_miramar\\miramar_up.tga"));
	pTextureSky[1].reset(CreateTexture(pDevice, L"envmap_miramar\\miramar_rt.tga"));
	pTextureSky[2].reset(CreateTexture(pDevice, L"envmap_miramar\\miramar_ft.tga"));
	pTextureSky[3].reset(CreateTexture(pDevice, L"envmap_miramar\\miramar_lf.tga"));
	pTextureSky[4].reset(CreateTexture(pDevice, L"envmap_miramar\\miramar_bk.tga"));
	if (!pTextureSky[0] || !pTextureSky[1] || !pTextureSky[2] || !pTextureSky[3] || !pTextureSky[4])
		return false;

	return true;
}

//*********************************************************************************************************************

void Skybox::update(const float tick)
{
	angle += 0.005f * tick;
}

//*********************************************************************************************************************

void Skybox::draw()
{
	D3DXMATRIX matView;
	const D3DXVECTOR3 eye(0.0f, 0.0f, 0.0f);
	const D3DXVECTOR3 at(cosf(angle), 0.0f, sinf(angle));
	const D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&matView, &eye, &at, &up);
	pDevice->SetTransform(D3DTS_VIEW, &matView);

	D3DXMATRIX matWorld;
	D3DXMatrixScaling(&matWorld, 707, 707, 707);
	pDevice->SetTransform(D3DTS_WORLD, &matWorld);

	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	pDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	pDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

	pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	pDevice->SetFVF(skyVertexFVF);
	pDevice->SetStreamSource(0, pVertexBufferSky.get(), 0, sizeof(SkyVertex));

	for (int s = 0; s < 5; s++)
	{
		pDevice->SetTexture(0, pTextureSky[s].get());
		pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, s * 6, 2);
	}

	pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
//	pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
//	pDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

//*********************************************************************************************************************
