#include "rock.h"
#include "random.h"
#include "wavefront.h"
#include "constants.h"
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

	constexpr int MAX_INSTANCE_COUNT = 50;

	const char* const LOD_FX[3] =
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
	, mTexture{ makeTexture(), makeTexture() }
	, mEffect{ makeEffect() }
	, mVertexDeclaration{ makeVertexDeclaration() }
	, mCamPos{ 0.0f, 0.0f, 0.0f }
	, mCamDir{ 0.0f, 1.0f, 0.0f }
	, mHeight{ nullptr }
	, mAngle{ nullptr }
{
}

bool Rock::init(const std::function<float(float, float)>& height, const std::function<float(float, float)>& angle)
{
	mHeight = height;
	mAngle = angle;

	if (!loadTbnObject(mDevice, "res\\rock\\rock_lod0.obj", mLod[0].mVertexBuffer, mLod[0].mIndexBuffer, mLod[0].mIndexCount, mLod[0].mSphere))
		return false;

	if (!loadTbnObject(mDevice, "res\\rock\\rock_lod1.obj", mLod[1].mVertexBuffer, mLod[1].mIndexBuffer, mLod[1].mIndexCount, mLod[1].mSphere))
		return false;

	if (!loadTbnObject(mDevice, "res\\rock\\rock_lod2.obj", mLod[2].mVertexBuffer, mLod[2].mIndexBuffer, mLod[2].mIndexCount, mLod[2].mSphere))
		return false;

	Instance instanceBuffer[MAX_INSTANCE_COUNT];
	mLod[0].mInstanceBuffer.reset(loadVertexBuffer(mDevice, instanceBuffer, sizeof(Instance), MAX_INSTANCE_COUNT, 0));
	mLod[1].mInstanceBuffer.reset(loadVertexBuffer(mDevice, instanceBuffer, sizeof(Instance), MAX_INSTANCE_COUNT, 0));
	mLod[2].mInstanceBuffer.reset(loadVertexBuffer(mDevice, instanceBuffer, sizeof(Instance), MAX_INSTANCE_COUNT, 0));
	if (!mLod[0].mInstanceBuffer || !mLod[1].mInstanceBuffer || !mLod[2].mInstanceBuffer)
		return false;

	createInstances();

	mVertexDeclaration.reset(loadVertexDeclaration(mDevice, VERTEX_ELEMENT));
	if (!mVertexDeclaration)
		return false;

	mTexture[0].reset(loadTexture(mDevice, L"res\\rock\\results\\rock_lowpoly_diffuse_png_dxt1_1.dds"));
	mTexture[1].reset(loadTexture(mDevice, L"res\\rock\\rock_lowpoly_normaldx.png"));
	if (!mTexture[0] || !mTexture[1])
		return false;

	mEffect.reset(loadEffect(mDevice, L"rock.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTexture("TextureDiffuse", mTexture[0].get());
	mEffect->SetTexture("TextureDepthShadow", mShadowZ);
	mEffect->SetTexture("TextureNormal", mTexture[1].get());

	mEffect->SetInt("ShadowTexSize", SHADOW_TEX_SIZE);

	return true;
}

void Rock::update(const float /*tick*/)
{
	const D3DXVECTOR3 camPos = mCamera->getPos();
	const float a = camPos.x - mCamPos.x;
	const float b = camPos.z - mCamPos.z;
	const float d = sqrtf(a * a + b * b);

	const D3DXVECTOR3 camDir = mCamera->getDir();
	const float f = D3DXVec3Dot(&camDir, &mCamDir);

	if (d > 10 || f < 0.99f)
	{
		mCamPos = camPos;
		mCamDir = camDir;
		createInstances();
	}
}

void Rock::draw(const RockRenderMode mode, const D3DXMATRIX& matLightViewProj) const
{
	const D3DXVECTOR3 camPos = mCamera->getPos();

	if (mode == RockRenderMode::REFRACT)
		mEffect->SetTechnique("Refract");
	else if (mode == RockRenderMode::REFLECT)
		mEffect->SetTechnique("Reflect");
	else if (mode == RockRenderMode::UNDERWATER_REFLECT)
		mEffect->SetTechnique("UnderwaterReflect");
	else if (mode == RockRenderMode::CASTER)
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

	mEffect->SetFloatArray("CameraPosition", reinterpret_cast<const float*>(&camPos), 3);

	mDevice->SetVertexDeclaration(mVertexDeclaration.get());

	for (int iLod = 0; iLod < 3; iLod++)
	{
		if (mode == RockRenderMode::NORMAL)
			mEffect->SetTechnique(LOD_FX[iLod]);

		mDevice->SetStreamSource(0, mLod[iLod].mVertexBuffer.get(), 0, sizeof(TbnVertex));
		mDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | mLod[iLod].mInstanceCount));

		mDevice->SetStreamSource(1, mLod[iLod].mInstanceBuffer.get(), 0, sizeof(Instance));
		mDevice->SetStreamSourceFreq(1, (D3DSTREAMSOURCE_INSTANCEDATA | 1ul));

		mDevice->SetIndices(mLod[iLod].mIndexBuffer.get());

		renderEffect(mEffect.get(), [this, iLod]()
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
	Instance* instanceBuffer[3] = { new Instance[MAX_INSTANCE_COUNT], new Instance[MAX_INSTANCE_COUNT], new Instance[MAX_INSTANCE_COUNT] };

	for (int j = 0; j < (66 * 3); j += 3)
	{
		for (int i = 0; i < (66 * 3); i += 3)
		{
			random.setseed(hash(i, j));
			const unsigned int r = random() % 1000;

			if (r >= 975)
			{
				const float x = (float)(i - (67 / 2));
				const float z = (float)(j - (67 / 2));

				const float t = mAngle(x, z);
				if (t < 0.35f)
					continue;

				const float y = mHeight(x, z) - 0.5f;
				if (y < -2)
					continue;

				const float a = x - mCamPos.x;
				const float b = z - mCamPos.z;
				const float d = sqrtf(a * a + b * b);
				const int iLod = (d < 30) ? 0 : (d < 60) ? 1 : 2;

				D3DXMATRIX matTrans;
				D3DXMatrixTranslation(&matTrans, x, y, z);

				D3DXMATRIX matScale;
				const float s = 0.01f + (float)(random() % 10) * 0.005f;
				D3DXMatrixScaling(&matScale, s, s, s);

				const float radius = mLod[iLod].mSphere.w * s;
				const D3DXVECTOR3 center(mLod[iLod].mSphere.x * s + x, mLod[iLod].mSphere.y * s + y, mLod[iLod].mSphere.z * s + z);
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
					instanceBuffer[iLod][placedCount[iLod]].m0[n] = matWorld.m[0][n];
					instanceBuffer[iLod][placedCount[iLod]].m1[n] = matWorld.m[1][n];
					instanceBuffer[iLod][placedCount[iLod]].m2[n] = matWorld.m[2][n];
					instanceBuffer[iLod][placedCount[iLod]].m3[n] = matWorld.m[3][n];
				}

				placedCount[iLod]++;
			}

			if (placedCount[0] + placedCount[1] + placedCount[2] >= MAX_INSTANCE_COUNT)
				break;
		}

		if (placedCount[0] + placedCount[1] + placedCount[2] >= MAX_INSTANCE_COUNT)
			break;
	}

	for (int i = 0; i < 3; i++)
	{
		void* pData{};
		IDirect3DVertexBuffer9* pVertexBuffer = mLod[i].mInstanceBuffer.get();
		if (SUCCEEDED(pVertexBuffer->Lock(0, 0, &pData, 0)))
		{
			mLod[i].mInstanceCount = placedCount[i];
			memcpy(pData, instanceBuffer[i], mLod[i].mInstanceCount * sizeof(Instance));
			pVertexBuffer->Unlock();
		}

		delete[] instanceBuffer[i];
	}
}
