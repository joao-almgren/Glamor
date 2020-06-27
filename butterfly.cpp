#include "butterfly.h"

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

Butterfly::Butterfly(IDirect3DDevice9* pDevice)
	: mDevice{ pDevice }
	, mVertexBuffer{ nullptr, vertexDeleter }
	, mTexture{ nullptr, textureDeleter }
	, mEffect{ nullptr, effectDeleter }
	, mVertexDeclaration{ nullptr, declarationDeleter }
	, mPos{ 0, 3, 55 }
	, mFlap{ 10 }, mFlapDir{ 1 }, mFlapPower{ 10 }
	, mRoll{ 0 }, mRollDir{ 1 }, mPitch{ 0 }, mPitchDir{ 1 }, mYaw{ 0 }
	, mAngle{ 0 }
{
}

//*********************************************************************************************************************

bool Butterfly::init()
{
	mVertexBuffer.reset(CreateVertexBuffer(mDevice, butterfly, sizeof(Vertex), 6, 0));
	if (!mVertexBuffer)
		return false;

	mVertexDeclaration.reset(CreateDeclaration(mDevice, vertexElement));
	if (!mVertexDeclaration)
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

void Butterfly::draw()
{
	mEffect->SetFloat("Angle", D3DXToRadian(mFlap));

	D3DXMATRIX matWorld, matTrans, matScale, matRotZ, matRotX, matRotY;
	D3DXMatrixScaling(&matScale, 0.25f, 0.25f, 0.25f);
	float x = mPos.x + 5 * sinf(D3DXToRadian(mAngle));
	float z = mPos.z + 5 * cosf(D3DXToRadian(mAngle));
	float y = mPos.y + 0.15f * sinf(D3DXToRadian(mFlap) - 0.5f * D3DX_PI) + 0.65f * cosf(D3DXToRadian(mAngle * 3));
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

	mDevice->SetVertexDeclaration(mVertexDeclaration.get());
	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(Vertex));

	RenderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 4);
	});
}

//*********************************************************************************************************************
