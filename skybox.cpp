#include "skybox.h"
#include "constants.h"
#include "camera.h"

namespace
{
	constexpr D3DVERTEXELEMENT9 VERTEX_ELEMENT[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 3 * 4, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	struct Vertex
	{
		[[maybe_unused]] D3DXVECTOR3 position;
		[[maybe_unused]] D3DXVECTOR2 texcoord;
	};

	const Vertex SKY[]
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

Skybox::Skybox(IDirect3DDevice9* pDevice, Camera* pCamera)
	: mDevice{ pDevice }
	, mCamera{ pCamera }
	, mVertexBuffer{ makeVertexBuffer() }
	, mTexture{ makeTexture(), makeTexture(), makeTexture(), makeTexture(), makeTexture() }
	, mEffect{ makeEffect() }
	, mVertexDeclaration{ makeVertexDeclaration() }
{
}

bool Skybox::init()
{
	mVertexBuffer.reset(loadVertexBuffer(mDevice, SKY, sizeof(Vertex), 20, 0));
	if (!mVertexBuffer)
		return false;

	mVertexDeclaration.reset(loadVertexDeclaration(mDevice, VERTEX_ELEMENT));
	if (!mVertexDeclaration)
		return false;

	mEffect.reset(loadEffect(mDevice, L"skybox.fx"));
	if (!mEffect)
		return false;

	mTexture[0].reset(loadTexture(mDevice, L"res\\envmap_miramar\\results\\miramar_up_tga_dxt1_1.dds"));
	mTexture[1].reset(loadTexture(mDevice, L"res\\envmap_miramar\\results\\miramar_rt_tga_dxt1_1.dds"));
	mTexture[2].reset(loadTexture(mDevice, L"res\\envmap_miramar\\results\\miramar_ft_tga_dxt1_1.dds"));
	mTexture[3].reset(loadTexture(mDevice, L"res\\envmap_miramar\\results\\miramar_lf_tga_dxt1_1.dds"));
	mTexture[4].reset(loadTexture(mDevice, L"res\\envmap_miramar\\results\\miramar_bk_tga_dxt1_1.dds"));
	if (!mTexture[0] || !mTexture[1] || !mTexture[2] || !mTexture[3] || !mTexture[4])
		return false;

	return true;
}

void Skybox::update(const float tick) noexcept
{
}

void Skybox::draw() const
{
	const D3DXVECTOR3 camPos = mCamera->getPos();

	D3DXMATRIX matProjection;
	mDevice->GetTransform(D3DTS_PROJECTION, &matProjection);

	D3DXMATRIX matView;
	mDevice->GetTransform(D3DTS_VIEW, &matView);

	D3DXMATRIX matWorld, matScale, matTrans;
	D3DXMatrixScaling(&matScale, FAR_PLANE, FAR_PLANE, FAR_PLANE);
	D3DXMatrixTranslation(&matTrans, camPos.x, camPos.y, camPos.z);
	matWorld = matScale * matTrans;
	mDevice->SetTransform(D3DTS_WORLD, &matWorld);

	D3DXMATRIX matWorldViewProj = matWorld * matView * matProjection;
	D3DXMatrixTranspose(&matWorldViewProj, &matWorldViewProj);
	mEffect->SetMatrix("WorldViewProjection", &matWorldViewProj);

	mDevice->SetVertexDeclaration(mVertexDeclaration.get());
	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(Vertex));

	for (int s = 0; s < 5; s++)
	{
		mEffect->SetTexture("TextureDiffuse", mTexture[s].get());
		renderEffect(mEffect.get(), [this, s]()
		{
			mDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, s * 4, 2);
		});
	}
}
