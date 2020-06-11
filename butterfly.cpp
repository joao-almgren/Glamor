#include "butterfly.h"

//*********************************************************************************************************************

namespace
{
	const auto vertexFVF{ D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) };
	struct Vertex
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texcoord;
	};

	const Vertex butterfly[]
	{
		{ { -1, 0, -1 }, {    0, 1 } },
		{ { -1, 0,  1 }, {    0, 0 } },
		{ {  0, 0, -1 }, { 0.5f, 1 } },
		{ {  0, 0,  1 }, { 0.5f, 0 } },
		{ {  1, 0, -1 }, {    1, 1 } },
		{ {  1, 0,  1 }, {    1, 0 } }
	};
}

//*********************************************************************************************************************

Butterfly::Butterfly(IDirect3DDevice9* pDevice)
	: mDevice(pDevice)
	, mVertexBuffer(nullptr, vertexDeleter)
	, mTexture(nullptr, textureDeleter)
	, mEffect(nullptr, effectDeleter)
	, mFlap(10)
	, mFlapDir(1)
{
}

//*********************************************************************************************************************

bool Butterfly::init()
{
	mVertexBuffer.reset(CreateVertexBuffer(mDevice, butterfly, sizeof(Vertex), 6, vertexFVF));
	if (!mVertexBuffer)
		return false;

	mTexture.reset(LoadTexture(mDevice, L"butterfly.png"));
	if (!mTexture)
		return false;

	mEffect.reset(CreateEffect(mDevice, L"butterfly.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTexture("Texture0", mTexture.get());
	mEffect->SetTechnique("Normal");

	return true;
}

//*********************************************************************************************************************

void Butterfly::update(const float /*tick*/)
{
	mFlap += mFlapDir * 2;
	if (mFlap >= 150 || mFlap <= 10)
		mFlapDir = -mFlapDir;
}

//*********************************************************************************************************************

void Butterfly::draw()
{
	mEffect->SetFloat("Angle", D3DXToRadian(mFlap));

	D3DXMATRIX matWorld, matTrans, matScale;
	D3DXMatrixScaling(&matScale, 0.5f, 0.5f, 0.5f);
	D3DXMatrixTranslation(&matTrans, 35, 25, 10);
	matWorld = matScale * matTrans;
	mDevice->SetTransform(D3DTS_WORLD, &matWorld);
	D3DXMatrixTranspose(&matWorld, &matWorld);
	mEffect->SetMatrix("World", &matWorld);

	D3DXMATRIX matView;
	mDevice->GetTransform(D3DTS_VIEW, &matView);
	D3DXMatrixTranspose(&matView, &matView);
	mEffect->SetMatrix("View", &matView);

	D3DXMATRIX matProjection;
	mDevice->GetTransform(D3DTS_PROJECTION, &matProjection);
	D3DXMatrixTranspose(&matProjection, &matProjection);
	mEffect->SetMatrix("Projection", &matProjection);

	mDevice->SetFVF(vertexFVF);
	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(Vertex));

	RenderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 4);
	});
}

//*********************************************************************************************************************
