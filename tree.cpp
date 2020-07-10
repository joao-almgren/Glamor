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

	struct Vertex
	{
		D3DXVECTOR3 position;
		D3DXVECTOR3 normal;
		D3DXVECTOR3 tangent;
		D3DXVECTOR3 bitangent;
		D3DXVECTOR2 texcoord;
	};

	struct Instance
	{
		D3DXVECTOR4 m0;
		D3DXVECTOR4 m1;
		D3DXVECTOR4 m2;
		D3DXVECTOR4 m3;
	};

	constexpr int maxInstanceCount = 30;
	Instance instance[maxInstanceCount];
}

//*********************************************************************************************************************

Tree::Tree(IDirect3DDevice9* pDevice, IDirect3DTexture9* pShadowZ)
	: mDevice{ pDevice }
	, mShadowZ{ pShadowZ }
	, mLod{}
	, mTexture{ MakeTexture(), MakeTexture(), MakeTexture() }
	, mEffect{ MakeEffect() }
	, mVertexDeclaration{ MakeVertexDeclaration() }
{
}

//*********************************************************************************************************************

bool Tree::init(std::function<float(float, float)> height, std::function<float(float, float)> angle)
{
	if (!loadObject("tree\\tree1a_trunk_lod0.obj", mLod[0].mVertexBuffer[0], mLod[0].mIndexBuffer[0], mLod[0].mIndexCount[0]))
		return false;

	if (!loadObject("tree\\tree1a_trunk_lod1.obj", mLod[1].mVertexBuffer[0], mLod[1].mIndexBuffer[0], mLod[1].mIndexCount[0]))
		return false;

	if (!loadObject("tree\\tree1a_trunk_lod2.obj", mLod[2].mVertexBuffer[0], mLod[2].mIndexBuffer[0], mLod[2].mIndexCount[0]))
		return false;

	if (!loadObject("tree\\tree1a_leaves_lod0.obj", mLod[0].mVertexBuffer[1], mLod[0].mIndexBuffer[1], mLod[0].mIndexCount[1]))
		return false;

	if (!loadObject("tree\\tree1a_leaves_lod1.obj", mLod[1].mVertexBuffer[1], mLod[1].mIndexBuffer[1], mLod[1].mIndexCount[1]))
		return false;

	if (!loadObject("tree\\tree1a_leaves_lod2.obj", mLod[2].mVertexBuffer[1], mLod[2].mIndexBuffer[1], mLod[2].mIndexCount[1]))
		return false;

	if (!createInstances(height, angle))
		return false;

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

void Tree::update(const float /*tick*/)
{
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

		if (mode == TreeRenderMode::Plain)
		{
			mDevice->SetStreamSource(0, mLod[iLod].mVertexBuffer[0].get(), 0, sizeof(Vertex));
			mDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | mLod[iLod].mInstanceCount));

			mDevice->SetIndices(mLod[iLod].mIndexBuffer[0].get());

			mEffect->SetTexture("Texture0", mTexture[0].get());
			mEffect->SetTechnique("Trunk");

			RenderEffect(mEffect.get(), [this, iLod]()
			{
				mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mLod[iLod].mIndexCount[0], 0, mLod[iLod].mIndexCount[0] / 3);
			});
		}

		mDevice->SetStreamSource(0, mLod[iLod].mVertexBuffer[1].get(), 0, sizeof(Vertex));
		mDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | mLod[iLod].mInstanceCount));

		mDevice->SetIndices(mLod[iLod].mIndexBuffer[1].get());

		mEffect->SetTexture("Texture0", mTexture[1].get());
		mEffect->SetTechnique((mode == TreeRenderMode::Plain) ? "PlainLeaves" : "BlendLeaves");

		RenderEffect(mEffect.get(), [this, iLod]()
		{
			mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mLod[iLod].mIndexCount[1], 0, mLod[iLod].mIndexCount[1] / 3);
		});
	}

	mDevice->SetStreamSourceFreq(0, 1);
	mDevice->SetStreamSourceFreq(1, 1);
	mDevice->SetStreamSource(1, nullptr, 0, 0);
}

//*********************************************************************************************************************

void CalculateTangents(Vertex& a, Vertex& b, Vertex& c)
{
	D3DXVECTOR3 v = b.position - a.position, w = c.position - a.position;
	float sx = b.texcoord.x - a.texcoord.x, sy = b.texcoord.y - a.texcoord.y;
	float tx = c.texcoord.x - a.texcoord.x, ty = c.texcoord.y - a.texcoord.y;

	float dirCorrection = (tx * sy - ty * sx) < 0.0f ? -1.0f : 1.0f;

	if (sx * ty == sy * tx)
	{
		sx = 0.0;
		sy = 1.0;
		tx = 1.0;
		ty = 0.0;
	}

	D3DXVECTOR3 tangent, bitangent;
	tangent.x = (w.x * sy - v.x * ty) * dirCorrection;
	tangent.y = (w.y * sy - v.y * ty) * dirCorrection;
	tangent.z = (w.z * sy - v.z * ty) * dirCorrection;
	bitangent.x = (w.x * sx - v.x * tx) * dirCorrection;
	bitangent.y = (w.y * sx - v.y * tx) * dirCorrection;
	bitangent.z = (w.z * sx - v.z * tx) * dirCorrection;

	D3DXVECTOR3 localTangent = tangent - a.normal * D3DXVec3Dot(&tangent, &a.normal);
	D3DXVECTOR3 localBitangent = bitangent - a.normal * D3DXVec3Dot(&bitangent, &a.normal);

	D3DXVec3Normalize(&localTangent, &localTangent);
	D3DXVec3Normalize(&localBitangent, &localBitangent);

	a.tangent = localTangent;
	a.bitangent = localBitangent;

	localTangent = tangent - b.normal * D3DXVec3Dot(&tangent, &b.normal);
	localBitangent = bitangent - b.normal * D3DXVec3Dot(&bitangent, &b.normal);

	D3DXVec3Normalize(&localTangent, &localTangent);
	D3DXVec3Normalize(&localBitangent, &localBitangent);

	b.tangent = localTangent;
	b.bitangent = localBitangent;

	localTangent = tangent - c.normal * D3DXVec3Dot(&tangent, &c.normal);
	localBitangent = bitangent - c.normal * D3DXVec3Dot(&bitangent, &c.normal);

	D3DXVec3Normalize(&localTangent, &localTangent);
	D3DXVec3Normalize(&localBitangent, &localBitangent);

	c.tangent = localTangent;
	c.bitangent = localBitangent;
}

//*********************************************************************************************************************

bool Tree::loadObject(std::string filename, VertexBuffer& vertexbuffer, IndexBuffer& indexbuffer, int& indexCount)
{
	std::vector<WFOVertex> vertex;
	std::vector<short> index;

	if (!LoadWFObject(filename, vertex, index))
		return false;

	int vertexCount = static_cast<int>(vertex.size());
	Vertex* vertex_buffer = new Vertex[vertexCount];
	for (int i = 0; i < vertexCount; i++)
		vertex_buffer[i] =
		{
			.position = vertex[i].p,
			.normal = vertex[i].n,
			.tangent = { 0, 0, 0 },
			.bitangent = { 0, 0, 0 },
			.texcoord = vertex[i].t,
		};

	indexCount = static_cast<int>(index.size());
	short* index_buffer = new short[indexCount];
	for (int i = 0; i < indexCount; i++)
		index_buffer[i] = index[i];

	for (int i = 0; i < indexCount; i += 3)
	{
		Vertex& a = vertex_buffer[index_buffer[i]];
		Vertex& b = vertex_buffer[index_buffer[i + 1]];
		Vertex& c = vertex_buffer[index_buffer[i + 2]];

		CalculateTangents(a, b, c);
	}

	vertexbuffer.reset(LoadVertexBuffer(mDevice, vertex_buffer, sizeof(Vertex), vertexCount, 0));
	delete[] vertex_buffer;

	indexbuffer.reset(LoadIndexBuffer(mDevice, index_buffer, indexCount));
	delete[] index_buffer;

	if (!vertexbuffer || !indexbuffer)
		return false;

	return true;
}

//*********************************************************************************************************************

bool Tree::createInstances(std::function<float(float, float)> height, std::function<float(float, float)> angle)
{
	Hash hash;
	hash.setseed(1337);
	Random random;

	int placedCount = 0;

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

				float a = angle(x, z);
				if (a < 0.75f)
					continue;

				float y = height(x, z) - 0.15f;
				if (y < 0)
					continue;

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
					instance[placedCount].m0[n] = matWorld.m[0][n];
					instance[placedCount].m1[n] = matWorld.m[1][n];
					instance[placedCount].m2[n] = matWorld.m[2][n];
					instance[placedCount].m3[n] = matWorld.m[3][n];
				}

				placedCount++;
			}

			if (placedCount >= maxInstanceCount)
				break;
		}

		if (placedCount >= maxInstanceCount)
			break;
	}

	if (placedCount != maxInstanceCount)
		return false;

	mLod[0].mInstanceCount = maxInstanceCount / 3;
	mLod[0].mInstanceBuffer.reset(LoadVertexBuffer(mDevice, instance, sizeof(Instance), mLod[0].mInstanceCount, 0));
	if (!mLod[0].mInstanceBuffer)
		return false;

	mLod[1].mInstanceCount = maxInstanceCount / 3;
	mLod[1].mInstanceBuffer.reset(LoadVertexBuffer(mDevice, &instance[1 * maxInstanceCount / 3], sizeof(Instance), mLod[0].mInstanceCount, 0));
	if (!mLod[1].mInstanceBuffer)
		return false;

	mLod[2].mInstanceCount = maxInstanceCount / 3;
	mLod[2].mInstanceBuffer.reset(LoadVertexBuffer(mDevice, &instance[2 * maxInstanceCount / 3], sizeof(Instance), mLod[0].mInstanceCount, 0));
	if (!mLod[2].mInstanceBuffer)
		return false;

	return true;
}

//*********************************************************************************************************************
