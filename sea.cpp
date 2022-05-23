#include "sea.h"
#include "constants.h"
#include "camera.h"

namespace
{
	constexpr auto UV = 10.0f;

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

	const Vertex SEA[]
	{
		{ { -0.5f, 0, -0.5f }, {  0,  0 } },
		{ {  0.5f, 0, -0.5f }, { UV,  0 } },
		{ { -0.5f, 0,  0.5f }, {  0, UV } },
		{ {  0.5f, 0,  0.5f }, { UV, UV } }
	};
}

Sea::Sea(IDirect3DDevice9* pDevice, Camera* pCamera, IDirect3DTexture9* pReflect, IDirect3DTexture9* pRefract,
	IDirect3DTexture9* pRefractZ, IDirect3DTexture9* pSurfaceZ, IDirect3DTexture9* pShadowZ)
	: mDevice{ pDevice }
	, mCamera{ pCamera }
	, mReflect{ pReflect }
	, mRefract{ pRefract }
	, mRefractZ{ pRefractZ }
	, mSurfaceZ{ pSurfaceZ }
	, mShadowZ{ pShadowZ }
	, mVertexBuffer{ makeVertexBuffer() }
	, mTexture{ makeTexture(), makeTexture() }
	, mEffect{ makeEffect() }
	, mVertexDeclaration{ makeVertexDeclaration() }
	, mWave{}
{
}

bool Sea::init()
{
	mVertexBuffer.reset(loadVertexBuffer(mDevice, SEA, sizeof(Vertex), 4, 0));
	if (!mVertexBuffer)
		return false;

	mVertexDeclaration.reset(loadVertexDeclaration(mDevice, VERTEX_ELEMENT));
	if (!mVertexDeclaration)
		return false;

	mTexture[0].reset(loadTexture(mDevice, L"res\\waterDUDV.png"));
	mTexture[1].reset(loadTexture(mDevice, L"res\\waterNormal.png"));
	if (!mTexture[0] || !mTexture[1])
		return false;

	mEffect.reset(loadEffect(mDevice, L"sea.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTexture("TextureDiffuseReflect", mReflect);
	mEffect->SetTexture("TextureDiffuseRefract", mRefract);
	mEffect->SetTexture("TextureDepthBottom", mRefractZ);
	mEffect->SetTexture("TextureDepthSurface", mSurfaceZ);
	mEffect->SetTexture("TextureDUDV", mTexture[0].get());
	mEffect->SetTexture("TextureNormal", mTexture[1].get());
	mEffect->SetTexture("TextureDepthShadow", mShadowZ);

	mEffect->SetFloat("NearPlane", NEAR_PLANE);
	mEffect->SetFloat("FarPlane", FAR_PLANE);
	mEffect->SetInt("ShadowTexSize", SHADOW_TEX_SIZE);

	return true;
}

void Sea::update(const float tick)
{
	mWave += tick / 1000.0f;
	if (mWave >= 1000)
		mWave = 0;
}

void Sea::draw(const SeaRenderMode mode, const D3DXMATRIX& matRTTProj, const D3DXMATRIX& matLightViewProj) const
{
	const D3DXVECTOR3 camPos = mCamera->getPos();

	if (mode == SeaRenderMode::PLAIN)
		mEffect->SetTechnique("Plain");
	else if (mode == SeaRenderMode::UNDERWATER)
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

	mEffect->SetFloatArray("CameraPosition", reinterpret_cast<const float*>(&camPos), 3);

	mDevice->SetVertexDeclaration(mVertexDeclaration.get());
	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(Vertex));

	renderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	});
}
