#pragma once
#include "d3dwrap.h"
#include "imesh.h"
#include <memory>

//*********************************************************************************************************************

class Cube : public iMesh
{
public:
	Cube(IDirect3DDevice9* pDevice);
	~Cube() = default;

	bool init() override;
	void update(const float tick = 1.0f) override;
	void draw() override;

private:
	std::unique_ptr<IDirect3DVertexBuffer9, decltype(vertexDeleter)> pVertexBuffer;
	std::unique_ptr<IDirect3DIndexBuffer9, decltype(indexDeleter)> pIndexBuffer;
	std::unique_ptr<IDirect3DTexture9, decltype(textureDeleter)> pTexture;
	std::unique_ptr<ID3DXEffect, decltype(effectDeleter)> pEffect;
	float angle;
};

//*********************************************************************************************************************
