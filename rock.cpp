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
		{ 0, 6 * 4, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 1, 0, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		{ 1, 4, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
		{ 1, 5 * 4, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
		{ 1, 9 * 4, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3 },
		{ 1, 13 * 4, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4 },
		D3DDECL_END()
	};

	struct Vertex
	{
		D3DXVECTOR3 p;
		D3DXVECTOR3 n;
		D3DXVECTOR2 t;
	};

	struct Instance
	{
		D3DCOLOR col{};
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
	, mTexture{ nullptr, textureDeleter }
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

	mTexture.reset(LoadTexture(mDevice, L"rock\\results\\rock_lowpoly_diffuse_png_dxt1_1.dds"));
	if (!mTexture)
		return false;

	mEffect.reset(CreateEffect(mDevice, L"rock.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTexture("Texture0", mTexture.get());
	mEffect->SetTexture("Texture1", mShadowZ);

	mEffect->SetInt("ShadowTexSize", gShadowTexSize);

	return true;
}

//*********************************************************************************************************************

void Rock::update(const float /*tick*/)
{
}

//*********************************************************************************************************************

void Rock::draw(RockRenderMode mode, const D3DXMATRIX& matLightViewProj)
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
			.p = vertex[i].p,
			.n = vertex[i].n,
			.t = vertex[i].t,
		};
	vertexbuffer.reset(CreateVertexBuffer(mDevice, vertex_buffer, sizeof(Vertex), vertexCount, 0));
	delete[] vertex_buffer;
	if (!vertexbuffer)
		return false;

	mIndexCount = static_cast<int>(index.size());
	short* index_buffer = new short[mIndexCount];
	for (int i = 0; i < mIndexCount; i++)
		index_buffer[i] = index[i];
	indexbuffer.reset(CreateIndexBuffer(mDevice, index_buffer, mIndexCount));
	delete[] index_buffer;
	if (!indexbuffer)
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

				int luminance = ((y > -1) ? 112 : 80) + rand() % 48;
				instance[placedCount].col = D3DCOLOR_XRGB(luminance, luminance, luminance);

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
