#include "scape.h"
#include "array.h"

//*********************************************************************************************************************

namespace
{
	constexpr float wrap = 2.5f;

	const unsigned long vertexFVF{ D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) };
	struct Vertex
	{
		D3DXVECTOR3 p, n;
		float u{}, v{};
	};

	bool operator==(const Vertex& a, const Vertex& b)
	{
		return (a.p == b.p);
	}
}

//*********************************************************************************************************************

Scape::Scape(IDirect3DDevice9* pDevice)
	: mDevice(pDevice)
	, mTexture{ { nullptr, textureDeleter }, { nullptr, textureDeleter }, { nullptr, textureDeleter } }
	, mEffect(nullptr, effectDeleter)
	, mHeightmap(0)
	, mHeightmapSize(3 * 67 + 1)
	, mChunk(9)
	, mIndexBuffer{ { nullptr, indexDeleter }, { nullptr, indexDeleter }, { nullptr, indexDeleter }, { nullptr, indexDeleter }, { nullptr, indexDeleter } }
	, mIndexCount{}
{
}

//*********************************************************************************************************************

bool Scape::init()
{
	if (!loadHeightmap(mHeightmapSize, 55, 6.0f))
		return false;

	mIndexCount[0] = generateIndices(mIndexBuffer[0], 66);
	mIndexCount[1] = generateIndices(mIndexBuffer[1], 32);
	mIndexCount[2] = generateIndices(mIndexBuffer[2], 16);
	mIndexCount[3] = generateIndices(mIndexBuffer[3], 8);
	if (!mIndexCount[0] || !mIndexCount[1] || !mIndexCount[2] || !mIndexCount[3])
		return false;

	for (int i = 0; i < 9; i++)
	{
		const int x = (i % 3);
		const int y = (i / 3);

		mChunk[i].mPosX = 66.0f * x;
		mChunk[i].mPosY = 66.0f * y;

		int offset = (66 * x) + (66 * y) * mHeightmapSize;

		if (!generateVertices(mChunk[i].mLod[0], 67, 1, offset))
			return false;

		if (!generateSkirt(mChunk[i].mLod[1], 67, 2, offset))
			return false;

		if (!generateSkirt(mChunk[i].mLod[2], 67, 4, offset))
			return false;

		if (!generateSkirt(mChunk[i].mLod[3], 67, 8, offset))
			return false;

		offset = (66 * x + 1) + (66 * y + 1) * mHeightmapSize;

		if (!generateVertices(mChunk[i].mLod[1], 33, 2, offset))
			return false;

		if (!generateVertices(mChunk[i].mLod[2], 17, 4, offset))
			return false;

		if (!generateVertices(mChunk[i].mLod[3], 9, 8, offset))
			return false;
	}

	mTexture[0].reset(LoadTexture(mDevice, L"cliff_pak_1_2005\\grass_01_v1.tga"));
	mTexture[1].reset(LoadTexture(mDevice, L"cliff_pak_1_2005\\cliff_01_v2.tga"));
	mTexture[2].reset(LoadTexture(mDevice, L"cliff_pak_1_2005\\cliff_03_v1.tga"));
	if (!mTexture[0] || !mTexture[1] || !mTexture[2])
		return false;

	mEffect.reset(CreateEffect(mDevice, L"scape.fx"));
	if (!mEffect)
		return false;

	mEffect->SetTexture("Texture0", mTexture[0].get());
	mEffect->SetTexture("Texture1", mTexture[1].get());
	mEffect->SetTexture("Texture2", mTexture[2].get());

	return true;
}

//*********************************************************************************************************************

void Scape::update(const float /*tick*/)
{
}

//*********************************************************************************************************************

void Scape::draw(const ScapeRenderMode mode, D3DXVECTOR3 camPos)
{
	camPos.y = 0;

	if (mode == ScapeRenderMode::Reflect)
		mEffect->SetTechnique("Technique1");
	else
		mEffect->SetTechnique("Technique0");

	D3DXMATRIX matProjection;
	mDevice->GetTransform(D3DTS_PROJECTION, &matProjection);
	D3DXMatrixTranspose(&matProjection, &matProjection);
	mEffect->SetMatrix("Projection", &matProjection);

	D3DXMATRIX matView;
	mDevice->GetTransform(D3DTS_VIEW, &matView);
	D3DXMatrixTranspose(&matView, &matView);
	mEffect->SetMatrix("View", &matView);

	for (auto& chunk : mChunk)
	{
		int lodIndex = 0;
		D3DXVECTOR3 lodPos(chunk.mPosX, 0.0f, chunk.mPosY);
		D3DXVECTOR3 distVec = lodPos - camPos;
		const float distance = D3DXVec3Length(&distVec);
		if (distance > 120)
			lodIndex = 3;
		else if (distance > 90)
			lodIndex = 2;
		else if (distance > 60)
			lodIndex = 1;

		D3DXMATRIX matWorld;
		D3DXMatrixTranslation(&matWorld, chunk.mPosX, 0.0f, chunk.mPosY);
		mDevice->SetTransform(D3DTS_WORLD, &matWorld);
		D3DXMatrixTranspose(&matWorld, &matWorld);
		mEffect->SetMatrix("World", &matWorld);

		mDevice->SetFVF(vertexFVF);
		mDevice->SetStreamSource(0, chunk.mLod[lodIndex].mVertexBuffer[0].get(), 0, sizeof Vertex);
		mDevice->SetIndices(mIndexBuffer[lodIndex].get());

		RenderEffect(mEffect.get(), [this, &chunk, lodIndex]()
		{
			mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, chunk.mLod[lodIndex].mVertexCount[0], 0, mIndexCount[lodIndex] / 3);
		});

		if (chunk.mLod[lodIndex].mVertexBuffer[1])
		{
			mDevice->SetStreamSource(0, chunk.mLod[lodIndex].mVertexBuffer[1].get(), 0, sizeof Vertex);
			mDevice->SetIndices(mIndexBuffer[4].get());

			RenderEffect(mEffect.get(), [this, &chunk, lodIndex]()
			{
				mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, chunk.mLod[lodIndex].mVertexCount[1], 0, mIndexCount[4] / 3);
			});
		}
	}
}

//*********************************************************************************************************************

bool Scape::loadHeightmap(const int size, const float scale, const float sealevel)
{
	const int pointCount = size * size;
	mHeightmap.resize(pointCount);

	FILE* f{};
	if (fopen_s(&f, "output.r32", "rb") || !f)
		return false;

	int index = 0;
	for (int j = 0; j < 256; j++)
	{
		for (int i = 0; i < 256; i++)
		{
			float val;
			fread(&val, sizeof(float), 1, f);

			if (i < size)
			{
				mHeightmap[index] = scale * val - sealevel;
				index++;
			}
		}

		if (index == pointCount)
			break;
	}

	if (ferror(f))
		return false;

	fclose(f);

	return true;
}

//*********************************************************************************************************************

unsigned int Scape::generateIndices(IndexBuffer& pIndexBuffer, const int size)
{
	const unsigned int indexCount = size * size * 6;
	short* indices = new short[indexCount];

	for (unsigned int i = 0; i < indexCount; i += 6)
	{
		int y = (i / 6) / size;
		int x = (i / 6) % size;

		// triangle 1
		indices[i + 0] = static_cast<short>((x + 0) + (y + 0) * (size + 1));
		indices[i + 1] = static_cast<short>((x + 1) + (y + 0) * (size + 1));
		indices[i + 2] = static_cast<short>((x + 0) + (y + 1) * (size + 1));

		// triangle 2
		indices[i + 3] = static_cast<short>((x + 0) + (y + 1) * (size + 1));
		indices[i + 4] = static_cast<short>((x + 1) + (y + 0) * (size + 1));
		indices[i + 5] = static_cast<short>((x + 1) + (y + 1) * (size + 1));
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
	const int index = offset + (x * scale) + (y * scale) * mHeightmapSize;
	return mHeightmap[index];
}

//*********************************************************************************************************************

D3DXVECTOR3 Scape::getNormal(const int offset, const int x, const int y)
{
	D3DXVECTOR3 normal;

	D3DXVECTOR3 p((x + 0.0f), getHeight(offset, x + 0, y + 0, 1), (y + 0.0f));
	D3DXVECTOR3 q((x + 1.0f), getHeight(offset, x + 1, y + 0, 1), (y + 0.0f));
	D3DXVECTOR3 r((x + 0.0f), getHeight(offset, x + 0, y + 1, 1), (y + 1.0f));

	D3DXVECTOR3 u(p - q);
	D3DXVECTOR3 v(p - r);
	D3DXVECTOR3 n;

	D3DXVec3Cross(&n, &v, &u);

	D3DXVec3Normalize(&normal, &n);

	return normal;
}

//*********************************************************************************************************************

bool Scape::generateVertices(Lod& lod, const int size, const int scale, const int offset)
{
	const int vertexCount = size * size;
	Vertex* vertices = new Vertex[vertexCount];

	for (int y = 0; y < size; y++)
		for (int x = 0; x < size; x++)
		{
			vertices[x + y * size].p.x = static_cast<float>(scale * (x - size / 2));
			vertices[x + y * size].p.z = static_cast<float>(scale * (y - size / 2));

			vertices[x + y * size].p.y = getHeight(offset, x, y, scale);

			vertices[x + y * size].n = getNormal(offset, scale * x, scale * y);

			if (scale > 1)
			{
				vertices[x + y * size].u = (x * scale + 1) / ((scale * (size - 1) + 2) / wrap);
				vertices[x + y * size].v = (y * scale + 1) / ((scale * (size - 1) + 2) / wrap);
			}
			else
			{
				vertices[x + y * size].u = x / ((size - 1) / wrap);
				vertices[x + y * size].v = y / ((size - 1) / wrap);
			}
		}

	lod.mVertexBuffer[0].reset(CreateVertexBuffer(mDevice, vertices, sizeof(Vertex), vertexCount, vertexFVF));
	lod.mVertexCount[0] = vertexCount;

	delete[] vertices;

	if (!lod.mVertexBuffer[0])
		return false;

	return true;
}

//*********************************************************************************************************************

float Scape::getInnerHeight(int offset, int x, int y, int scale, int size)
{
	offset = offset + 1 + mHeightmapSize;
	size = size - 2;
	x--;
	y--;

	if (y == 0 || y == (size - 1))
	{
		const int stepX = x % scale;
		if (stepX == 0)
		{
			return getHeight(offset, x, y, 1);
		}
		else
		{
			float a = getHeight(offset, x - stepX, y, 1);
			float b = getHeight(offset, x + (scale - stepX), y, 1);
			float h = stepX * (b - a) / scale;
			return a + h;
		}
	}
	else if (x == 0 || x == (size - 1))
	{
		const int stepY = y % scale;
		if (stepY == 0)
		{
			return getHeight(offset, x, y, 1);
		}
		else
		{
			float a = getHeight(offset, x, y - stepY, 1);
			float b = getHeight(offset, x, y + (scale - stepY), 1);
			float h = stepY * (b - a) / scale;
			return a + h;
		}
	}
	return 0;
}

//*********************************************************************************************************************

void genCell(const Vertex& a, const Vertex& b, const Vertex& c, const Vertex& d, Array<Vertex>& vb, Array<short>& ib)
{
	short m = static_cast<short>(vb.appendAbsent(a));
	short n = static_cast<short>(vb.appendAbsent(b));
	short o = static_cast<short>(vb.appendAbsent(c));
	short p = static_cast<short>(vb.appendAbsent(d));

	ib.append({
		m, n, o,
		o, n, p
	});
}

//*********************************************************************************************************************

bool Scape::generateSkirt(Lod& lod, const int size, const int scale, const int offset)
{
	Array<Vertex> vb;
	Array<short> ib;

	const float m = static_cast<float>(size / 2);
	const float uv = (size - 1) / wrap;

	// corner - top left
	{
		int y = 0, x = 0;
		Vertex a{ D3DXVECTOR3(x - m, getHeight(offset, x, y, 1), y - m), getNormal(offset, x, y), x / uv, y / uv };
		Vertex b{ D3DXVECTOR3(x + 1 - m, getHeight(offset, x + 1, y, 1), y - m), getNormal(offset, x + 1, y), (x + 1) / uv, y / uv };
		Vertex c{ D3DXVECTOR3(x - m, getHeight(offset, x, y + 1, 1), y + 1 - m), getNormal(offset, x, y + 1), x / uv, (y + 1) / uv };
		Vertex d{ D3DXVECTOR3(x + 1 - m, getInnerHeight(offset, x + 1, y + 1, scale, size), y + 1 - m), getNormal(offset, x + 1, y + 1), (x + 1) / uv, (y + 1) / uv };
		genCell(a, b, c, d, vb, ib);
	}
	// horizontal - top
	{
		int y = 0;
		for (int x = 1; x < (size - 2); x++)
		{
			Vertex a{ D3DXVECTOR3(x - m, getHeight(offset, x, y, 1), y - m), getNormal(offset, x, y), x / uv, y / uv };
			Vertex b{ D3DXVECTOR3(x + 1 - m, getHeight(offset, x + 1, y, 1), y - m), getNormal(offset, x + 1, y), (x + 1) / uv, y / uv };
			Vertex c{ D3DXVECTOR3(x - m, getInnerHeight(offset, x, y + 1, scale, size), y + 1 - m), getNormal(offset, x, y + 1), x / uv, (y + 1) / uv };
			Vertex d{ D3DXVECTOR3(x + 1 - m, getInnerHeight(offset, x + 1, y + 1, scale, size), y + 1 - m), getNormal(offset, x + 1, y + 1), (x + 1) / uv, (y + 1) / uv };
			genCell(a, b, c, d, vb, ib);
		}
	}
	// corner - top right
	{
		int y = 0, x = size - 2;
		Vertex a{ D3DXVECTOR3(x - m, getHeight(offset, x, y, 1), y - m), getNormal(offset, x, y), x / uv, y / uv };
		Vertex b{ D3DXVECTOR3(x + 1 - m, getHeight(offset, x + 1, y, 1), y - m), getNormal(offset, x + 1, y), (x + 1) / uv, y / uv };
		Vertex c{ D3DXVECTOR3(x - m, getInnerHeight(offset, x, y + 1, scale, size), y + 1 - m), getNormal(offset, x, y + 1), x / uv, (y + 1) / uv };
		Vertex d{ D3DXVECTOR3(x + 1 - m, getHeight(offset, x + 1, y + 1, 1), y + 1 - m), getNormal(offset, x + 1, y + 1), (x + 1) / uv, (y + 1) / uv };
		genCell(a, b, c, d, vb, ib);
	}
	// vertical - right
	{
		int x = size - 2;
		for (int y = 1; y < (size - 2); y++)
		{
			Vertex a{ D3DXVECTOR3(x - m, getInnerHeight(offset, x, y, scale, size), y - m), getNormal(offset, x, y), x / uv, y / uv };
			Vertex b{ D3DXVECTOR3(x + 1 - m, getHeight(offset, x + 1, y, 1), y - m), getNormal(offset, x + 1, y), (x + 1) / uv, y / uv };
			Vertex c{ D3DXVECTOR3(x - m, getInnerHeight(offset, x, y + 1, scale, size), y + 1 - m), getNormal(offset, x, y + 1), x / uv, (y + 1) / uv };
			Vertex d{ D3DXVECTOR3(x + 1 - m, getHeight(offset, x + 1, y + 1, 1), y + 1 - m), getNormal(offset, x + 1, y + 1), (x + 1) / uv, (y + 1) / uv };
			genCell(a, b, c, d, vb, ib);
		}
	}
	// corner - bottom right
	{
		int y = size - 2, x = size - 2;
		Vertex a{ D3DXVECTOR3(x - m, getInnerHeight(offset, x, y, scale, size), y - m), getNormal(offset, x, y), x / uv, y / uv };
		Vertex b{ D3DXVECTOR3(x + 1 - m, getHeight(offset, x + 1, y, 1), y - m), getNormal(offset, x + 1, y), (x + 1) / uv, y / uv };
		Vertex c{ D3DXVECTOR3(x - m, getHeight(offset, x, y + 1, 1), y + 1 - m), getNormal(offset, x, y + 1), x / uv, (y + 1) / uv };
		Vertex d{ D3DXVECTOR3(x + 1 - m, getHeight(offset, x + 1, y + 1, 1), y + 1 - m), getNormal(offset, x + 1, y + 1), (x + 1) / uv, (y + 1) / uv };
		genCell(a, b, c, d, vb, ib);
	}
	// vertical - left
	{
		int x = 0;
		for (int y = 1; y < (size - 2); y++)
		{
			Vertex a{ D3DXVECTOR3(x - m, getHeight(offset, x, y, 1), y - m), getNormal(offset, x, y), x / uv, y / uv };
			Vertex b{ D3DXVECTOR3(x + 1 - m, getInnerHeight(offset, x + 1, y, scale, size), y - m), getNormal(offset, x + 1, y), (x + 1) / uv, y / uv };
			Vertex c{ D3DXVECTOR3(x - m, getHeight(offset, x, y + 1, 1), y + 1 - m), getNormal(offset, x, y + 1), x / uv, (y + 1) / uv };
			Vertex d{ D3DXVECTOR3(x + 1 - m, getInnerHeight(offset, x + 1, y + 1, scale, size), y + 1 - m), getNormal(offset, x + 1, y + 1), (x + 1) / uv, (y + 1) / uv };
			genCell(a, b, c, d, vb, ib);
		}
	}
	// corner - bottom left
	{
		int y = size - 2, x = 0;
		Vertex a{ D3DXVECTOR3(x - m, getHeight(offset, x, y, 1), y - m), getNormal(offset, x, y), x / uv, y / uv };
		Vertex b{ D3DXVECTOR3(x + 1 - m, getInnerHeight(offset, x + 1, y, scale, size), y - m), getNormal(offset, x + 1, y), (x + 1) / uv, y / uv };
		Vertex c{ D3DXVECTOR3(x - m, getHeight(offset, x, y + 1, 1), y + 1 - m), getNormal(offset, x, y + 1), x / uv, (y + 1) / uv };
		Vertex d{ D3DXVECTOR3(x + 1 - m, getHeight(offset, x + 1, y + 1, 1), y + 1 - m), getNormal(offset, x + 1, y + 1), (x + 1) / uv, (y + 1) / uv };
		genCell(a, b, c, d, vb, ib);
	}
	// horizontal - bottom
	{
		int y = size - 2;
		for (int x = 1; x < (size - 2); x++)
		{
			Vertex a{ D3DXVECTOR3(x - m, getInnerHeight(offset, x, y, scale, size), y - m), getNormal(offset, x, y), x / uv, y / uv };
			Vertex b{ D3DXVECTOR3(x + 1 - m, getInnerHeight(offset, x + 1, y, scale, size), y - m), getNormal(offset, x + 1, y), (x + 1) / uv, y / uv };
			Vertex c{ D3DXVECTOR3(x - m, getHeight(offset, x, y + 1, 1), y + 1 - m), getNormal(offset, x, y + 1), x / uv, (y + 1) / uv };
			Vertex d{ D3DXVECTOR3(x + 1 - m, getHeight(offset, x + 1, y + 1, 1), y + 1 - m), getNormal(offset, x + 1, y + 1), (x + 1) / uv, (y + 1) / uv };
			genCell(a, b, c, d, vb, ib);
		}
	}

	if (!mIndexBuffer[4])
	{
		mIndexCount[4] = static_cast<unsigned int>(ib.size());
		mIndexBuffer[4].reset(CreateIndexBuffer(mDevice, ib.data(), mIndexCount[4]));

		if (!mIndexBuffer[4])
			return false;
	}

	lod.mVertexCount[1] = static_cast<unsigned int>(vb.size());
	lod.mVertexBuffer[1].reset(CreateVertexBuffer(mDevice, vb.data(), sizeof(Vertex), lod.mVertexCount[1], vertexFVF));

	if (!lod.mVertexBuffer[1])
		return false;

	return true;
}

//*********************************************************************************************************************
