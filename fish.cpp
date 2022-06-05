#include "fish.h"
#include "wavefront.h"
#include <vector>

namespace
{
	constexpr D3DVERTEXELEMENT9 VERTEX_ELEMENT[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 3 * 4, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 6 * 4, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 },
		{ 0, 9 * 4, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0 },
		{ 0, 12 * 4, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 1, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
		{ 1, 4 * 4, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
		{ 1, 8 * 4, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3 },
		{ 1, 12 * 4, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4 },
		D3DDECL_END()
	};

	struct Instance
	{
		D3DXVECTOR4 m0;
		D3DXVECTOR4 m1;
		D3DXVECTOR4 m2;
		D3DXVECTOR4 m3;
	};

	constexpr int MAX_INSTANCE_COUNT = 2;
	Instance instance[MAX_INSTANCE_COUNT];
}

Fish::Fish(IDirect3DDevice9* pDevice)
	: mDevice{ pDevice }
	, mVertexBuffer{ makeVertexBuffer() }
	, mIndexBuffer{ makeIndexBuffer() }
	, mInstanceBuffer{ makeVertexBuffer() }
	, mTexture{ makeTexture() }
	, mEffect{ makeEffect() }
	, mVertexDeclaration{ makeVertexDeclaration() }
	, mIndexCount{}
	, mAngle{}
{
}

bool Fish::init()
{
	if (!loadTbnObject(mDevice, "res\\fish\\tropicalfish12.obj", mVertexBuffer, mIndexBuffer, mIndexCount, mSphere))
		return false;

	if (!createInstances())
		return false;

	mVertexDeclaration.reset(loadVertexDeclaration(mDevice, VERTEX_ELEMENT));
	if (!mVertexDeclaration)
		return false;

	mTexture.reset(loadTexture(mDevice, L"res\\fish\\results\\tropicalfish12_jpg_dxt1_1.dds"));
	if (!mTexture)
		return false;

	mEffect.reset(loadEffect(mDevice, L"fish.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTexture("TextureDiffuse", mTexture.get());

	return true;
}

void Fish::update(const float tick) noexcept
{
	mAngle += 3.0f * tick;
	if (mAngle >= 360)
		mAngle = 0;
}

void Fish::draw(const FishRenderMode mode) const
{
	if (mode == FishRenderMode::REFLECT)
		mEffect->SetTechnique("Reflect");
	else
		mEffect->SetTechnique("Normal");

	mEffect->SetFloat("Angle", D3DXToRadian(mAngle));

	D3DXMATRIX matProjection;
	mDevice->GetTransform(D3DTS_PROJECTION, &matProjection);
	D3DXMatrixTranspose(&matProjection, &matProjection);
	mEffect->SetMatrix("Projection", &matProjection);

	D3DXMATRIX matView;
	mDevice->GetTransform(D3DTS_VIEW, &matView);
	D3DXMatrixTranspose(&matView, &matView);
	mEffect->SetMatrix("View", &matView);

	mDevice->SetVertexDeclaration(mVertexDeclaration.get());

	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(TbnVertex));
	mDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | MAX_INSTANCE_COUNT));

	mDevice->SetStreamSource(1, mInstanceBuffer.get(), 0, sizeof(Instance));
	mDevice->SetStreamSourceFreq(1, (D3DSTREAMSOURCE_INSTANCEDATA | 1ul));

	mDevice->SetIndices(mIndexBuffer.get());

	renderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mIndexCount, 0, mIndexCount / 3);
	});

	mDevice->SetStreamSourceFreq(0, 1);
	mDevice->SetStreamSourceFreq(1, 1);
	mDevice->SetStreamSource(1, nullptr, 0, 0);
}

bool Fish::createInstances()
{
	D3DXMATRIX matTrans;
	D3DXMatrixTranslation(&matTrans, -10, -1, 40);

	float s = 0.005f;
	D3DXMATRIX matScale;
	D3DXMatrixScaling(&matScale, s, s, s);

	D3DXMATRIX matRotY;
	D3DXMatrixRotationY(&matRotY, D3DXToRadian(rand() % 360));  // NOLINT(concurrency-mt-unsafe)

	D3DXMATRIX matWorld = matRotY * matScale * matTrans;
	D3DXMatrixTranspose(&matWorld, &matWorld);
	for (int n = 0; n < 4; n++)
	{
		instance[0].m0[n] = matWorld.m[0][n];
		instance[0].m1[n] = matWorld.m[1][n];
		instance[0].m2[n] = matWorld.m[2][n];
		instance[0].m3[n] = matWorld.m[3][n];
	}

	D3DXMatrixTranslation(&matTrans, -11, -1.5, 40);

	s = 0.003f;
	D3DXMatrixScaling(&matScale, s, s, s);

	matWorld = matRotY * matScale * matTrans;
	D3DXMatrixTranspose(&matWorld, &matWorld);
	for (int n = 0; n < 4; n++)
	{
		instance[1].m0[n] = matWorld.m[0][n];
		instance[1].m1[n] = matWorld.m[1][n];
		instance[1].m2[n] = matWorld.m[2][n];
		instance[1].m3[n] = matWorld.m[3][n];
	}

	mInstanceBuffer.reset(loadVertexBuffer(mDevice, instance, sizeof(Instance), MAX_INSTANCE_COUNT, 0));
	if (!mInstanceBuffer)
		return false;

	return true;
}
