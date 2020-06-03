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
	, mTexture{ {nullptr, textureDeleter}, {nullptr, textureDeleter} }
	, mEffect(nullptr, effectDeleter)
	, mWave{}
{
}

//*********************************************************************************************************************

bool Sea::init()
{
	mVertexBuffer.reset(CreateVertexBuffer(mDevice, sea, sizeof(Vertex), 6, vertexFVF));
	if (!mVertexBuffer)
		return false;

	mTexture[0].reset(LoadTexture(mDevice, L"waterDUDV.png"));
	mTexture[1].reset(LoadTexture(mDevice, L"waterNormal.png"));
	if (!mTexture[0] || !mTexture[1])
		return false;

	mEffect.reset(CreateEffect(mDevice, L"sea.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTexture("Texture0", mReflect);
	mEffect->SetTexture("Texture1", mRefract);
	mEffect->SetTexture("Texture2", mRefractZ);
	mEffect->SetTexture("Texture3", mSurfaceZ);
	mEffect->SetTexture("Texture4", mTexture[0].get());
	mEffect->SetTexture("Texture5", mTexture[1].get());

	return true;
}

//*********************************************************************************************************************

void Sea::update(const float tick)
{
	mWave += tick / 1000.0f;
	if (mWave >= 1000)
		mWave = 0;

	mEffect->SetFloat("Wave", mWave);
}

//*********************************************************************************************************************

void Sea::draw(SeaRenderMode mode, const D3DXMATRIX& matRTTProj, const D3DXVECTOR3& camPos)
{
	if (mode == SeaRenderMode::Plain)
		mEffect->SetTechnique("Technique1");
	else
		mEffect->SetTechnique("Technique0");

	D3DXMATRIX matWorld, matTrans, matScale;
	//D3DXMatrixScaling(&matScale, 1000, 1, 1000);
	D3DXMatrixScaling(&matScale, 66 * 3, 1, 66 * 3);
	D3DXMatrixTranslation(&matTrans, 66, 0, 66);
	matWorld = matScale * matTrans;
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

	mEffect->SetFloatArray("CameraPosition", (float*)&camPos, 3);

	mDevice->SetFVF(vertexFVF);
	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(Vertex));

	RenderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
	});
}

//*********************************************************************************************************************
