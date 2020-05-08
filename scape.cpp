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
	, mTexture{ { nullptr, textureDeleter }, { nullptr, textureDeleter } }
	, mEffect(nullptr, effectDeleter)
	, mHeight(0)
	, mHeightSize(0)
	, mChunk(1)
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
	mHeightSize = 256;

	if (!loadHeightmap(mHeightSize, 50))
		return false;

	if (!(mIndexCount[0] = generateIndices(mIndexBuffer[0], mHeightSize)))
		return false;

	if (!(mIndexCount[1] = generateIndices(mIndexBuffer[1], mHeightSize / 2)))
		return false;

	if (!(mIndexCount[2] = generateIndices(mIndexBuffer[2], mHeightSize / 4)))
		return false;

	if (!(mIndexCount[3] = generateIndices(mIndexBuffer[3], mHeightSize / 8)))
		return false;

	if (!generateVertices(mChunk[0].mLod[0], mHeightSize, 1))
		return false;

	if (!generateVertices(mChunk[0].mLod[1], mHeightSize / 2, 2))
		return false;

	if (!generateVertices(mChunk[0].mLod[2], mHeightSize / 4, 4))
		return false;

	if (!generateVertices(mChunk[0].mLod[3], mHeightSize / 8, 8))
		return false;

	mTexture[0].reset(CreateTexture(mDevice, L"cliff_pak_1_2005\\grass_01_v1.tga"));
	if (!mTexture[0])
		return false;

	mTexture[1].reset(CreateTexture(mDevice, L"cliff_pak_1_2005\\cliff_01_v2.tga"));
	if (!mTexture[1])
		return false;

	mEffect.reset(CreateEffect(mDevice, L"scape.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTechnique("Technique0");
	mEffect->SetTexture("Texture0", mTexture[0].get());
	mEffect->SetTexture("Texture1", mTexture[1].get());

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

	D3DXMATRIX matWorld;
	D3DXMatrixIdentity(&matWorld);
	mDevice->SetTransform(D3DTS_WORLD, &matWorld);
	mEffect->SetMatrix("World", &matWorld);

	D3DXMATRIX worldViewProjection = matWorld * matView * matProjection;
	D3DXMatrixTranspose(&worldViewProjection, &worldViewProjection);
	mEffect->SetMatrix("WorldViewProj", &worldViewProjection);

	mDevice->SetFVF(vertexFVF);
	mDevice->SetStreamSource(0, mChunk[0].mLod[mLodIndex].pVertexBuffer.get(), 0, sizeof Vertex);
	mDevice->SetIndices(mIndexBuffer[mLodIndex].get());

	RenderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mChunk[0].mLod[mLodIndex].vertexCount, 0, mIndexCount[mLodIndex] / 3);
	});
}

//*********************************************************************************************************************

bool Scape::loadHeightmap(const int size, const float scale)
{
	const int pointCount = size * size;
	mHeight.resize(pointCount);

	FILE* f{};
	if (fopen_s(&f, "output.r32", "rb") || !f)
		return false;

	for (int i = 0; i < pointCount; i++)
	{
		float val;
		fread(&val, sizeof(float), 1, f);
		mHeight[i] = scale * val;
	}

	if (ferror(f))
		return false;

	fclose(f);

	return true;
}

//*********************************************************************************************************************

int Scape::generateIndices(IndexBuffer& pIndexBuffer, const int size)
{
	const int nCellCols = size - 1;
	const int nCellRows = size - 1;
	const int nVerticesPerCell = 2 * 3;
	const int indexCount = nCellCols * nCellRows * nVerticesPerCell;

	short* indices = new short[indexCount];

	int baseIndex = 0;
	for (int y = 0; y < nCellRows; y++)
		for (int x = 0; x < nCellCols; x++)
		{
			// triangle 1
			indices[baseIndex + 0] = (short)((x) + (y) * size);
			indices[baseIndex + 1] = (short)((x + 1) + (y) * size);
			indices[baseIndex + 2] = (short)((x) + (y + 1) * size);

			// triangle 2
			indices[baseIndex + 3] = (short)((x) + (y + 1) * size);
			indices[baseIndex + 4] = (short)((x + 1) + (y) * size);
			indices[baseIndex + 5] = (short)((x + 1) + (y + 1) * size);

			baseIndex += 6; // next cell
		}

	pIndexBuffer.reset(CreateIndexBuffer(mDevice, indices, indexCount));

	delete[] indices;

	if (!pIndexBuffer)
		return 0;

	return indexCount;
}

//*********************************************************************************************************************

bool Scape::generateVertices(Lod& lod, const int size, const float scale)
{
	const int vertexCount = size * size;
	Vertex* vertices = new Vertex[vertexCount];

	const int ratio = mHeightSize / size;

	for (int y = 0; y < size; y++)
		for (int x = 0; x < size; x++)
		{
			vertices[x + y * size].p.x = scale * (float)(x - (size / 2));
			vertices[x + y * size].p.z = scale * (float)(y - (size / 2));

			vertices[x + y * size].p.y = mHeight[(x * ratio) + (y * ratio) * mHeightSize];

			vertices[x + y * size].n.x = 0.0f;
			vertices[x + y * size].n.z = 0.0f;
			vertices[x + y * size].n.y = 1.0f;

			vertices[x + y * size].u = (float)(x) / (size / 20);
			vertices[x + y * size].v = (float)(y) / (size / 20);
		}

	// hack - put face normals in vertex normals
	for (int z = 0; z < (size - 1); z++)
		for (int x = 0; x < (size - 1); x++)
		{
			D3DXVECTOR3 p(vertices[(x + 0) + (z + 0) * size].p);
			D3DXVECTOR3 q(vertices[(x + 1) + (z + 0) * size].p);
			D3DXVECTOR3 r(vertices[(x + 0) + (z + 1) * size].p);

			D3DXVECTOR3 u(p - q);
			D3DXVECTOR3 v(p - r);
			D3DXVECTOR3 n;

			D3DXVec3Cross(&n, &v, &u);

			D3DXVec3Normalize(&vertices[x + z * size].n, &n);
		}

	lod.pVertexBuffer.reset(CreateVertexBuffer(mDevice, vertices, sizeof(Vertex), vertexCount, vertexFVF));
	lod.vertexCount = vertexCount;

	delete[] vertices;

	if (!lod.pVertexBuffer)
		return false;

	return true;
}

//*********************************************************************************************************************
