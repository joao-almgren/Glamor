#include "tree.h"
#include "array.h"
#include "random.h"
#include "wavefront.h"

//*********************************************************************************************************************

namespace
{
	const D3DVERTEXELEMENT9 vertexElement[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 3 * 4, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 6 * 4, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 1, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
		{ 1, 4 * 4, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
		{ 1, 8 * 4, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3 },
		{ 1, 12 * 4, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4 },
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
		D3DXVECTOR4 m0;
		D3DXVECTOR4 m1;
		D3DXVECTOR4 m2;
		D3DXVECTOR4 m3;
	};

	constexpr int maxInstanceCount = 15;
	Instance instance[maxInstanceCount];
}

//*********************************************************************************************************************

Tree::Tree(IDirect3DDevice9* pDevice)
	: mDevice(pDevice)
	, mVertexBuffer{ { nullptr, vertexDeleter }, { nullptr, vertexDeleter } }
	, mIndexBuffer{ { nullptr, indexDeleter }, { nullptr, indexDeleter } }
	, mInstanceBuffer(nullptr, vertexDeleter)
	, mTexture{ { nullptr, textureDeleter }, { nullptr, textureDeleter } }
	, mEffect(nullptr, effectDeleter)
	, mVertexDeclaration(nullptr, declarationDeleter)
	, mIndexCount(0)
{
}

//*********************************************************************************************************************

bool Tree::init(std::function<float(float, float)> height, std::function<float(float, float)> angle)
{
	if (!loadObject("tree\\tree_trunk.obj", mVertexBuffer[0], mIndexBuffer[0]))
		return false;

	if (!loadObject("tree\\tree_leaves.obj", mVertexBuffer[1], mIndexBuffer[1]))
		return false;

	if (!createInstances(height, angle))
		return false;

	mVertexDeclaration.reset(CreateDeclaration(mDevice, vertexElement));
	if (!mVertexDeclaration)
		return false;

	mTexture[0].reset(LoadTexture(mDevice, L"tree\\tree_bark.tga"));
	mTexture[1].reset(LoadTexture(mDevice, L"tree\\tree_leaves.tga"));
	if (!mTexture[0] || !mTexture[1])
		return false;

	mEffect.reset(CreateEffect(mDevice, L"tree.fx"));
	if (!mEffect)
		return false;

	return true;
}

//*********************************************************************************************************************

void Tree::update(const float /*tick*/)
{
}

//*********************************************************************************************************************

void Tree::draw(TreeRenderMode mode)
{
	D3DXMATRIX matProjection;
	mDevice->GetTransform(D3DTS_PROJECTION, &matProjection);
	D3DXMatrixTranspose(&matProjection, &matProjection);
	mEffect->SetMatrix("Projection", &matProjection);

	D3DXMATRIX matView;
	mDevice->GetTransform(D3DTS_VIEW, &matView);
	D3DXMatrixTranspose(&matView, &matView);
	mEffect->SetMatrix("View", &matView);

	mDevice->SetVertexDeclaration(mVertexDeclaration.get());

	mDevice->SetStreamSource(1, mInstanceBuffer.get(), 0, sizeof(Instance));
	mDevice->SetStreamSourceFreq(1, (D3DSTREAMSOURCE_INSTANCEDATA | 1ul));

	if (mode == TreeRenderMode::Plain)
	{
		mDevice->SetStreamSource(0, mVertexBuffer[0].get(), 0, sizeof(Vertex));
		mDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | maxInstanceCount));

		mDevice->SetIndices(mIndexBuffer[0].get());

		mEffect->SetTexture("Texture0", mTexture[0].get());
		mEffect->SetTechnique("Trunk");

		RenderEffect(mEffect.get(), [this]()
		{
			mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mIndexCount, 0, mIndexCount / 3);
		});
	}

	mDevice->SetStreamSource(0, mVertexBuffer[1].get(), 0, sizeof(Vertex));
	mDevice->SetStreamSourceFreq(0, (D3DSTREAMSOURCE_INDEXEDDATA | maxInstanceCount));

	mDevice->SetIndices(mIndexBuffer[1].get());

	mEffect->SetTexture("Texture0", mTexture[1].get());
	if (mode == TreeRenderMode::Plain)
		mEffect->SetTechnique("PlainLeaves");
	else
		mEffect->SetTechnique("BlendLeaves");

	RenderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mIndexCount, 0, mIndexCount / 3);
	});

	mDevice->SetStreamSourceFreq(0, 1);
	mDevice->SetStreamSourceFreq(1, 1);
}

//*********************************************************************************************************************

bool Tree::loadObject(std::string filename, VertexBuffer& vertexbuffer, IndexBuffer& indexbuffer)
{
	Array<ObjectVertex> vertex;
	Array<short> index;

	if (!LoadObject(filename, vertex, index))
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

			if (r >= 90)
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

	mInstanceBuffer.reset(CreateVertexBuffer(mDevice, instance, sizeof(Instance), maxInstanceCount, 0));
	if (!mInstanceBuffer)
		return false;

	return true;
}

//*********************************************************************************************************************
