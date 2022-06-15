#include "butterfly.h"
#include "camera.h"
#include "config.h"

namespace
{
	constexpr D3DVERTEXELEMENT9 VERTEX_ELEMENT[]
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

	const Vertex BUTTERFLY[]
	{
		{ { -1, 0, -1 }, {    0, 1 } },
		{ { -1, 0,  1 }, {    0, 0 } },
		{ {  0, 0, -1 }, { 0.5f, 1 } },
		{ {  0, 0,  1 }, { 0.5f, 0 } },
		{ {  1, 0, -1 }, {    1, 1 } },
		{ {  1, 0,  1 }, {    1, 0 } }
	};
}

Butterfly::Butterfly(std::shared_ptr<IDirect3DDevice9> pDevice, Camera* pCamera, IDirect3DTexture9* pShadowZ)
	: mDevice{ std::move(pDevice) }
	, mCamera{ pCamera }
	, mShadowZ{ pShadowZ }
	, mVertexBuffer{ makeVertexBuffer() }
	, mTexture{ makeTexture() }
	, mEffect{ makeEffect() }
	, mVertexDeclaration{ makeVertexDeclaration() }
	, mPos{ 0.0f, 3.0f, 55.0f }
	, mFlap{ 10.0f }, mFlapDir{ 1.0f }, mFlapPower{ 10.0f }
	, mRoll{ 0 }, mRollDir{ 1.0f }, mPitch{ 0 }, mPitchDir{ 1.0f }, mYaw{ 0 }
	, mAngle{ 0 }
{
}

bool Butterfly::init()
{
	mVertexBuffer.reset(loadVertexBuffer(mDevice.get(), BUTTERFLY, sizeof(Vertex), 6, 0));
	if (!mVertexBuffer)
		return false;

	mVertexDeclaration.reset(loadVertexDeclaration(mDevice.get(), VERTEX_ELEMENT));
	if (!mVertexDeclaration)
		return false;

	mTexture.reset(loadTexture(mDevice.get(), L"res\\butterfly.png"));
	if (!mTexture)
		return false;

	mEffect.reset(loadEffect(mDevice.get(), L"butterfly.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTexture("TextureDiffuse", mTexture.get());
	mEffect->SetTexture("TextureDepthShadow", mShadowZ);

	mEffect->SetInt("ShadowTexSize", Config::SHADOW_TEX_SIZE);

	mEffect->SetTechnique("Normal");

	return true;
}

void Butterfly::update(const float tick) noexcept
{
	mFlap += mFlapDir * mFlapPower * tick;
	if (mFlap >= 150 || mFlap <= 10)
		mFlapDir = -mFlapDir;

	mRoll += mRollDir * tick;
	if (mRoll >= 30 || mRoll <= -30)
		mRollDir = -mRollDir;

	mPitch += mPitchDir * 0.5f * tick;
	if (mPitch >= 10 || mPitch <= -50)
		mPitchDir = -mPitchDir;

	mAngle += 0.5f * tick;
	if (mAngle >= 360)
		mAngle = 0;

	mYaw = mAngle + 90;
}

void Butterfly::draw(const D3DXMATRIX& matLightViewProj) const
{
	mEffect->SetFloat("Angle", D3DXToRadian(mFlap));

	const float x = mPos.x + 5 * sinf(D3DXToRadian(mAngle));
	const float z = mPos.z + 5 * cosf(D3DXToRadian(mAngle));
	const float y = mPos.y + 0.15f * sinf(D3DXToRadian(mFlap) - 0.5f * D3DX_PI) + 0.65f * cosf(D3DXToRadian(mAngle * 3));

	constexpr float radius = 0.25f;
	const D3DXVECTOR3 center(x, y, z);
	if (!mCamera->isSphereInFrustum(center, radius))
		return;

	D3DXMATRIX matWorld, matTrans, matScale, matRotZ, matRotX, matRotY;
	D3DXMatrixScaling(&matScale, 0.25f, 0.25f, 0.25f);
	D3DXMatrixTranslation(&matTrans, x, y, z);
	D3DXMatrixRotationZ(&matRotZ, D3DXToRadian(mRoll));
	D3DXMatrixRotationX(&matRotX, D3DXToRadian(mPitch));
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

	renderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 4);
	});
}
