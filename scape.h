#pragma once
#include "d3dwrap.h"
#include "imesh.h"
#include <memory>
#include <vector>

//*********************************************************************************************************************

struct Lod
{
	std::unique_ptr<IDirect3DVertexBuffer9, decltype(vertexDeleter)> pVertexBuffer;
	int vertexCount;

	Lod()
		: pVertexBuffer(nullptr, vertexDeleter)
		, vertexCount(0)
	{
	}
};

//*********************************************************************************************************************

struct Chunk
{
	std::vector<Lod> mLod;
	int mapX, mapY;

	Chunk()
		: mLod(4)
		, mapX(0)
		, mapY(0)
	{
	}
};

//*********************************************************************************************************************

class Scape : public iMesh
{
public:
	Scape(IDirect3DDevice9* pDevice);
	~Scape() = default;

	bool init() override;
	void update(const float tick = 1.0f) override;
	void draw() override;

private:
	bool loadHeightmap(const int size, const float scale);
	int generateIndices(IndexBuffer& pIndexBuffer, const int size);
	bool generateVertices(Lod& lod, const int size, const float scale);

	Texture mTexture[2];
	Effect mEffect;
	std::vector<float> mHeight;
	int mHeightSize;
	std::vector<Chunk> mChunk;
	int mLodIndex;
	IndexBuffer mIndexBuffer[4];
	int mIndexCount[4];
};

//*********************************************************************************************************************
