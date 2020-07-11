#include "tree.h"
#include "random.h"
#include "wavefront.h"
#include "constants.h"

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

	const int maxInstanceCount = 30;

	enum { TRUNK, STENCIL, BLEND };
	const char* const lodFx[3][3] =
	{
		{ "Trunk", "StencilLeaves", "BlendLeaves" },
		{ "TrunkSimple", "StencilLeavesSimple", nullptr },
		{ "TrunkSimple", "StencilLeavesSimple", nullptr }
	};
}

//*********************************************************************************************************************

Tree::Tree(IDirect3DDevice9* pDevice, IDirect3DTexture9* pShadowZ)
	: mDevice{ pDevice }
	, mShadowZ{ pShadowZ }
	, mLod{}
	, mTexture{ MakeTexture(), MakeTexture(), MakeTexture() }
	, mEffect{ MakeEffect() }
	, mVertexDeclaration{ MakeVertexDeclaration() }
	, mCamPos{ 0.0f, 0.0f, 0.0f }
{
}

//*********************************************************************************************************************

bool Tree::init(std::function<float(float, float)> height, std::function<float(float, float)> angle)
{
	mHeight = height;
	mAngle = angle;

	if (!LoadTbnObject(mDevice, "tree\\tree1a_trunk_lod0.obj", mLod[0].mVertexBuffer[0], mLod[0].mIndexBuffer[0], mLod[0].mIndexCount[0]))
		return false;

	if (!LoadTbnObject(mDevice, "tree\\tree1a_trunk_lod1.obj", mLod[1].mVertexBuffer[0], mLod[1].mIndexBuffer[0], mLod[1].mIndexCount[0]))
		return false;

	if (!LoadTbnObject(mDevice, "tree\\tree1a_trunk_lod2.obj", mLod[2].mVertexBuffer[0], mLod[2].mIndexBuffer[0], mLod[2].mIndexCount[0]))
		return false;

	if (!LoadTbnObject(mDevice, "tree\\tree1a_leaves_lod0.obj", mLod[0].mVertexBuffer[1], mLod[0].mIndexBuffer[1], mLod[0].mIndexCount[1]))
		return false;

	if (!LoadTbnObject(mDevice, "tree\\tree1a_leaves_lod1.obj", mLod[1].mVertexBuffer[1], mLod[1].mIndexBuffer[1], mLod[1].mIndexCount[1]))
		return false;

	if (!LoadTbnObject(mDevice, "tree\\tree1a_leaves_lod2.obj", mLod[2].mVertexBuffer[1], mLod[2].mIndexBuffer[1], mLod[2].mIndexCount[1]))
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

	mTexture[0].reset(LoadTexture(mDevice, L"tree\\results\\tree1a_bark_tga_dxt1_1.dds"));
	mTexture[1].reset(LoadTexture(mDevice, L"tree\\results\\tree1a_leaves_tga_dxt5_1.dds"));
	mTexture[2].reset(LoadTexture(mDevice, L"tree\\tree1a_bark_normals.tga"));
	if (!mTexture[0] || !mTexture[1] || !mTexture[2])
		return false;

	mEffect.reset(LoadEffect(mDevice, L"tree.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTexture("Texture1", mShadowZ);
	mEffect->SetTexture("Texture2", mTexture[2].get());

	mEffect->SetInt("ShadowTexSize", gShadowTexSize);

	return true;
}

//*********************************************************************************************************************

void Tree::update(const D3DXVECTOR3& camPos, const float /*tick*/)
{
	float a = camPos.x - mCamPos.x;
	float b = camPos.z - mCamPos.z;
	float d = sqrtf(a * a + b * b);

	if (d > 10)
	{
		mCamPos = camPos;
		createInstances();
	}
}

//*********************************************************************************************************************

void Tree::draw(TreeRenderMode mode, const D3DXVECTOR3& camPos, const D3DXMATRIX& matLightViewProj)
{
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
		mDevice->SetStreamSource(1, mLod[iLod].mInstanceBuffer.get(), 0, sizeof(Instance));
		mDevice->SetStreamSourceFreq(1, (D3DSTREAMSOURCE_INSTANCEDATA | 1ul));

		if (mode == TreeRenderMode::Pass0)
		{
			mEffect->SetTechnique(lodFx[iLod][TRUNK]);
			mEffect->SetTexture("Texture0", mTexture[0].get());

			mDevice->SetStreamSource(0, mLod[iLod].mVertexBuffer[0].get(), 0, sizeof(TbnVertex));
			mDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | mLod[iLod].mInstanceCount));

			mDevice->SetIndices(mLod[iLod].mIndexBuffer[0].get());

			RenderEffect(mEffect.get(), [this, iLod]()
			{
				mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mLod[iLod].mIndexCount[0], 0, mLod[iLod].mIndexCount[0] / 3);
			});
		}

		const char* fx = (mode == TreeRenderMode::Pass0) ? lodFx[iLod][STENCIL] : lodFx[iLod][BLEND];
		if (fx)
		{
			mEffect->SetTechnique(fx);
			mEffect->SetTexture("Texture0", mTexture[1].get());
		
			mDevice->SetStreamSource(0, mLod[iLod].mVertexBuffer[1].get(), 0, sizeof(TbnVertex));
			mDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | mLod[iLod].mInstanceCount));

			mDevice->SetIndices(mLod[iLod].mIndexBuffer[1].get());

			RenderEffect(mEffect.get(), [this, iLod]()
			{
				mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mLod[iLod].mIndexCount[1], 0, mLod[iLod].mIndexCount[1] / 3);
			});
		}
	}

	mDevice->SetStreamSourceFreq(0, 1);
	mDevice->SetStreamSourceFreq(1, 1);
	mDevice->SetStreamSource(1, nullptr, 0, 0);
}

//*********************************************************************************************************************

void Tree::createInstances()
{
	Hash hash;
	hash.setseed(1337);
	Random random;

	int placedCount[3] = { 0, 0, 0 };
	Instance* instance_buffer[3] = { new Instance[maxInstanceCount], new Instance[maxInstanceCount], new Instance[maxInstanceCount] };

	for (int j = 0; j < (66 * 3); j += 5)
	{
		for (int i = 0; i < (66 * 3); i += 5)
		{
			random.setseed(hash(i, j));
			unsigned int r = random() % 100;

			if (r >= 75)
			{
				float x = (float)(i - (67 / 2));
				float z = (float)(j - (67 / 2));

				float t = mAngle(x, z);
				if (t < 0.75f)
					continue;

				float y = mHeight(x, z) - 0.15f;
				if (y < 0)
					continue;

				float a = x - mCamPos.x;
				float b = z - mCamPos.z;
				float d = sqrtf(a * a + b * b);
				int iLod = (d < 30) ? 0 : (d < 60) ? 1 : 2;

				D3DXMATRIX matTrans;
				D3DXMatrixTranslation(&matTrans, x, y, z);

				D3DXMATRIX matScale;
				float s = 0.5f + (random() % 10) * 0.05f;
				D3DXMatrixScaling(&matScale, s, s, s);

				D3DXMATRIX matRotY;
				D3DXMatrixRotationY(&matRotY, D3DXToRadian(random() % 360));

				D3DXMATRIX matWorld = matRotY * matScale * matTrans;
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

//*********************************************************************************************************************
