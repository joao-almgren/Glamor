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

Sea::Sea(IDirect3DDevice9* pDevice, IDirect3DTexture9* pReflect, IDirect3DTexture9* pRefract, IDirect3DTexture9* pRefractZ, IDirect3DTexture9* pSurfaceZ)
	: mDevice(pDevice)
	, mReflect(pReflect)
	, mRefract(pRefract)
	, mRefractZ(pRefractZ)
	, mSurfaceZ(pSurfaceZ)
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

	mEffect->SetTexture("Texture0", mTexture.get());
	mEffect->SetTexture("Texture1", mReflect);
	mEffect->SetTexture("Texture2", mRefract);
	mEffect->SetTexture("Texture3", mRefractZ);
	mEffect->SetTexture("Texture4", mSurfaceZ);

	return true;
}

//*********************************************************************************************************************

void Sea::update(const float /*tick*/)
{
}

//*********************************************************************************************************************

void Sea::draw(SeaRenderMode mode, const D3DXMATRIX& matRTTProj)
{
	if (mode == SeaRenderMode::Plain)
		mEffect->SetTechnique("Technique1");
	else
		mEffect->SetTechnique("Technique0");

	D3DXMATRIX matWorld;
	//D3DXMatrixScaling(&matWorld, 66 * 3, 1, 66 * 3);
	D3DXMatrixScaling(&matWorld, 500, 1, 500);
	mDevice->SetTransform(D3DTS_WORLD, &matWorld);
	D3DXMatrixTranspose(&matWorld, &matWorld);
	mEffect->SetMatrix("World", &matWorld);

	D3DXMATRIX matView;
	mDevice->GetTransform(D3DTS_VIEW, &matView);

	D3DXVECTOR4 seaNormal(0, 1, 0, 0);
	D3DXVec4Transform(&seaNormal, &seaNormal, &matView);
	mEffect->SetFloatArray("ViewSeaNormal", (float*)&seaNormal, 3);

	D3DXMatrixTranspose(&matView, &matView);
	mEffect->SetMatrix("View", &matView);

	D3DXMATRIX matProjection;
	mDevice->GetTransform(D3DTS_PROJECTION, &matProjection);
	D3DXMatrixTranspose(&matProjection, &matProjection);
	mEffect->SetMatrix("Projection", &matProjection);

	D3DXMatrixTranspose(&matProjection, &matRTTProj);
	mEffect->SetMatrix("RTTProjection", &matProjection);

	mDevice->SetFVF(vertexFVF);
	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(Vertex));

	RenderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
	});
}

//*********************************************************************************************************************
