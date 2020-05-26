#include "sea.h"

//*********************************************************************************************************************

namespace
{
	constexpr auto uv = 10.0f;

	const auto vertexFVF{ D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) };
	struct Vertex
	{
		float x{}, y{}, z{};
		float u{}, v{};
	};

	const Vertex sea[6]
	{
		{ -0.5f, 0, -0.5f,  0,  0 },
		{  0.5f, 0, -0.5f, uv,  0 },
		{ -0.5f, 0,  0.5f,  0, uv },
		{ -0.5f, 0,  0.5f,  0, uv },
		{  0.5f, 0, -0.5f, uv,  0 },
		{  0.5f, 0,  0.5f, uv, uv }
	};
}

//*********************************************************************************************************************

Sea::Sea(IDirect3DDevice9* pDevice, IDirect3DTexture9* pReflect, IDirect3DTexture9* pRefract)
	: mDevice(pDevice)
	, mReflect(pReflect)
	, mRefract(pRefract)
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

	mTexture.reset(LoadTexture(mDevice, L"WaterPlain0012_1_500.tga"));
	if (!mTexture)
		return false;

	mEffect.reset(CreateEffect(mDevice, L"sea.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTechnique("Technique0");
	mEffect->SetTexture("Texture0", mTexture.get());
	mEffect->SetTexture("Texture1", mReflect);
	mEffect->SetTexture("Texture2", mRefract);

	return true;
}

//*********************************************************************************************************************

void Sea::update(const float /*tick*/)
{
}

//*********************************************************************************************************************

//*********************************************************************************************************************

void Sea::draw(const D3DXMATRIX& matRTTProj, const D3DXVECTOR3& camPos)
{
	D3DXMATRIX matWorld;
	//D3DXMatrixScaling(&matWorld, 66 * 3, 1, 66 * 3);
	D3DXMatrixScaling(&matWorld, 500, 1, 500);
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
	mEffect->SetMatrix("Proj", &matProjection);

	D3DXMatrixTranspose(&matProjection, &matRTTProj);
	mEffect->SetMatrix("RTTProj", &matProjection);

	mEffect->SetFloatArray("CamPos", (float*)&camPos, 3);

	mDevice->SetFVF(vertexFVF);
	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(Vertex));

	RenderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
	});
}

//*********************************************************************************************************************
