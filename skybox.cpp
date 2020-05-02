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
	: iMesh(pDevice)
	, pVertexBuffer(nullptr, vertexDeleter)
	, pTexture{ { nullptr, textureDeleter }, { nullptr, textureDeleter }, { nullptr, textureDeleter }, { nullptr, textureDeleter },  { nullptr, textureDeleter } }
{
}

//*********************************************************************************************************************

bool Skybox::init()
{
	pVertexBuffer.reset(CreateVertexBuffer(pDevice, sky, sizeof(Vertex), 30, vertexFVF));
	if (!pVertexBuffer)
		return false;

	pTexture[0].reset(CreateTexture(pDevice, L"envmap_miramar\\miramar_up.tga"));
	pTexture[1].reset(CreateTexture(pDevice, L"envmap_miramar\\miramar_rt.tga"));
	pTexture[2].reset(CreateTexture(pDevice, L"envmap_miramar\\miramar_ft.tga"));
	pTexture[3].reset(CreateTexture(pDevice, L"envmap_miramar\\miramar_lf.tga"));
	pTexture[4].reset(CreateTexture(pDevice, L"envmap_miramar\\miramar_bk.tga"));
	if (!pTexture[0] || !pTexture[1] || !pTexture[2] || !pTexture[3] || !pTexture[4])
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
	D3DXMATRIX matWorld;
	D3DXMatrixScaling(&matWorld, 500, 500, 500);
	pDevice->SetTransform(D3DTS_WORLD, &matWorld);

	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	pDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	pDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

	pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	pDevice->SetFVF(vertexFVF);
	pDevice->SetStreamSource(0, pVertexBuffer.get(), 0, sizeof(Vertex));

	for (int s = 0; s < 5; s++)
	{
		pDevice->SetTexture(0, pTexture[s].get());
		pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, s * 6, 2);
	}

	//pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	//pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	//pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	//pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	//pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
//	pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
//	pDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

//*********************************************************************************************************************
