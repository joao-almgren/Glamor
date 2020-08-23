#include "statue.h"
#include "wavefront.h"
#include "constants.h"
#include "camera.h"
#include <vector>
#include <string>

//*********************************************************************************************************************

namespace
{
	const D3DVERTEXELEMENT9 vertexElement[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 3 * 4, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 6 * 4, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 },
		{ 0, 9 * 4, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0 },
		{ 0, 12 * 4, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
}

//*********************************************************************************************************************

Statue::Statue(IDirect3DDevice9* pDevice, Camera* pCamera, IDirect3DTexture9* pShadowZ)
	: mDevice{ pDevice }
	, mCamera{ pCamera }
	, mShadowZ{ pShadowZ }
	, mVertexBuffer{ MakeVertexBuffer() }
	, mIndexBuffer{ MakeIndexBuffer() }
	, mTexture{ MakeTexture(), MakeTexture() }
	, mEffect{ MakeEffect() }
	, mVertexDeclaration{ MakeVertexDeclaration() }
	, mIndexCount{ 0 }
	, mSphere{}
{
}

//*********************************************************************************************************************

bool Statue::init()
{
	if (!LoadTbnObject(mDevice, "res\\statue\\statue.obj", mVertexBuffer, mIndexBuffer, mIndexCount, mSphere))
		return false;

	mVertexDeclaration.reset(LoadVertexDeclaration(mDevice, vertexElement));
	if (!mVertexDeclaration)
		return false;

	mTexture[0].reset(LoadTexture(mDevice, L"res\\statue\\statue_texture_tga_dxt1_1.dds"));
	mTexture[1].reset(LoadTexture(mDevice, L"res\\statue\\statue_normal.png"));
	if (!mTexture[0] || !mTexture[1])
		return false;
	
	mEffect.reset(LoadEffect(mDevice, L"statue.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTexture("TextureDiffuse", mTexture[0].get());
	mEffect->SetTexture("TextureNormal", mTexture[1].get());
	mEffect->SetTexture("TextureDepthShadow", mShadowZ);

	mEffect->SetInt("ShadowTexSize", gShadowTexSize);

	return true;
}

//*********************************************************************************************************************

void Statue::update(const float /*tick*/)
{
}

//*********************************************************************************************************************

void Statue::draw(StatueRenderMode mode, const D3DXMATRIX& matLightViewProj)
{
	const D3DXVECTOR3 camPos = mCamera->getPos();

	if (mode == StatueRenderMode::Reflect)
		mEffect->SetTechnique("Reflect");
	else if (mode == StatueRenderMode::Simple)
		mEffect->SetTechnique("Simple");
	else if (mode == StatueRenderMode::Caster)
		mEffect->SetTechnique("Caster");
	else
		mEffect->SetTechnique("Normal");

	if (mode == StatueRenderMode::Normal || mode == StatueRenderMode::Simple || mode == StatueRenderMode::Caster)
	{
		float radius = mSphere.w * 0.5f;
		D3DXVECTOR3 center(mSphere.x * 0.5f, mSphere.y * 0.5f + 0.5f, mSphere.z * 0.5f);
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

	mEffect->SetFloatArray("CameraPosition", (float*)&camPos, 3);

	mDevice->SetVertexDeclaration(mVertexDeclaration.get());
	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(TbnVertex));
	mDevice->SetIndices(mIndexBuffer.get());

	RenderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mIndexCount, 0, mIndexCount / 3);
	});
}

//*********************************************************************************************************************
