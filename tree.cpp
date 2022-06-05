#include "tree.h"
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

	enum { TRUNK, STENCIL, BLEND };
	const char* const LOD_FX[3][3] =
	{
		{ "Trunk", "StencilLeaves", "BlendLeaves" },
		{ "TrunkSimple", "StencilLeavesSimple", nullptr },
		{ "TrunkSimple", "StencilLeavesSimple", nullptr }
	};
}

Tree::Tree(IDirect3DDevice9* pDevice, Camera* pCamera, IDirect3DTexture9* pShadowZ)
	: mDevice{ pDevice }
	, mCamera{ pCamera }
	, mShadowZ{ pShadowZ }
	, mLod{}
	, mTexture{ makeTexture(), makeTexture(), makeTexture() }
	, mEffect{ makeEffect() }
	, mVertexDeclaration{ makeVertexDeclaration() }
	, mCamPos{ 0.0f, 0.0f, 0.0f }
	, mCamDir{ 0.0f, 1.0f, 0.0f }
	, mHeight{ nullptr }
	, mAngle{ nullptr }
{
}

bool Tree::init(const std::function<float(float, float)>& height, const std::function<float(float, float)>& angle)
{
	mHeight = height;
	mAngle = angle;

	if (!loadTbnObject(mDevice, "res\\tree\\tree1a_trunk_lod0.obj", mLod[0].mVertexBuffer[0], mLod[0].mIndexBuffer[0], mLod[0].mIndexCount[0], mLod[0].mSphere[0]))
		return false;

	if (!loadTbnObject(mDevice, "res\\tree\\tree1a_trunk_lod1.obj", mLod[1].mVertexBuffer[0], mLod[1].mIndexBuffer[0], mLod[1].mIndexCount[0], mLod[1].mSphere[0]))
		return false;

	if (!loadTbnObject(mDevice, "res\\tree\\tree1a_trunk_lod2.obj", mLod[2].mVertexBuffer[0], mLod[2].mIndexBuffer[0], mLod[2].mIndexCount[0], mLod[2].mSphere[0]))
		return false;

	if (!loadTbnObject(mDevice, "res\\tree\\tree1a_leaves_lod0.obj", mLod[0].mVertexBuffer[1], mLod[0].mIndexBuffer[1], mLod[0].mIndexCount[1], mLod[0].mSphere[1]))
		return false;

	if (!loadTbnObject(mDevice, "res\\tree\\tree1a_leaves_lod1.obj", mLod[1].mVertexBuffer[1], mLod[1].mIndexBuffer[1], mLod[1].mIndexCount[1], mLod[1].mSphere[1]))
		return false;

	if (!loadTbnObject(mDevice, "res\\tree\\tree1a_leaves_lod2.obj", mLod[2].mVertexBuffer[1], mLod[2].mIndexBuffer[1], mLod[2].mIndexCount[1], mLod[2].mSphere[1]))
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

	mTexture[0].reset(loadTexture(mDevice, L"res\\tree\\results\\tree1a_bark_tga_dxt1_1.dds"));
	mTexture[1].reset(loadTexture(mDevice, L"res\\tree\\results\\tree1a_leaves_tga_dxt5_1.dds"));
	mTexture[2].reset(loadTexture(mDevice, L"res\\tree\\tree1a_bark_normals.tga"));
	if (!mTexture[0] || !mTexture[1] || !mTexture[2])
		return false;

	mEffect.reset(loadEffect(mDevice, L"tree.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTexture("TextureDepthShadow", mShadowZ);
	mEffect->SetTexture("TextureNormal", mTexture[2].get());

	mEffect->SetInt("ShadowTexSize", SHADOW_TEX_SIZE);

	return true;
}

void Tree::update(const float tick)
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

void Tree::draw(const TreeRenderMode mode, const D3DXMATRIX& matLightViewProj) const
{
	const D3DXVECTOR3 camPos = mCamera->getPos();

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
		mDevice->SetStreamSource(1, mLod[iLod].mInstanceBuffer.get(), 0, sizeof(Instance));
		mDevice->SetStreamSourceFreq(1, (D3DSTREAMSOURCE_INSTANCEDATA | 1ul));

		if (mode == TreeRenderMode::ALPHA_CLIP || mode == TreeRenderMode::CASTER)
		{
			const char* fx = (mode == TreeRenderMode::ALPHA_CLIP) ? LOD_FX[iLod][TRUNK] : "TrunkCaster";
			mEffect->SetTechnique(fx);
			mEffect->SetTexture("TextureDiffuse", mTexture[0].get());

			mDevice->SetStreamSource(0, mLod[iLod].mVertexBuffer[0].get(), 0, sizeof(TbnVertex));
			mDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | mLod[iLod].mInstanceCount));

			mDevice->SetIndices(mLod[iLod].mIndexBuffer[0].get());

			renderEffect(mEffect.get(), [this, iLod]()
			{
				mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mLod[iLod].mIndexCount[0], 0, mLod[iLod].mIndexCount[0] / 3);
			});
		}

		const char* fx = (mode == TreeRenderMode::ALPHA_CLIP) ? LOD_FX[iLod][STENCIL] : (mode == TreeRenderMode::ALPHA_BLEND) ? LOD_FX[iLod][BLEND] : "StencilLeavesCaster";
		mEffect->SetTechnique(fx);
		mEffect->SetTexture("TextureDiffuse", mTexture[1].get());
		
		mDevice->SetStreamSource(0, mLod[iLod].mVertexBuffer[1].get(), 0, sizeof(TbnVertex));
		mDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | mLod[iLod].mInstanceCount));

		mDevice->SetIndices(mLod[iLod].mIndexBuffer[1].get());

		renderEffect(mEffect.get(), [this, iLod]()
		{
			mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mLod[iLod].mIndexCount[1], 0, mLod[iLod].mIndexCount[1] / 3);
		});
	}

	mDevice->SetStreamSourceFreq(0, 1);
	mDevice->SetStreamSourceFreq(1, 1);
	mDevice->SetStreamSource(1, nullptr, 0, 0);
}

void Tree::createInstances()
{
	Hash hash;
	hash.setseed(1337);
	Random random;

	int placedCount[3] = { 0, 0, 0 };
	Instance* instanceBuffer[3] = { new Instance[MAX_INSTANCE_COUNT], new Instance[MAX_INSTANCE_COUNT], new Instance[MAX_INSTANCE_COUNT] };

	for (int j = 0; j < (66 * 3); j += 5)
	{
		for (int i = 0; i < (66 * 3); i += 5)
		{
			random.setseed(hash(i, j));
			const unsigned int r = random() % 100;

			if (r >= 75)
			{
				const float x = (float)(i - (67 / 2));
				const float z = (float)(j - (67 / 2));

				const float t = mAngle(x, z);
				if (t < 0.75f)
					continue;

				const float y = mHeight(x, z) - 0.15f;
				if (y < 0)
					continue;

				const float a = x - mCamPos.x;
				const float b = z - mCamPos.z;
				const float d = sqrtf(a * a + b * b);
				int iLod = (d < 30) ? 0 : (d < 60) ? 1 : 2;

				D3DXMATRIX matTrans;
				D3DXMatrixTranslation(&matTrans, x, y, z);

				D3DXMATRIX matScale;
				const float s = 0.5f + (float)(random() % 10) * 0.05f;
				D3DXMatrixScaling(&matScale, s, s, s);

				const float radius0 = mLod[iLod].mSphere[0].w * s;
				const float radius1 = mLod[iLod].mSphere[1].w * s;
				const D3DXVECTOR3 center0(mLod[iLod].mSphere[0].x * s + x, mLod[iLod].mSphere[0].y * s + y, mLod[iLod].mSphere[0].z * s + z);
				const D3DXVECTOR3 center1(mLod[iLod].mSphere[1].x * s + x, mLod[iLod].mSphere[1].y * s + y, mLod[iLod].mSphere[1].z * s + z);
				if (!mCamera->isSphereInFrustum(center0, radius0) && !mCamera->isSphereInFrustum(center1, radius1))
					continue;

				D3DXMATRIX matRotY;
				D3DXMatrixRotationY(&matRotY, D3DXToRadian(random() % 360));

				D3DXMATRIX matWorld = matRotY * matScale * matTrans;
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
