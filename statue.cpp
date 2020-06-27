#include "statue.h"
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
		{ 0, 6 * 4, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 },
		{ 0, 9 * 4, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0 },
		{ 0, 12 * 4, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
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

	const D3DVERTEXELEMENT9 linesVertexElement[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 3 * 4, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		D3DDECL_END()
	};

	struct LinesVertex
	{
		D3DXVECTOR3 position;
		D3DCOLOR col{};
	};

	struct Lines
	{
		VertexBuffer mVertexBuffer{ nullptr, vertexDeleter };
		Declaration mVertexDeclaration{ nullptr, declarationDeleter };
		int mVertexCount{ 0 };
	} gLines;
}

//*********************************************************************************************************************

Statue::Statue(IDirect3DDevice9* pDevice)
	: mDevice(pDevice)
	, mVertexBuffer(nullptr, vertexDeleter)
	, mIndexBuffer(nullptr, indexDeleter)
	, mTexture{ { nullptr, textureDeleter }, { nullptr, textureDeleter } }
	, mEffect(nullptr, effectDeleter)
	, mVertexDeclaration(nullptr, declarationDeleter)
	, mIndexCount(0)
{
}

//*********************************************************************************************************************

bool generateLines(IDirect3DDevice9* pDevice, Vertex* vertex_buffer, int vertexCount)
{
	gLines.mVertexDeclaration.reset(CreateDeclaration(pDevice, linesVertexElement));
	if (!gLines.mVertexDeclaration)
		return false;

	gLines.mVertexCount = vertexCount * 6;
	LinesVertex* lines_vertex_buffer = new LinesVertex[gLines.mVertexCount];

	for (int i = 0; i < vertexCount; i++)
	{
		lines_vertex_buffer[i * 6].col = D3DCOLOR_XRGB(0, 0, 255);
		lines_vertex_buffer[i * 6].position = vertex_buffer[i].position;
		lines_vertex_buffer[i * 6 + 1].col = D3DCOLOR_XRGB(0, 0, 255);
		lines_vertex_buffer[i * 6 + 1].position = vertex_buffer[i].position + vertex_buffer[i].normal * 0.5f;

		lines_vertex_buffer[i * 6 + 2].col = D3DCOLOR_XRGB(255, 0, 0);
		lines_vertex_buffer[i * 6 + 2].position = vertex_buffer[i].position;
		lines_vertex_buffer[i * 6 + 3].col = D3DCOLOR_XRGB(255, 0, 0);
		lines_vertex_buffer[i * 6 + 3].position = vertex_buffer[i].position + vertex_buffer[i].tangent * 0.5f;

		lines_vertex_buffer[i * 6 + 4].col = D3DCOLOR_XRGB(0, 255, 0);
		lines_vertex_buffer[i * 6 + 4].position = vertex_buffer[i].position;
		lines_vertex_buffer[i * 6 + 5].col = D3DCOLOR_XRGB(0, 255, 0);
		lines_vertex_buffer[i * 6 + 5].position = vertex_buffer[i].position + vertex_buffer[i].bitangent * 0.5f;
	}

	gLines.mVertexBuffer.reset(CreateVertexBuffer(pDevice, lines_vertex_buffer, sizeof(LinesVertex), gLines.mVertexCount, 0));
	delete[] lines_vertex_buffer;
	if (!gLines.mVertexBuffer)
		return false;

	return true;
}


//*********************************************************************************************************************

bool Statue::init()
{
	if (!loadObject("statue\\statue.obj", mVertexBuffer, mIndexBuffer))
		return false;

	mVertexDeclaration.reset(CreateDeclaration(mDevice, vertexElement));
	if (!mVertexDeclaration)
		return false;

	mTexture[0].reset(LoadTexture(mDevice, L"statue\\statue_texture_tga_dxt1_1.dds"));
	mTexture[1].reset(LoadTexture(mDevice, L"statue\\statue_normal.png"));
	if (!mTexture[0] || !mTexture[1])
		return false;
	
	mEffect.reset(CreateEffect(mDevice, L"statue.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTexture("Texture0", mTexture[0].get());
	mEffect->SetTexture("Texture1", mTexture[1].get());

	return true;
}

//*********************************************************************************************************************

void Statue::update(const float /*tick*/)
{
}

//*********************************************************************************************************************

void Statue::draw(StatueRenderMode mode, const D3DXVECTOR3& camPos)
{
	if (mode == StatueRenderMode::Reflect)
		mEffect->SetTechnique("Reflect");
	else if (mode == StatueRenderMode::Refract)
		mEffect->SetTechnique("Refract");
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

	D3DXMATRIX matWorld, matTrans, matScale;
	D3DXMatrixScaling(&matScale, 0.5f, 0.5f, 0.5f);
	D3DXMatrixTranslation(&matTrans, 0, 0, 0);
	matWorld = matScale * matTrans;
	mDevice->SetTransform(D3DTS_WORLD, &matWorld);
	D3DXMatrixTranspose(&matWorld, &matWorld);
	mEffect->SetMatrix("World", &matWorld);

	mEffect->SetFloatArray("CameraPosition", (float*)&camPos, 3);

	mDevice->SetVertexDeclaration(mVertexDeclaration.get());
	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(Vertex));
	mDevice->SetIndices(mIndexBuffer.get());

	RenderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mIndexCount, 0, mIndexCount / 3);
	});

	//if (mode == StatueRenderMode::Normal)
	//{
	//	mEffect->SetTechnique("Lines");
	//	mDevice->SetVertexDeclaration(gLines.mVertexDeclaration.get());
	//	mDevice->SetStreamSource(0, gLines.mVertexBuffer.get(), 0, sizeof(LinesVertex));

	//	RenderEffect(mEffect.get(), [this]()
	//	{
	//		mDevice->DrawPrimitive(D3DPT_LINELIST, 0, gLines.mVertexCount / 2);
	//	});
	//}
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

bool Statue::loadObject(std::string filename, VertexBuffer& vertexbuffer, IndexBuffer& indexbuffer)
{
	std::vector<WFOVertex> vertex;
	std::vector<short> index;

	if (!LoadWFObject(filename, vertex, index))
		return false;

	int vertexCount = static_cast<int>(vertex.size());
	Vertex* vertex_buffer = new Vertex[vertexCount];
	for (int i = 0; i < vertexCount; i++)
	{
		vertex_buffer[i] =
		{
			.position = vertex[i].p,
			.normal = vertex[i].n,
			.tangent = { 0, 0, 0 },
			.bitangent = { 0, 0, 0 },
			.texcoord = vertex[i].t,
		};
	}

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

	bool gotLines = true;// generateLines(mDevice, vertex_buffer, vertexCount);

	vertexbuffer.reset(CreateVertexBuffer(mDevice, vertex_buffer, sizeof(Vertex), vertexCount, 0));
	delete[] vertex_buffer;

	indexbuffer.reset(CreateIndexBuffer(mDevice, index_buffer, mIndexCount));
	delete[] index_buffer;

	if (!vertexbuffer || !indexbuffer || !gotLines)
		return false;

	return true;
}

//*********************************************************************************************************************
