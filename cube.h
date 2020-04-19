#pragma once
#include "d3dwrap.h"
#include "imesh.h"
#include <memory>

class Cube : public iMesh
{
public:
	Cube();
	~Cube() = default;

	bool init(IDirect3DDevice9* p3DDevice) override;
	void draw() override;

private:
	IDirect3DDevice9* pDevice;
	std::unique_ptr<IDirect3DVertexBuffer9, decltype(vertexDeleter)> pVertexBufferCube;
	std::unique_ptr<IDirect3DIndexBuffer9, decltype(indexDeleter)> pIndexBufferCube;
	std::unique_ptr<ID3DXEffect, decltype(effectDeleter)> pEffect;
	std::unique_ptr<IDirect3DTexture9, decltype(textureDeleter)> pTextureCube;
};
