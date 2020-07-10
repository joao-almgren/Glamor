#include "sea.h"
#include "constants.h"

//*********************************************************************************************************************

namespace
{
	constexpr auto uv = 10.0f;

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

	const Vertex sea[]
	{
		{ { -0.5f, 0, -0.5f }, {  0,  0 } },
		{ {  0.5f, 0, -0.5f }, { uv,  0 } },
		{ { -0.5f, 0,  0.5f }, {  0, uv } },
		{ {  0.5f, 0,  0.5f }, { uv, uv } }
	};
}

//*********************************************************************************************************************

Sea::Sea(IDirect3DDevice9* pDevice, IDirect3DTexture9* pReflect, IDirect3DTexture9* pRefract, IDirect3DTexture9* pRefractZ, IDirect3DTexture9* pSurfaceZ, IDirect3DTexture9* pShadowZ)
	: mDevice{ pDevice }
	, mReflect{ pReflect }
	, mRefract{ pRefract }
	, mRefractZ{ pRefractZ }
	, mSurfaceZ{ pSurfaceZ }
	, mShadowZ{ pShadowZ }
	, mVertexBuffer{ MakeVertexBuffer() }
	, mTexture{ MakeTexture(), MakeTexture() }
	, mEffect{ MakeEffect() }
	, mVertexDeclaration{ MakeVertexDeclaration() }
	, mWave{ 0.0f }
{
}

//*********************************************************************************************************************

bool Sea::init()
{
	mVertexBuffer.reset(LoadVertexBuffer(mDevice, sea, sizeof(Vertex), 4, 0));
	if (!mVertexBuffer)
		return false;

	mVertexDeclaration.reset(LoadVertexDeclaration(mDevice, vertexElement));
	if (!mVertexDeclaration)
		return false;

	mTexture[0].reset(LoadTexture(mDevice, L"waterDUDV.png"));
	mTexture[1].reset(LoadTexture(mDevice, L"waterNormal.png"));
	if (!mTexture[0] || !mTexture[1])
		return false;

	mEffect.reset(LoadEffect(mDevice, L"sea.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTexture("Texture0", mReflect);
	mEffect->SetTexture("Texture1", mRefract);
	mEffect->SetTexture("Texture2", mRefractZ);
	mEffect->SetTexture("Texture3", mSurfaceZ);
	mEffect->SetTexture("Texture4", mTexture[0].get());
	mEffect->SetTexture("Texture5", mTexture[1].get());
	mEffect->SetTexture("Texture6", mShadowZ);

	mEffect->SetFloat("NearPlane", gNearPlane);
	mEffect->SetFloat("FarPlane", gFarPlane);
	mEffect->SetInt("ShadowTexSize", gShadowTexSize);

	return true;
}

//*********************************************************************************************************************

void Sea::update(const float tick)
{
	mWave += tick / 1000.0f;
	if (mWave >= 1000)
		mWave = 0;
}

//*********************************************************************************************************************

void Sea::draw(SeaRenderMode mode, const D3DXVECTOR3& camPos, const D3DXMATRIX& matRTTProj, const D3DXMATRIX& matLightViewProj)
{
	if (mode == SeaRenderMode::Plain)
		mEffect->SetTechnique("Plain");
	else if (mode == SeaRenderMode::Underwater)
		mEffect->SetTechnique("Underwater");
	else
		mEffect->SetTechnique("Normal");

	mEffect->SetFloat("Wave", mWave);

	D3DXMATRIX matWorld, matTrans, matScale;
	D3DXMatrixScaling(&matScale, 66 * 3, 1, 66 * 3);
	D3DXMatrixTranslation(&matTrans, 66, 0, 66);
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

	D3DXMatrixTranspose(&matProjection, &matRTTProj);
	mEffect->SetMatrix("RTTProjection", &matProjection);

	D3DXMatrixTranspose(&matProjection, &matLightViewProj);
	mEffect->SetMatrix("LightViewProj", &matProjection);

	mEffect->SetFloatArray("CameraPosition", (float*)&camPos, 3);

	mDevice->SetVertexDeclaration(mVertexDeclaration.get());
	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(Vertex));

	RenderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	});
}

//*********************************************************************************************************************
