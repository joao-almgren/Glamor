#pragma once
#include "d3dwrap.h"
#include "imesh.h"
#include <memory>
#include <vector>

//*********************************************************************************************************************

struct Lod
{
	std::unique_ptr<IDirect3DVertexBuffer9, decltype(vertexDeleter)> mVertexBuffer[2];
	int mVertexCount[2];

	Lod()
		: mVertexBuffer{ { nullptr, vertexDeleter }, { nullptr, vertexDeleter } }
		, mVertexCount{ 0, 0 }
	{
	}
};

//*********************************************************************************************************************

struct Chunk
{
	std::vector<Lod> mLod;
	float mPosX, mPosY;

	Chunk()
		: mLod(4)
		, mPosX(0.0f)
		, mPosY(0.0f)
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

	void setPos(const D3DXVECTOR3& pos);

private:
	bool loadHeightmap(const int size, const float scale);
	int generateIndices(IndexBuffer& pIndexBuffer, const int size);
	bool generateVertices(Lod& lod, const int size, const int scale, const int offset);
	float getHeight(const int offset, const int x, const int y, const int scale);
	D3DXVECTOR3 getNormal(const int offset, const int x, const int y);

	Texture mTexture[3];
	Effect mEffect;
	std::vector<float> mHeightmap;
	const int mHeightmapSize;
	std::vector<Chunk> mChunk;
	IndexBuffer mIndexBuffer[4];
	int mIndexCount[4];
	D3DXVECTOR3 mPos;
};

//*********************************************************************************************************************
