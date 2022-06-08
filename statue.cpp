#include "statue.h"
#include "wavefront.h"
#include "config.h"
#include "camera.h"

namespace
{
	constexpr D3DVERTEXELEMENT9 VERTEX_ELEMENT[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 3 * 4, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 6 * 4, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 },
		{ 0, 9 * 4, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0 },
		{ 0, 12 * 4, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
}

Statue::Statue(IDirect3DDevice9* pDevice, Camera* pCamera, IDirect3DTexture9* pShadowZ)
	: mDevice{ pDevice }
	, mCamera{ pCamera }
	, mShadowZ{ pShadowZ }
	, mVertexBuffer{ makeVertexBuffer() }
	, mIndexBuffer{ makeIndexBuffer() }
	, mTexture{ makeTexture(), makeTexture() }
	, mEffect{ makeEffect() }
	, mVertexDeclaration{ makeVertexDeclaration() }
	, mIndexCount{ 0 }
{
}

bool Statue::init()
{
	if (!loadTbnObject(mDevice, "res\\statue\\statue.obj", mVertexBuffer, mIndexBuffer, mIndexCount, mSphere))
		return false;

	mVertexDeclaration.reset(loadVertexDeclaration(mDevice, VERTEX_ELEMENT));
	if (!mVertexDeclaration)
		return false;

	mTexture[0].reset(loadTexture(mDevice, L"res\\statue\\statue_texture_tga_dxt1_1.dds"));
	mTexture[1].reset(loadTexture(mDevice, L"res\\statue\\statue_normal.png"));
	if (!mTexture[0] || !mTexture[1])
		return false;
	
	mEffect.reset(loadEffect(mDevice, L"statue.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTexture("TextureDiffuse", mTexture[0].get());
	mEffect->SetTexture("TextureNormal", mTexture[1].get());
	mEffect->SetTexture("TextureDepthShadow", mShadowZ);

	mEffect->SetInt("ShadowTexSize", Config::SHADOW_TEX_SIZE);

	return true;
}

void Statue::update(const float tick) noexcept
{
}

void Statue::draw(const StatueRenderMode mode, const D3DXMATRIX& matLightViewProj) const
{
	const D3DXVECTOR3 camPos = mCamera->getPos();

	if (mode == StatueRenderMode::REFLECT)
		mEffect->SetTechnique("Reflect");
	else if (mode == StatueRenderMode::SIMPLE)
		mEffect->SetTechnique("Simple");
	else if (mode == StatueRenderMode::CASTER)
		mEffect->SetTechnique("Caster");
	else
		mEffect->SetTechnique("Normal");

	if (mode == StatueRenderMode::NORMAL || mode == StatueRenderMode::SIMPLE || mode == StatueRenderMode::CASTER)
	{
		const float radius = mSphere.w * 0.5f;
		const D3DXVECTOR3 center(mSphere.x * 0.5f, mSphere.y * 0.5f + 0.5f, mSphere.z * 0.5f);
		if (!mCamera->isSphereInFrustum(center, radius))
			return;
	}

	D3DXMATRIX matProjection;
	mDevice->GetTransform(D3DTS_PROJECTION, &matProjection);
	D3DXMatrixTranspose(&matProjection, &matProjection);
	mEffect->SetMatrix("Projection", &matProjection);

	D3DXMATRIX matView;
	mDevice->GetTransform(D3DTS_VIEW, &matView);
	D3DXMatrixTranspose(&matView, &matView);
	mEffect->SetMatrix("View", &matView);

	D3DXMATRIX matWorld, matTrans, matScale;
	D3DXMatrixScaling(&matScale, 0.5f, 0.5f, 0.5f);
	D3DXMatrixTranslation(&matTrans, 0, 0.5f, 0);
	matWorld = matScale * matTrans;
	mDevice->SetTransform(D3DTS_WORLD, &matWorld);
	D3DXMatrixTranspose(&matWorld, &matWorld);
	mEffect->SetMatrix("World", &matWorld);

	D3DXMatrixTranspose(&matProjection, &matLightViewProj);
	mEffect->SetMatrix("LightViewProj", &matProjection);

	mEffect->SetFloatArray("CameraPosition", reinterpret_cast<const float*>(&camPos), 3);

	mDevice->SetVertexDeclaration(mVertexDeclaration.get());
	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(TbnVertex));
	mDevice->SetIndices(mIndexBuffer.get());

	renderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mIndexCount, 0, mIndexCount / 3);
	});
}
