#pragma once
#include "d3dwrap.h"
#include "imesh.h"
#include <memory>
#include <vector>

//*********************************************************************************************************************

struct Lod
{
	std::unique_ptr<IDirect3DVertexBuffer9, decltype(vertexDeleter)> pVertexBuffer;
	std::unique_ptr<IDirect3DIndexBuffer9, decltype(indexDeleter)> pIndexBuffer;
	int vertexCount, indexCount;

	Lod()
		: pVertexBuffer(nullptr, vertexDeleter)
		, pIndexBuffer(nullptr, indexDeleter)
		, vertexCount(0)
		, indexCount(0)
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
	bool generateIndices(Lod& lod, const int size);
	bool generateVertices(Lod& lod, const int size, const float scale);

	std::unique_ptr<IDirect3DTexture9, decltype(textureDeleter)> mTexture[2];
	std::unique_ptr<ID3DXEffect, decltype(effectDeleter)> mEffect;
	std::vector<float> mHeight;
	int mHeightSize;
	std::vector<Lod> mLod;

	int mLodIndex;
};

//*********************************************************************************************************************
