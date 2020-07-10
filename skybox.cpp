#include "skybox.h"
#include "constants.h"

//*********************************************************************************************************************

namespace
{
	const D3DVERTEXELEMENT9 vertexElement[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 3 * 4, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	struct Vertex
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texcoord;
	};

	const Vertex sky[]
	{
		{ { -0.5f,  0.5f, -0.5f }, { 0, 0 } },
		{ {  0.5f,  0.5f, -0.5f }, { 1, 0 } },
		{ { -0.5f,  0.5f,  0.5f }, { 0, 1 } },
		{ {  0.5f,  0.5f,  0.5f }, { 1, 1 } },

		{ { -0.5f,  0.5f,  0.5f }, { 0, 0 } },
		{ {  0.5f,  0.5f,  0.5f }, { 1, 0 } },
		{ { -0.5f, -0.5f,  0.5f }, { 0, 1 } },
		{ {  0.5f, -0.5f,  0.5f }, { 1, 1 } },

		{ {  0.5f,  0.5f,  0.5f }, { 0, 0 } },
		{ {  0.5f,  0.5f, -0.5f }, { 1, 0 } },
		{ {  0.5f, -0.5f,  0.5f }, { 0, 1 } },
		{ {  0.5f, -0.5f, -0.5f }, { 1, 1 } },

		{ {  0.5f,  0.5f, -0.5f }, { 0, 0 } },
		{ { -0.5f,  0.5f, -0.5f }, { 1, 0 } },
		{ {  0.5f, -0.5f, -0.5f }, { 0, 1 } },
		{ { -0.5f, -0.5f, -0.5f }, { 1, 1 } },

		{ { -0.5f,  0.5f, -0.5f }, { 0, 0 } },
		{ { -0.5f,  0.5f,  0.5f }, { 1, 0 } },
		{ { -0.5f, -0.5f, -0.5f }, { 0, 1 } },
		{ { -0.5f, -0.5f,  0.5f }, { 1, 1 } }
	};
}

//*********************************************************************************************************************

Skybox::Skybox(IDirect3DDevice9* pDevice)
	: mDevice{ pDevice }
	, mVertexBuffer{ MakeVertexBuffer() }
	, mTexture{ MakeTexture(), MakeTexture(), MakeTexture(), MakeTexture(), MakeTexture() }
	, mVertexDeclaration{ MakeVertexDeclaration() }
{
}

//*********************************************************************************************************************

bool Skybox::init()
{
	mVertexBuffer.reset(LoadVertexBuffer(mDevice, sky, sizeof(Vertex), 20, 0));
	if (!mVertexBuffer)
		return false;

	mVertexDeclaration.reset(LoadVertexDeclaration(mDevice, vertexElement));
	if (!mVertexDeclaration)
		return false;

	mTexture[0].reset(LoadTexture(mDevice, L"envmap_miramar\\results\\miramar_up_tga_dxt1_1.dds"));
	mTexture[1].reset(LoadTexture(mDevice, L"envmap_miramar\\results\\miramar_rt_tga_dxt1_1.dds"));
	mTexture[2].reset(LoadTexture(mDevice, L"envmap_miramar\\results\\miramar_ft_tga_dxt1_1.dds"));
	mTexture[3].reset(LoadTexture(mDevice, L"envmap_miramar\\results\\miramar_lf_tga_dxt1_1.dds"));
	mTexture[4].reset(LoadTexture(mDevice, L"envmap_miramar\\results\\miramar_bk_tga_dxt1_1.dds"));
	if (!mTexture[0] || !mTexture[1] || !mTexture[2] || !mTexture[3] || !mTexture[4])
		return false;

	return true;
}

//*********************************************************************************************************************

void Skybox::update(const float /*tick*/)
{
}

//*********************************************************************************************************************

void Skybox::draw(const D3DXVECTOR3& camPos)
{
	D3DXMATRIX matWorld, matScale, matTrans;
	D3DXMatrixScaling(&matScale, gFarPlane, gFarPlane, gFarPlane);
	D3DXMatrixTranslation(&matTrans, camPos.x, camPos.y, camPos.z);
	matWorld = matScale * matTrans;
	mDevice->SetTransform(D3DTS_WORLD, &matWorld);

	mDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	mDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	mDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	mDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

	mDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	mDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	mDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	mDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	mDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	mDevice->SetVertexDeclaration(mVertexDeclaration.get());
	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(Vertex));

	for (int s = 0; s < 5; s++)
	{
		mDevice->SetTexture(0, mTexture[s].get());
		mDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, s * 4, 2);
	}

	mDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

//*********************************************************************************************************************
