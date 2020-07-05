#include "rock.h"
#include "random.h"
#include "wavefront.h"
#include "constants.h"
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

	constexpr int maxInstanceCount = 25;
	Instance instance[maxInstanceCount];
}

//*********************************************************************************************************************

Rock::Rock(IDirect3DDevice9* pDevice, IDirect3DTexture9* pShadowZ)
	: mDevice{ pDevice }
	, mShadowZ{ pShadowZ }
	, mVertexBuffer{ nullptr, vertexDeleter }
	, mIndexBuffer{ nullptr, indexDeleter }
	, mInstanceBuffer{ nullptr, vertexDeleter }
	, mTexture{ { nullptr, textureDeleter }, { nullptr, textureDeleter } }
	, mEffect{ nullptr, effectDeleter }
	, mVertexDeclaration{ nullptr, declarationDeleter }
	, mIndexCount{ 0 }
{
}

//*********************************************************************************************************************

bool Rock::init(std::function<float(float, float)> height, std::function<float(float, float)> angle)
{
	if (!loadObject("rock\\Rock.obj", mVertexBuffer, mIndexBuffer))
		return false;

	if (!createInstances(height, angle))
		return false;

	mVertexDeclaration.reset(CreateDeclaration(mDevice, vertexElement));
	if (!mVertexDeclaration)
		return false;

	mTexture[0].reset(LoadTexture(mDevice, L"rock\\results\\rock_lowpoly_diffuse_png_dxt1_1.dds"));
	mTexture[1].reset(LoadTexture(mDevice, L"rock\\rock_lowpoly_normaldx.png"));
	if (!mTexture[0] || !mTexture[1])
		return false;

	mEffect.reset(CreateEffect(mDevice, L"rock.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTexture("Texture0", mTexture[0].get());
	mEffect->SetTexture("Texture1", mShadowZ);
	mEffect->SetTexture("Texture2", mTexture[1].get());

	mEffect->SetInt("ShadowTexSize", gShadowTexSize);

	return true;
}

//*********************************************************************************************************************

void Rock::update(const float /*tick*/)
{
}

//*********************************************************************************************************************

void Rock::draw(RockRenderMode mode, const D3DXVECTOR3& camPos, const D3DXMATRIX& matLightViewProj)
{
	if (mode == RockRenderMode::Refract)
		mEffect->SetTechnique("Refract");
	else if (mode == RockRenderMode::Reflect)
		mEffect->SetTechnique("Reflect");
	else if (mode == RockRenderMode::UnderwaterReflect)
		mEffect->SetTechnique("UnderwaterReflect");
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

	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(Vertex));
	mDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | maxInstanceCount));

	mDevice->SetStreamSource(1, mInstanceBuffer.get(), 0, sizeof(Instance));
	mDevice->SetStreamSourceFreq(1, (D3DSTREAMSOURCE_INSTANCEDATA | 1ul));

	mDevice->SetIndices(mIndexBuffer.get());

	RenderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mIndexCount, 0, mIndexCount / 3);
	});

	mDevice->SetStreamSourceFreq(0, 1);
	mDevice->SetStreamSourceFreq(1, 1);
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

bool Rock::loadObject(std::string filename, VertexBuffer& vertexbuffer, IndexBuffer& indexbuffer)
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

	mIndexCount = static_cast<int>(index.size());
	short* index_buffer = new short[mIndexCount];
	for (int i = 0; i < mIndexCount; i++)
		index_buffer[i] = index[i];

	for (int i = 0; i < mIndexCount; i += 3)
	{
		Vertex& a = vertex_buffer[index_buffer[i]];
		Vertex& b = vertex_buffer[index_buffer[i + 1]];
		Vertex& c = vertex_buffer[index_buffer[i + 2]];

		CalculateTangents(a, b, c);
	}

	vertexbuffer.reset(CreateVertexBuffer(mDevice, vertex_buffer, sizeof(Vertex), vertexCount, 0));
	delete[] vertex_buffer;

	indexbuffer.reset(CreateIndexBuffer(mDevice, index_buffer, mIndexCount));
	delete[] index_buffer;

	if (!vertexbuffer || !indexbuffer)
		return false;

	return true;
}

//*********************************************************************************************************************

bool Rock::createInstances(std::function<float(float, float)> height, std::function<float(float, float)> angle)
{
	Hash hash;
	hash.setseed(42);
	Random random;

	int placedCount = 0;

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

				float a = angle(x, z);
				if (a < 0.35f)
					continue;

				float y = height(x, z) - 0.5f;
				if (y < -2)
					continue;

				D3DXMATRIX matTrans;
				D3DXMatrixTranslation(&matTrans, x, y, z);

				D3DXMATRIX matScale;
				float s = 0.01f + (random() % 10) * 0.005f;
				D3DXMatrixScaling(&matScale, s, s, s);

				D3DXMATRIX matRotZ, matRotY, matRotX;
				D3DXMatrixRotationZ(&matRotZ, D3DXToRadian(random() % 360));
				D3DXMatrixRotationY(&matRotY, D3DXToRadian(random() % 360));
				D3DXMatrixRotationX(&matRotX, D3DXToRadian(random() % 360));

				D3DXMATRIX matWorld = matRotZ * matRotY * matRotX * matScale * matTrans;
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

	mInstanceBuffer.reset(CreateVertexBuffer(mDevice, instance, sizeof(Instance), maxInstanceCount, 0));
	if (!mInstanceBuffer)
		return false;

	return true;
}

//*********************************************************************************************************************
