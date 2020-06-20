#include "grass.h"
#include "array.h"
#include "random.h"
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

//*********************************************************************************************************************

namespace
{
	const D3DVERTEXELEMENT9 vertexElement[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 3 * 4, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 1, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
		{ 1, 4 * 4, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
		{ 1, 8 * 4, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3 },
		{ 1, 12 * 4, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4 },
		D3DDECL_END()
	};

	struct Vertex
	{
		D3DXVECTOR3 p;
		D3DXVECTOR2 t;
	};

	bool operator==(const Vertex& a, const Vertex& b)
	{
		return (a.p == b.p && a.t == b.t);
	}

	struct Instance
	{
		D3DXVECTOR4 m0;
		D3DXVECTOR4 m1;
		D3DXVECTOR4 m2;
		D3DXVECTOR4 m3;
	};

	constexpr int maxInstanceCount = 1000;
	Instance instance[maxInstanceCount];
}

//*********************************************************************************************************************

Grass::Grass(IDirect3DDevice9* pDevice)
	: mDevice(pDevice)
	, mVertexBuffer(nullptr, vertexDeleter)
	, mIndexBuffer(nullptr, indexDeleter)
	, mInstanceBuffer(nullptr, vertexDeleter)
	, mTexture(nullptr, textureDeleter)
	, mEffect(nullptr, effectDeleter)
	, mVertexDeclaration(nullptr, declarationDeleter)
	, mIndexCount(0)
	, mPos{}
	, mPlacedCount(0)
{
}

//*********************************************************************************************************************

bool Grass::init(std::function<float(float, float)> height, std::function<float(float, float)> angle)
{
	mHeight = height;
	mAngle = angle;

	if (!loadObject())
		return false;

	mInstanceBuffer.reset(CreateVertexBuffer(mDevice, instance, sizeof(Instance), maxInstanceCount, 0));
	if (!mInstanceBuffer)
		return false;

	createInstances();

	mVertexDeclaration.reset(CreateDeclaration(mDevice, vertexElement));
	if (!mVertexDeclaration)
		return false;

	mTexture.reset(LoadTexture(mDevice, L"grass\\grass.png"));
	if (!mTexture)
		return false;

	mEffect.reset(CreateEffect(mDevice, L"grass.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTexture("Texture0", mTexture.get());

	return true;
}

//*********************************************************************************************************************

void Grass::update(D3DXVECTOR3 camPos, const float /*tick*/)
{
	float a = camPos.x - mPos.x;
	float b = camPos.z - mPos.z;
	float d = sqrtf(a * a + b * b);

	if (d > 8)
	{
		mPos = camPos;
		createInstances();
	}
}

//*********************************************************************************************************************

void Grass::draw(GrassRenderMode mode)
{
	if (mode == GrassRenderMode::Plain)
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

	mDevice->SetVertexDeclaration(mVertexDeclaration.get());

	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(Vertex));
	mDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | mPlacedCount));

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

bool Grass::loadObject()
{
	std::vector<D3DXVECTOR3> position;
	std::vector<D3DXVECTOR2> texcoord;
	Array<Vertex> vertex;
	std::vector<short> index;

	std::string line;
	std::ifstream fin(L"grass\\grass2.obj");
	while (std::getline(fin, line))
	{
		std::basic_stringstream stream(line);
		std::string type;
		stream >> type;
		if (type == "v")
		{
			D3DXVECTOR3 p;
			stream >> p.x >> p.y >> p.z;
			position.push_back(p);
		}
		else if (type == "vt")
		{
			D3DXVECTOR2 t;
			stream >> t.x >> t.y;
			t.y = 1 - t.y;
			texcoord.push_back(t);
		}
		else if (type == "f")
		{
			for (int i = 0; i < 3; i++)
			{
				char c, d;
				int p, t, n;
				stream >> p >> c >> t >> d >> n;
				if (c != '/' || d != '/')
					return false;
				if (p > position.size() || t > texcoord.size())
					return false;

				Vertex v;
				v.p = position[p - 1];
				v.t = texcoord[t - 1];
				short x = static_cast<short>(vertex.appendAbsent(v));
				index.push_back(x);
			}
		}
	}

	if (!vertex.size() || !index.size())
		return false;

	int vertexCount = static_cast<int>(vertex.size());
	Vertex* vertex_buffer = new Vertex[vertexCount];
	for (int i = 0; i < vertexCount; i++)
		vertex_buffer[i] = vertex[i];
	mVertexBuffer.reset(CreateVertexBuffer(mDevice, vertex_buffer, sizeof(Vertex), vertexCount, 0));
	delete[] vertex_buffer;
	if (!mVertexBuffer)
		return false;

	mIndexCount = static_cast<int>(index.size());
	short* index_buffer = new short[mIndexCount];
	for (int i = 0; i < mIndexCount; i++)
		index_buffer[i] = index[i];
	mIndexBuffer.reset(CreateIndexBuffer(mDevice, index_buffer, mIndexCount));
	delete[] index_buffer;
	if (!mIndexBuffer)
		return false;

	return true;
}

//*********************************************************************************************************************

void Grass::createInstances()
{
	Hash hash;
	hash.setseed(1);
	Random random;

	mPlacedCount = 0;
	for (int j = 0; j < 65; j++)
	{
		for (int i = 0; i < 65; i++)
		{
			unsigned int s = (int)(mPos.x) + i;
			unsigned int t = (int)(mPos.z) + j;

			random.setseed(hash(s, t));
			unsigned int r = random() % 100;

			if (r > 85)
			{
				float x = (float)((int)(mPos.x) + (i - 32) + (random() % 10) * 0.01f);
				float z = (float)((int)(mPos.z) + (j - 32) + (random() % 10) * 0.01f);

				float y = mHeight(x, z) - 0.15f;
				if (y < 1)
					continue;

				float a = mAngle(x, z);
				if (a < 0.5f)
					continue;

				D3DXMATRIX matTrans;
				D3DXMatrixTranslation(&matTrans, x, y, z);

				D3DXMATRIX matScale;
				float c = 0.1f + (random() % 5) * 0.1f;
				D3DXMatrixScaling(&matScale, c, c, c);

				D3DXMATRIX matRotY;
				D3DXMatrixRotationY(&matRotY, D3DXToRadian(random() % 360));

				D3DXMATRIX matWorld = matRotY * matScale * matTrans;
				D3DXMatrixTranspose(&matWorld, &matWorld);
				for (int n = 0; n < 4; n++)
				{
					instance[mPlacedCount].m0[n] = matWorld.m[0][n];
					instance[mPlacedCount].m1[n] = matWorld.m[1][n];
					instance[mPlacedCount].m2[n] = matWorld.m[2][n];
					instance[mPlacedCount].m3[n] = matWorld.m[3][n];
				}

				mPlacedCount++;
			}

			if (mPlacedCount >= maxInstanceCount)
				break;
		}

		if (mPlacedCount >= maxInstanceCount)
			break;
	}

	void* pData{};
	IDirect3DVertexBuffer9* pVertexBuffer = mInstanceBuffer.get();
	if (SUCCEEDED(pVertexBuffer->Lock(0, 0, &pData, 0)))
	{
		memcpy(pData, instance, mPlacedCount * sizeof(Instance));
		pVertexBuffer->Unlock();
	}
}

//*********************************************************************************************************************
