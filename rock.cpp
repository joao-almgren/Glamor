#include "rock.h"
#include "random.h"
#include "wavefront.h"
#include "constants.h"
#include "camera.h"
#include <vector>
#include <string>

namespace
{
	const D3DVERTEXELEMENT9 vertexElement[] =
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

	const int maxInstanceCount = 50;

	const char* const lodFx[3] =
	{
		"Normal",
		"Simple",
		"Simple"
	};
}

Rock::Rock(IDirect3DDevice9* pDevice, Camera* pCamera, IDirect3DTexture9* pShadowZ)
	: mDevice{ pDevice }
	, mCamera{ pCamera }
	, mShadowZ{ pShadowZ }
	, mLod{}
	, mTexture{ MakeTexture(), MakeTexture() }
	, mEffect{ MakeEffect() }
	, mVertexDeclaration{ MakeVertexDeclaration() }
	, mCamPos{ 0.0f, 0.0f, 0.0f }
	, mCamDir{ 0.0f, 1.0f, 0.0f }
	, mHeight{ nullptr }
	, mAngle{ nullptr }
{
}

bool Rock::init(std::function<float(float, float)> height, std::function<float(float, float)> angle)
{
	mHeight = height;
	mAngle = angle;

	if (!LoadTbnObject(mDevice, "res\\rock\\rock_lod0.obj", mLod[0].mVertexBuffer, mLod[0].mIndexBuffer, mLod[0].mIndexCount, mLod[0].mSphere))
		return false;

	if (!LoadTbnObject(mDevice, "res\\rock\\rock_lod1.obj", mLod[1].mVertexBuffer, mLod[1].mIndexBuffer, mLod[1].mIndexCount, mLod[1].mSphere))
		return false;

	if (!LoadTbnObject(mDevice, "res\\rock\\rock_lod2.obj", mLod[2].mVertexBuffer, mLod[2].mIndexBuffer, mLod[2].mIndexCount, mLod[2].mSphere))
		return false;

	Instance* instance_buffer = new Instance[maxInstanceCount];
	mLod[0].mInstanceBuffer.reset(LoadVertexBuffer(mDevice, instance_buffer, sizeof(Instance), maxInstanceCount, 0));
	mLod[1].mInstanceBuffer.reset(LoadVertexBuffer(mDevice, instance_buffer, sizeof(Instance), maxInstanceCount, 0));
	mLod[2].mInstanceBuffer.reset(LoadVertexBuffer(mDevice, instance_buffer, sizeof(Instance), maxInstanceCount, 0));
	delete[] instance_buffer;
	if (!mLod[0].mInstanceBuffer || !mLod[1].mInstanceBuffer || !mLod[2].mInstanceBuffer)
		return false;

	createInstances();

	mVertexDeclaration.reset(LoadVertexDeclaration(mDevice, vertexElement));
	if (!mVertexDeclaration)
		return false;

	mTexture[0].reset(LoadTexture(mDevice, L"res\\rock\\results\\rock_lowpoly_diffuse_png_dxt1_1.dds"));
	mTexture[1].reset(LoadTexture(mDevice, L"res\\rock\\rock_lowpoly_normaldx.png"));
	if (!mTexture[0] || !mTexture[1])
		return false;

	mEffect.reset(LoadEffect(mDevice, L"rock.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTexture("TextureDiffuse", mTexture[0].get());
	mEffect->SetTexture("TextureDepthShadow", mShadowZ);
	mEffect->SetTexture("TextureNormal", mTexture[1].get());

	mEffect->SetInt("ShadowTexSize", gShadowTexSize);

	return true;
}

void Rock::update(const float /*tick*/)
{
	const D3DXVECTOR3 camPos = mCamera->getPos();
	float a = camPos.x - mCamPos.x;
	float b = camPos.z - mCamPos.z;
	float d = sqrtf(a * a + b * b);

	const D3DXVECTOR3 camDir = mCamera->getDir();
	float f = D3DXVec3Dot(&camDir, &mCamDir);

	if (d > 10 || f < 0.99f)
	{
		mCamPos = camPos;
		mCamDir = camDir;
		createInstances();
	}
}

void Rock::draw(RockRenderMode mode, const D3DXMATRIX& matLightViewProj)
{
	const D3DXVECTOR3 camPos = mCamera->getPos();

	if (mode == RockRenderMode::Refract)
		mEffect->SetTechnique("Refract");
	else if (mode == RockRenderMode::Reflect)
		mEffect->SetTechnique("Reflect");
	else if (mode == RockRenderMode::UnderwaterReflect)
		mEffect->SetTechnique("UnderwaterReflect");
	else if (mode == RockRenderMode::Caster)
		mEffect->SetTechnique("Caster");
	else
		mEffect->SetTechnique("Normal");

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

	mEffect->SetFloatArray("CameraPosition", (float*)&camPos, 3);

	mDevice->SetVertexDeclaration(mVertexDeclaration.get());

	for (int iLod = 0; iLod < 3; iLod++)
	{
		if (mode == RockRenderMode::Normal)
			mEffect->SetTechnique(lodFx[iLod]);

		mDevice->SetStreamSource(0, mLod[iLod].mVertexBuffer.get(), 0, sizeof(TbnVertex));
		mDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | mLod[iLod].mInstanceCount));

		mDevice->SetStreamSource(1, mLod[iLod].mInstanceBuffer.get(), 0, sizeof(Instance));
		mDevice->SetStreamSourceFreq(1, (D3DSTREAMSOURCE_INSTANCEDATA | 1ul));

		mDevice->SetIndices(mLod[iLod].mIndexBuffer.get());

		RenderEffect(mEffect.get(), [this, iLod]()
		{
			mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mLod[iLod].mIndexCount, 0, mLod[iLod].mIndexCount / 3);
		});
	}

	mDevice->SetStreamSourceFreq(0, 1);
	mDevice->SetStreamSourceFreq(1, 1);
	mDevice->SetStreamSource(1, nullptr, 0, 0);
}

void Rock::createInstances()
{
	Hash hash;
	hash.setseed(42);
	Random random;

	int placedCount[3] = { 0, 0, 0 };
	Instance* instance_buffer[3] = { new Instance[maxInstanceCount], new Instance[maxInstanceCount], new Instance[maxInstanceCount] };

	for (int j = 0; j < (66 * 3); j += 3)
	{
		for (int i = 0; i < (66 * 3); i += 3)
		{
			random.setseed(hash(i, j));
			unsigned int r = random() % 1000;

			if (r >= 975)
			{
				float x = (float)(i - (67 / 2));
				float z = (float)(j - (67 / 2));

				float t = mAngle(x, z);
				if (t < 0.35f)
					continue;

				float y = mHeight(x, z) - 0.5f;
				if (y < -2)
					continue;

				float a = x - mCamPos.x;
				float b = z - mCamPos.z;
				float d = sqrtf(a * a + b * b);
				int iLod = (d < 30) ? 0 : (d < 60) ? 1 : 2;

				D3DXMATRIX matTrans;
				D3DXMatrixTranslation(&matTrans, x, y, z);

				D3DXMATRIX matScale;
				float s = 0.01f + (random() % 10) * 0.005f;
				D3DXMatrixScaling(&matScale, s, s, s);

				float radius = mLod[iLod].mSphere.w * s;
				D3DXVECTOR3 center(mLod[iLod].mSphere.x * s + x, mLod[iLod].mSphere.y * s + y, mLod[iLod].mSphere.z * s + z);
				if (!mCamera->isSphereInFrustum(center, radius))
					continue;

				D3DXMATRIX matRotZ, matRotY, matRotX;
				D3DXMatrixRotationZ(&matRotZ, D3DXToRadian(random() % 360));
				D3DXMatrixRotationY(&matRotY, D3DXToRadian(random() % 360));
				D3DXMatrixRotationX(&matRotX, D3DXToRadian(random() % 360));

				D3DXMATRIX matWorld = matRotZ * matRotY * matRotX * matScale * matTrans;
				D3DXMatrixTranspose(&matWorld, &matWorld);
				for (int n = 0; n < 4; n++)
				{
					instance_buffer[iLod][placedCount[iLod]].m0[n] = matWorld.m[0][n];
					instance_buffer[iLod][placedCount[iLod]].m1[n] = matWorld.m[1][n];
					instance_buffer[iLod][placedCount[iLod]].m2[n] = matWorld.m[2][n];
					instance_buffer[iLod][placedCount[iLod]].m3[n] = matWorld.m[3][n];
				}

				placedCount[iLod]++;
			}

			if (placedCount[0] + placedCount[1] + placedCount[2] >= maxInstanceCount)
				break;
		}

		if (placedCount[0] + placedCount[1] + placedCount[2] >= maxInstanceCount)
			break;
	}

	for (int i = 0; i < 3; i++)
	{
		void* pData{};
		IDirect3DVertexBuffer9* pVertexBuffer = mLod[i].mInstanceBuffer.get();
		if (SUCCEEDED(pVertexBuffer->Lock(0, 0, &pData, 0)))
		{
			mLod[i].mInstanceCount = placedCount[i];
			memcpy(pData, instance_buffer[i], mLod[i].mInstanceCount * sizeof(Instance));
			pVertexBuffer->Unlock();
		}

		delete[] instance_buffer[i];
	}
}
