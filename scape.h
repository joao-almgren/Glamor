#pragma once
#include "d3dwrap.h"
#include "imesh.h"
#include <memory>
#include <vector>

//*********************************************************************************************************************

struct Lod
{
	std::unique_ptr<IDirect3DVertexBuffer9, decltype(vertexDeleter)> mVertexBuffer[2];
	unsigned int mVertexCount[2];

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
	float getHeight(const int offset, const int x, const int y, const int scale);
	D3DXVECTOR3 getNormal(const int offset, const int x, const int y);

	unsigned int generateIndices(IndexBuffer& indexBuffer, const int size);
	bool generateVertices(Lod& lod, const int size, const int scale, const int offset);

	bool generateSkirt(Lod& lod, const int size, const int scale, const int offset);

	Texture mTexture[3];
	Effect mEffect;
	std::vector<float> mHeightmap;
	const unsigned int mHeightmapSize;
	std::vector<Chunk> mChunk;
	IndexBuffer mIndexBuffer[5];
	unsigned int mIndexCount[5];
	D3DXVECTOR3 mPos;
};

//*********************************************************************************************************************