#include "rock.h"
#include "array.h"
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

	bool operator==(const Vertex& a, const Vertex& b)
	{
		return (a.p == b.p && a.n == b.n && a.t == b.t);
	}

	struct Instance
	{
		D3DCOLOR col;
		D3DXVECTOR4 m0;
		D3DXVECTOR4 m1;
		D3DXVECTOR4 m2;
		D3DXVECTOR4 m3;
	};

	constexpr int numInstances = 2;
	Instance instance[numInstances];
}

//*********************************************************************************************************************

Rock::Rock(IDirect3DDevice9* pDevice)
	: mDevice(pDevice)
	, mVertexBuffer(nullptr, vertexDeleter)
	, mIndexBuffer(nullptr, indexDeleter)
	, mInstanceBuffer(nullptr, vertexDeleter)
	, mTexture(nullptr, textureDeleter)
	, mEffect(nullptr, effectDeleter)
	, mVertexDeclaration(nullptr, declarationDeleter)
	, numVertices(0)
{
}

//*********************************************************************************************************************

bool Rock::init()
{
	if (!loadObject())
		return false;

	if (!createInstances())
		return false;

	mVertexDeclaration.reset(CreateDeclaration(mDevice, vertexElement));
	if (!mVertexDeclaration)
		return false;

	mTexture.reset(LoadTexture(mDevice, L"rock\\Rock_LowPoly_Diffuse.png"));
	if (!mTexture)
		return false;

	mEffect.reset(CreateEffect(mDevice, L"rock.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTechnique("Technique0");
	mEffect->SetTexture("Texture0", mTexture.get());

	return true;
}

//*********************************************************************************************************************

void Rock::update(const float /*tick*/)
{
}

//*********************************************************************************************************************

void Rock::draw()
{
	D3DXMATRIX matProjection;
	mDevice->GetTransform(D3DTS_PROJECTION, &matProjection);

	D3DXMATRIX matView;
	mDevice->GetTransform(D3DTS_VIEW, &matView);

	D3DXMATRIX viewProjection = matView * matProjection;
	D3DXMatrixTranspose(&viewProjection, &viewProjection);
	mEffect->SetMatrix("ViewProjection", &viewProjection);

	mDevice->SetVertexDeclaration(mVertexDeclaration.get());

	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(Vertex));
	mDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | numInstances));

	mDevice->SetStreamSource(1, mInstanceBuffer.get(), 0, sizeof(Instance));
	mDevice->SetStreamSourceFreq(1, (D3DSTREAMSOURCE_INSTANCEDATA | 1ul));

	mDevice->SetIndices(mIndexBuffer.get());

	RenderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numVertices, 0, numVertices * 3);
	});

	mDevice->SetStreamSourceFreq(0, 1);
	mDevice->SetStreamSourceFreq(1, 1);
}

//*********************************************************************************************************************

bool Rock::loadObject()
{
	std::vector<D3DXVECTOR3> position;
	std::vector<D3DXVECTOR3> normal;
	std::vector<D3DXVECTOR2> texcoord;
	Array<Vertex> vertex;
	std::vector<short> index;

	std::string line;
	std::ifstream fin(L"rock\\Rock.obj");
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
		else if (type == "vn")
		{
			D3DXVECTOR3 n;
			stream >> n.x >> n.y >> n.z;
			D3DXVec3Normalize(&n, &n);
			normal.push_back(n);
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
				if (p > position.size() || t > texcoord.size() || n > normal.size())
					return false;

				Vertex v;
				v.p = position[p - 1];
				v.n = normal[n - 1];
				v.t = texcoord[t - 1];
				short x = static_cast<short>(vertex.appendAbsent(v));
				index.push_back(x);
			}
		}
	}

	if (!vertex.size() || !index.size())
		return false;

	numVertices = static_cast<int>(vertex.size());
	Vertex* vertex_buffer = new Vertex[numVertices];
	for (int i = 0; i < numVertices; i++)
		vertex_buffer[i] = vertex[i];
	mVertexBuffer.reset(CreateVertexBuffer(mDevice, vertex_buffer, sizeof(Vertex), numVertices, 0));
	delete[] vertex_buffer;
	if (!mVertexBuffer)
		return false;

	short* index_buffer = new short[index.size()];
	for (int i = 0; i < index.size(); i++)
		index_buffer[i] = index[i];
	mIndexBuffer.reset(CreateIndexBuffer(mDevice, index_buffer, (unsigned int)index.size()));
	delete[] index_buffer;
	if (!mIndexBuffer)
		return false;

	return true;
}

//*********************************************************************************************************************

bool Rock::createInstances()
{
	instance[0].col = D3DCOLOR_XRGB(128, 128, 128);

	D3DXMATRIX matWorld , matTrans, matScale, matRot;
	D3DXMatrixScaling(&matScale, 0.03f, 0.03f, 0.03f);
	D3DXMatrixTranslation(&matTrans, -15, 5, 35);
	matWorld = matScale * matTrans;
	D3DXMatrixTranspose(&matWorld, &matWorld);

	for (int i = 0; i < 4; i++)
	{
		instance[0].m0[i] = matWorld.m[0][i];
		instance[0].m1[i] = matWorld.m[1][i];
		instance[0].m2[i] = matWorld.m[2][i];
		instance[0].m3[i] = matWorld.m[3][i];
	}

	instance[1].col = D3DCOLOR_XRGB(255, 255, 255);

	D3DXMatrixScaling(&matScale, 0.02f, 0.02f, 0.02f);
	D3DXMatrixRotationZ(&matRot, 1);
	D3DXMatrixTranslation(&matTrans, -10, 10, 35);
	matWorld = matScale * matRot * matTrans;
	D3DXMatrixTranspose(&matWorld, &matWorld);

	for (int i = 0; i < 4; i++)
	{
		instance[1].m0[i] = matWorld.m[0][i];
		instance[1].m1[i] = matWorld.m[1][i];
		instance[1].m2[i] = matWorld.m[2][i];
		instance[1].m3[i] = matWorld.m[3][i];
	}

	mInstanceBuffer.reset(CreateVertexBuffer(mDevice, instance, sizeof(Instance), numInstances, 0));
	if (!mInstanceBuffer)
		return false;

	return true;
}

//*********************************************************************************************************************
