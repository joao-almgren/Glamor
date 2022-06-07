#include "grass.h"
#include "random.h"
#include "wavefront.h"
#include "config.h"
#include "camera.h"
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

	constexpr int MAX_INSTANCE_COUNT = 1000;
	Instance instance[MAX_INSTANCE_COUNT];
}

Grass::Grass(IDirect3DDevice9* pDevice, Camera* pCamera, IDirect3DTexture9* pShadowZ)
	: mDevice{ pDevice }
	, mCamera{ pCamera }
	, mShadowZ{ pShadowZ }
	, mVertexBuffer{ makeVertexBuffer() }
	, mIndexBuffer{ makeIndexBuffer() }
	, mInstanceBuffer{ makeVertexBuffer() }
	, mTexture{ makeTexture() }
	, mEffect{ makeEffect() }
	, mVertexDeclaration{ makeVertexDeclaration() }
	, mIndexCount{}
	, mCamPos{ 0.0f, 0.0f, 0.0f }
	, mCamDir{ 0.0f, 1.0f, 0.0f }
	, mInstanceCount{}
	, mHeight{ nullptr }
	, mAngle{ nullptr }
{
}

bool Grass::init(const std::function<float(float, float)>& height, const std::function<float(float, float)>& angle)
{
	mHeight = height;
	mAngle = angle;

	if (!loadTbnObject(mDevice, "res\\grass\\grass2.obj", mVertexBuffer, mIndexBuffer, mIndexCount, mSphere))
		return false;

	mInstanceBuffer.reset(loadVertexBuffer(mDevice, instance, sizeof(Instance), MAX_INSTANCE_COUNT, 0));
	if (!mInstanceBuffer)
		return false;

	createInstances();

	mVertexDeclaration.reset(loadVertexDeclaration(mDevice, VERTEX_ELEMENT));
	if (!mVertexDeclaration)
		return false;

	mTexture.reset(loadTexture(mDevice, L"res\\grass\\grass.png"));
	if (!mTexture)
		return false;

	mEffect.reset(loadEffect(mDevice, L"grass.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTexture("TextureDiffuse", mTexture.get());
	mEffect->SetTexture("TextureDepthShadow", mShadowZ);

	mEffect->SetInt("ShadowTexSize", Config::SHADOW_TEX_SIZE);

	return true;
}

void Grass::update(const float tick)
{
	const D3DXVECTOR3 camPos = mCamera->getPos();
	const float a = camPos.x - mCamPos.x;
	const float b = camPos.z - mCamPos.z;
	const float d = sqrtf(a * a + b * b);

	const D3DXVECTOR3 camDir = mCamera->getDir();
	const float f = D3DXVec3Dot(&camDir, &mCamDir);

	if (d > 5 || f < 0.99f)
	{
		mCamPos = camPos;
		mCamDir = camDir;
		createInstances();
	}
}

void Grass::draw(const GrassRenderMode mode, const D3DXMATRIX& matLightViewProj) const
{
	if (mode == GrassRenderMode::PLAIN)
		mEffect->SetTechnique("Plain");
	else
		mEffect->SetTechnique("Blend");

	D3DXMATRIX matProjection;
	mDevice->GetTransform(D3DTS_PROJECTION, &matProjection);
	D3DXMatrixTranspose(&matProjection, &matProjection);
	mEffect->SetMatrix("Projection", &matProjection);

	D3DXMATRIX matView;
	mDevice->GetTransform(D3DTS_VIEW, &matView);
	D3DXMatrixTranspose(&matView, &matView);
	mEffect->SetMatrix("View", &matView);

	D3DXMatrixTranspose(&matProjection, &matLightViewProj);
	mEffect->SetMatrix("LightViewProj", &matProjection);

	mDevice->SetVertexDeclaration(mVertexDeclaration.get());

	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(TbnVertex));
	mDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | mInstanceCount));

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

void Grass::createInstances()
{
	Hash hash;
	hash.setseed(1);
	Random random;

	mInstanceCount = 0;
	for (int j = 0; j < 65; j++)
	{
		for (int i = 0; i < 65; i++)
		{
			unsigned int s = (int)(mCamPos.x) + i;
			unsigned int t = (int)(mCamPos.z) + j;

			random.setseed(hash(s, t));
			unsigned int r = random() % 100;

			if (r > 50)
			{
				float x = ((int)(mCamPos.x) + (i - 32) + (float)(random() % 10) * 0.01f);
				float z = ((int)(mCamPos.z) + (j - 32) + (float)(random() % 10) * 0.01f);

				float y = mHeight(x, z) - 0.15f;
				if (y < 1)
					continue;

				float a = mAngle(x, z);
				if (a < 0.5f)
					continue;

				D3DXMATRIX matTrans;
				D3DXMatrixTranslation(&matTrans, x, y, z);

				D3DXMATRIX matScale;
				float c = 0.1f + (float)(random() % 5) * 0.1f;
				D3DXMatrixScaling(&matScale, c, c, c);

				const float radius = mSphere.w * c;
				const D3DXVECTOR3 center(mSphere.x * c + x, mSphere.y * c + y, mSphere.z * c + z);
				if (!mCamera->isSphereInFrustum(center, radius))
					continue;

				D3DXMATRIX matRotY;
				D3DXMatrixRotationY(&matRotY, D3DXToRadian(random() % 360));

				D3DXMATRIX matWorld = matRotY * matScale * matTrans;
				D3DXMatrixTranspose(&matWorld, &matWorld);
				for (int n = 0; n < 4; n++)
				{
					instance[mInstanceCount].m0[n] = matWorld.m[0][n];
					instance[mInstanceCount].m1[n] = matWorld.m[1][n];
					instance[mInstanceCount].m2[n] = matWorld.m[2][n];
					instance[mInstanceCount].m3[n] = matWorld.m[3][n];
				}

				mInstanceCount++;
			}

			if (mInstanceCount >= MAX_INSTANCE_COUNT)
				break;
		}

		if (mInstanceCount >= MAX_INSTANCE_COUNT)
			break;
	}

	void* pData{ nullptr };
	IDirect3DVertexBuffer9* pVertexBuffer = mInstanceBuffer.get();
	if (SUCCEEDED(pVertexBuffer->Lock(0, 0, &pData, 0)))
	{
		memcpy(pData, instance, mInstanceCount * sizeof(Instance));
		pVertexBuffer->Unlock();
	}
}
