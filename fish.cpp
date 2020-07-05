#include "fish.h"
#include "wavefront.h"
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
		D3DXVECTOR2 texcoord;
	};

	struct Instance
	{
		D3DXVECTOR4 m0;
		D3DXVECTOR4 m1;
		D3DXVECTOR4 m2;
		D3DXVECTOR4 m3;
	};

	constexpr int maxInstanceCount = 2;
	Instance instance[maxInstanceCount];
}

//*********************************************************************************************************************

Fish::Fish(IDirect3DDevice9* pDevice)
	: mDevice{ pDevice }
	, mVertexBuffer{ nullptr, vertexDeleter }
	, mIndexBuffer{ nullptr, indexDeleter }
	, mInstanceBuffer{ nullptr, vertexDeleter }
	, mTexture{ nullptr, textureDeleter }
	, mEffect{ nullptr, effectDeleter }
	, mVertexDeclaration{ nullptr, declarationDeleter }
	, mIndexCount{ 0 }
	, mAngle{ 0 }
{
}

//*********************************************************************************************************************

bool Fish::init()
{
	if (!loadObject("fish\\tropicalfish12.obj", mVertexBuffer, mIndexBuffer))
		return false;

	if (!createInstances())
		return false;

	mVertexDeclaration.reset(CreateDeclaration(mDevice, vertexElement));
	if (!mVertexDeclaration)
		return false;

	mTexture.reset(LoadTexture(mDevice, L"fish\\results\\tropicalfish12_jpg_dxt1_1.dds"));
	if (!mTexture)
		return false;

	mEffect.reset(CreateEffect(mDevice, L"fish.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTexture("Texture0", mTexture.get());

	return true;
}

//*********************************************************************************************************************

void Fish::update(const float /*tick*/)
{
	mAngle += 3;
	if (mAngle >= 360)
		mAngle = 0;
}

//*********************************************************************************************************************

void Fish::draw(FishRenderMode mode)
{
	if (mode == FishRenderMode::Reflect)
		mEffect->SetTechnique("Reflect");
	else
		mEffect->SetTechnique("Normal");

	mEffect->SetFloat("Angle", D3DXToRadian(mAngle));

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

bool Fish::loadObject(std::string filename, VertexBuffer& vertexbuffer, IndexBuffer& indexbuffer)
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
			.texcoord = vertex[i].t,
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

bool Fish::createInstances()
{
	D3DXMATRIX matTrans;
	D3DXMatrixTranslation(&matTrans, -10, -1, 40);

	float s = 0.005f;
	D3DXMATRIX matScale;
	D3DXMatrixScaling(&matScale, s, s, s);

	D3DXMATRIX matRotY;
	D3DXMatrixRotationY(&matRotY, D3DXToRadian(rand() % 360));

	D3DXMATRIX matWorld = matRotY * matScale * matTrans;
	D3DXMatrixTranspose(&matWorld, &matWorld);
	for (int n = 0; n < 4; n++)
	{
		instance[0].m0[n] = matWorld.m[0][n];
		instance[0].m1[n] = matWorld.m[1][n];
		instance[0].m2[n] = matWorld.m[2][n];
		instance[0].m3[n] = matWorld.m[3][n];
	}

	D3DXMatrixTranslation(&matTrans, -11, -1.5, 40);

	s = 0.003f;
	D3DXMatrixScaling(&matScale, s, s, s);

	matWorld = matRotY * matScale * matTrans;
	D3DXMatrixTranspose(&matWorld, &matWorld);
	for (int n = 0; n < 4; n++)
	{
		instance[1].m0[n] = matWorld.m[0][n];
		instance[1].m1[n] = matWorld.m[1][n];
		instance[1].m2[n] = matWorld.m[2][n];
		instance[1].m3[n] = matWorld.m[3][n];
	}

	mInstanceBuffer.reset(CreateVertexBuffer(mDevice, instance, sizeof(Instance), maxInstanceCount, 0));
	if (!mInstanceBuffer)
		return false;

	return true;
}

//*********************************************************************************************************************
