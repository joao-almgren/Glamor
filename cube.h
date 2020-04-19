#pragma once
#include "d3dwrap.h"
#include "imesh.h"
#include <memory>

class Cube : public iMesh
{
public:
	Cube();
	~Cube() = default;

	bool init(IDirect3DDevice9* pDevice) override;
	void draw(IDirect3DDevice9* pDevice) override;

private:
	std::unique_ptr<IDirect3DVertexBuffer9, decltype(vertexDeleter)> pVertexBufferCube;
	std::unique_ptr<IDirect3DIndexBuffer9, decltype(indexDeleter)> pIndexBufferCube;
	std::unique_ptr<ID3DXEffect, decltype(effectDeleter)> pEffect;
	std::unique_ptr<IDirect3DTexture9, decltype(textureDeleter)> pTextureCube;
};
