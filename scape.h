#pragma once
#include "d3dwrap.h"
#include "imesh.h"
#include <memory>
#include <vector>

//*********************************************************************************************************************

class Scape : public iMesh
{
public:
	Scape(IDirect3DDevice9* pDevice);
	~Scape() = default;

	bool init() override;
	void update(const float tick) override;
	void draw() override;

private:
	std::unique_ptr<IDirect3DVertexBuffer9, decltype(vertexDeleter)> pVertexBuffer;
	std::unique_ptr<IDirect3DIndexBuffer9, decltype(indexDeleter)> pIndexBuffer;
	std::unique_ptr<IDirect3DTexture9, decltype(textureDeleter)> pTexture;
	std::unique_ptr<ID3DXEffect, decltype(effectDeleter)> pEffect;
	int vertexCount, indexCount;
	std::vector<float> height;
};

//*********************************************************************************************************************
