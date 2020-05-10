#include "scape.h"

//*********************************************************************************************************************

namespace
{
	const unsigned long vertexFVF{ D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) };
	struct Vertex
	{
		D3DXVECTOR3 p, n;
		float u{}, v{};
	};
}

//*********************************************************************************************************************

Scape::Scape(IDirect3DDevice9* pDevice)
	: iMesh(pDevice)
	, mTexture{ { nullptr, textureDeleter }, { nullptr, textureDeleter }, { nullptr, textureDeleter } }
	, mEffect(nullptr, effectDeleter)
	, mHeightmap(0)
	, mHeightmapSize(256)
	, mChunk(16)
	, mLodIndex(0)
	, mIndexBuffer{ { nullptr, indexDeleter }, { nullptr, indexDeleter }, { nullptr, indexDeleter }, { nullptr, indexDeleter } }
	, mIndexCount{ 0, 0, 0, 0 }
{
}

//*********************************************************************************************************************

#pragma warning( push )
#pragma warning( disable : 4706 )

bool Scape::init()
{
	if (!loadHeightmap(mHeightmapSize, 50))
		return false;

	if (!(mIndexCount[0] = generateIndices(mIndexBuffer[0], mHeightmapSize / 4)))
		return false;

	if (!(mIndexCount[1] = generateIndices(mIndexBuffer[1], mHeightmapSize / 8)))
		return false;

	if (!(mIndexCount[2] = generateIndices(mIndexBuffer[2], mHeightmapSize / 16)))
		return false;

	if (!(mIndexCount[3] = generateIndices(mIndexBuffer[3], mHeightmapSize / 32)))
		return false;

	for (int i = 0; i < 16; i++)
	{
		int x = (i % 4);
		int y = (i / 4);

		mChunk[i].mapX = x * 64.0f - 128.0f;
		mChunk[i].mapY = y * 64.0f - 128.0f;

		int xoffs = x * (mHeightmapSize / 4);
		int yoffs = y * (mHeightmapSize / 4) * mHeightmapSize;
		int offset = xoffs + yoffs;

		if (!generateVertices(mChunk[i].mLod[0], (mHeightmapSize / 4) + 1, 1, offset))
			return false;

		if (!generateVertices(mChunk[i].mLod[1], (mHeightmapSize / 8) + 1, 2, offset))
			return false;

		if (!generateVertices(mChunk[i].mLod[2], (mHeightmapSize / 16) + 1, 4, offset))
			return false;

		if (!generateVertices(mChunk[i].mLod[3], (mHeightmapSize / 32) + 1, 8, offset))
			return false;
	}

	mTexture[0].reset(CreateTexture(mDevice, L"cliff_pak_1_2005\\grass_01_v1.tga"));
	if (!mTexture[0])
		return false;

	mTexture[1].reset(CreateTexture(mDevice, L"cliff_pak_1_2005\\cliff_01_v2.tga"));
	if (!mTexture[1])
		return false;

	mTexture[2].reset(CreateTexture(mDevice, L"cliff_pak_1_2005\\cliff_03_v1.tga"));
	if (!mTexture[2])
		return false;

	mEffect.reset(CreateEffect(mDevice, L"scape.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTechnique("Technique0");
	mEffect->SetTexture("Texture0", mTexture[0].get());
	mEffect->SetTexture("Texture1", mTexture[1].get());
	mEffect->SetTexture("Texture2", mTexture[2].get());

	return true;

#pragma warning( pop )
}

//*********************************************************************************************************************

void Scape::update(const float /*tick*/)
{
	static int count = 0;
	count++;

	if (count > 100)
	{
		mLodIndex++;
		if (mLodIndex > 3)
			mLodIndex = 0;

		count = 0;
	}
}

//*********************************************************************************************************************

void Scape::draw()
{
	D3DXMATRIX matProjection;
	mDevice->GetTransform(D3DTS_PROJECTION, &matProjection);

	D3DXMATRIX matView;
	mDevice->GetTransform(D3DTS_VIEW, &matView);

	for (int i = 0; i < 16; i++)
	{
		D3DXMATRIX matWorld;
		D3DXMatrixTranslation(&matWorld, mChunk[i].mapX, 0, mChunk[i].mapY);
		mDevice->SetTransform(D3DTS_WORLD, &matWorld);
		mEffect->SetMatrix("World", &matWorld);

		D3DXMATRIX worldViewProjection = matWorld * matView * matProjection;
		D3DXMatrixTranspose(&worldViewProjection, &worldViewProjection);
		mEffect->SetMatrix("WorldViewProj", &worldViewProjection);

		mDevice->SetFVF(vertexFVF);
		mDevice->SetStreamSource(0, mChunk[i].mLod[mLodIndex].pVertexBuffer.get(), 0, sizeof Vertex);
		mDevice->SetIndices(mIndexBuffer[mLodIndex].get());

		RenderEffect(mEffect.get(), [this, i]()
		{
			mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mChunk[i].mLod[mLodIndex].vertexCount, 0, mIndexCount[mLodIndex] / 3);
		});
	}
}

//*********************************************************************************************************************

bool Scape::loadHeightmap(const int size, const float scale)
{
	const int pointCount = size * size;
	mHeightmap.resize(pointCount);

	FILE* f{};
	if (fopen_s(&f, "output.r32", "rb") || !f)
		return false;

	for (int i = 0; i < pointCount; i++)
	{
		float val;
		fread(&val, sizeof(float), 1, f);
		mHeightmap[i] = scale * val;
	}

	if (ferror(f))
		return false;

	fclose(f);

	return true;
}

//*********************************************************************************************************************

int Scape::generateIndices(IndexBuffer& pIndexBuffer, const int size)
{
	const int indexCount = size * size * 6;
	short* indices = new short[indexCount];

	int baseIndex = 0;
	for (int y = 0; y < size; y++)
		for (int x = 0; x < size; x++)
		{
			// triangle 1
			indices[baseIndex + 0] = (short)((x) + (y) * (size + 1));
			indices[baseIndex + 1] = (short)((x + 1) + (y) * (size + 1));
			indices[baseIndex + 2] = (short)((x) + (y + 1) * (size + 1));

			// triangle 2
			indices[baseIndex + 3] = (short)((x) + (y + 1) * (size + 1));
			indices[baseIndex + 4] = (short)((x + 1) + (y) * (size + 1));
			indices[baseIndex + 5] = (short)((x + 1) + (y + 1) * (size + 1));

			baseIndex += 6; // next cell
		}

	pIndexBuffer.reset(CreateIndexBuffer(mDevice, indices, indexCount));

	delete[] indices;

	if (!pIndexBuffer)
		return 0;

	return indexCount;
}

//*********************************************************************************************************************

float Scape::getHeight(const int offset, const int x, const int y, const int scale)
{
	float h = 0;
	for (int j = 0; j < scale; j++)
		for (int i = 0; i < scale; i++)
		{
			const int index = offset + ((x * scale) + i) + ((y * scale) + j) * mHeightmapSize;
			if (index < mHeightmap.size())
				h += mHeightmap[index];
		}
	return h / (scale * scale);
}

//*********************************************************************************************************************

bool Scape::generateVertices(Lod& lod, const int size, const int scale, const int offset)
{
	const int vertexCount = size * size;
	Vertex* vertices = new Vertex[vertexCount];

	for (int y = 0; y < size; y++)
		for (int x = 0; x < size; x++)
		{
			vertices[x + y * size].p.x = (float)(scale * x);
			vertices[x + y * size].p.z = (float)(scale * y);

			vertices[x + y * size].p.y = getHeight(offset, x, y, scale);

			vertices[x + y * size].n.x = 0.0f;
			vertices[x + y * size].n.z = 0.0f;
			vertices[x + y * size].n.y = 1.0f;

			vertices[x + y * size].u = (float)(x) / ((size - 1) / 3.0f);
			vertices[x + y * size].v = (float)(y) / ((size - 1) / 3.0f);
		}

	// hack - put face normals in vertex normals - pain on edges between chunks
	for (int y = 0; y < (size - 1); y++)
		for (int x = 0; x < (size - 1); x++)
		{
			D3DXVECTOR3 p(vertices[(x + 0) + (y + 0) * size].p);
			D3DXVECTOR3 q(vertices[(x + 1) + (y + 0) * size].p);
			D3DXVECTOR3 r(vertices[(x + 0) + (y + 1) * size].p);

			D3DXVECTOR3 u(p - q);
			D3DXVECTOR3 v(p - r);
			D3DXVECTOR3 n;

			D3DXVec3Cross(&n, &v, &u);

			D3DXVec3Normalize(&vertices[x + y * size].n, &n);
		}

	lod.pVertexBuffer.reset(CreateVertexBuffer(mDevice, vertices, sizeof(Vertex), vertexCount, vertexFVF));
	lod.vertexCount = vertexCount;

	delete[] vertices;

	if (!lod.pVertexBuffer)
		return false;

	return true;
}

//*********************************************************************************************************************
