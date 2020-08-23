#include "butterfly.h"
#include "camera.h"
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

Butterfly::Butterfly(IDirect3DDevice9* pDevice, Camera* pCamera, IDirect3DTexture9* pShadowZ)
	: mDevice{ pDevice }
	, mCamera{ pCamera }
	, mShadowZ{ pShadowZ }
	, mVertexBuffer{ MakeVertexBuffer() }
	, mTexture{ MakeTexture() }
	, mEffect{ MakeEffect() }
	, mVertexDeclaration{ MakeVertexDeclaration() }
	, mPos{ 0.0f, 3.0f, 55.0f }
	, mFlap{ 10.0f }, mFlapDir{ 1.0f }, mFlapPower{ 10.0f }
	, mRoll{ 0.0f }, mRollDir{ 1.0f }, mPitch{ 0.0f }, mPitchDir{ 1.0f }, mYaw{ 0.0f }
	, mAngle{ 0.0f }
{
}

//*********************************************************************************************************************

bool Butterfly::init()
{
	mVertexBuffer.reset(LoadVertexBuffer(mDevice, butterfly, sizeof(Vertex), 6, 0));
	if (!mVertexBuffer)
		return false;

	mVertexDeclaration.reset(LoadVertexDeclaration(mDevice, vertexElement));
	if (!mVertexDeclaration)
		return false;

	mTexture.reset(LoadTexture(mDevice, L"res\\butterfly.png"));
	if (!mTexture)
		return false;

	mEffect.reset(LoadEffect(mDevice, L"butterfly.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTexture("TextureDiffuse", mTexture.get());
	mEffect->SetTexture("TextureDepthShadow", mShadowZ);

	mEffect->SetInt("ShadowTexSize", gShadowTexSize);

	mEffect->SetTechnique("Normal");

	return true;
}

//*********************************************************************************************************************

void Butterfly::update(const float /*tick*/)
{
	mFlap += mFlapDir * mFlapPower;
	if (mFlap >= 150 || mFlap <= 10)
	{
		mFlapDir = -mFlapDir;
		if (mFlap >= 150)
		{
			mFlapPower = (float)(10 + rand() % 5);
		}
	}

	mRoll += mRollDir;
	if (mRoll >= 30 || mRoll <= -30)
		mRollDir = -mRollDir;

	mPitch += mPitchDir * 0.5f;
	if (mPitch >= 10 || mPitch <= -50)
		mPitchDir = -mPitchDir;

	mAngle += 0.5f;
	if (mAngle >= 360)
		mAngle = 0;
}

//*********************************************************************************************************************

void Butterfly::draw(const D3DXMATRIX& matLightViewProj)
{
	mEffect->SetFloat("Angle", D3DXToRadian(mFlap));

	float x = mPos.x + 5 * sinf(D3DXToRadian(mAngle));
	float z = mPos.z + 5 * cosf(D3DXToRadian(mAngle));
	float y = mPos.y + 0.15f * sinf(D3DXToRadian(mFlap) - 0.5f * D3DX_PI) + 0.65f * cosf(D3DXToRadian(mAngle * 3));

	float radius = 0.25f;
	D3DXVECTOR3 center(x, y, z);
	if (!mCamera->isSphereInFrustum(center, radius))
		return;

	D3DXMATRIX matWorld, matTrans, matScale, matRotZ, matRotX, matRotY;
	D3DXMatrixScaling(&matScale, 0.25f, 0.25f, 0.25f);
	D3DXMatrixTranslation(&matTrans, x, y, z);
	D3DXMatrixRotationZ(&matRotZ, D3DXToRadian(mRoll));
	D3DXMatrixRotationX(&matRotX, D3DXToRadian(mPitch));
	mYaw = mAngle + 90;
	D3DXMatrixRotationY(&matRotY, D3DXToRadian(mYaw));
	matWorld = matScale * matRotY * matRotX * matRotZ * matTrans;
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

	D3DXMatrixTranspose(&matProjection, &matLightViewProj);
	mEffect->SetMatrix("LightViewProj", &matProjection);

	mDevice->SetVertexDeclaration(mVertexDeclaration.get());
	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(Vertex));

	RenderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 4);
	});
}

//*********************************************************************************************************************
