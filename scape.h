#pragma once
#include "d3dwrap.h"
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

enum class ScapeRenderMode { Normal, Reflect };

//*********************************************************************************************************************

class Scape
{
public:
	Scape(IDirect3DDevice9* pDevice);

	bool init();
	void update(const float tick = 1.0f);
	void draw(const ScapeRenderMode mode, D3DXVECTOR3 camPos);

private:
	bool loadHeightmap(const int size, const float scale, const float sealevel);
	float getHeight(const int offset, const int x, const int y, const int scale);
	D3DXVECTOR3 getNormal(const int offset, const int x, const int y);

	unsigned int generateIndices(IndexBuffer& indexBuffer, const int size);
	bool generateVertices(Lod& lod, const int size, const int scale, const int offset);

	float getInnerHeight(int offset, int x, int y, int scale, int size);
	bool generateSkirt(Lod& lod, const int size, const int scale, const int offset);

	IDirect3DDevice9* mDevice;
	Texture mTexture[3];
	Effect mEffect;
	std::vector<float> mHeightmap;
	const unsigned int mHeightmapSize;
	std::vector<Chunk> mChunk;
	IndexBuffer mIndexBuffer[5];
	unsigned int mIndexCount[5];
};

//*********************************************************************************************************************
