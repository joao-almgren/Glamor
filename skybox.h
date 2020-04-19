#pragma once
#include "d3dwrap.h"
#include "imesh.h"
#include <memory>

class Skybox : public iMesh
{
public:
	Skybox();
	~Skybox() = default;

	bool init(IDirect3DDevice9* pDevice) override;
	void draw(IDirect3DDevice9* pDevice) override;

private:
	std::unique_ptr<IDirect3DVertexBuffer9, decltype(vertexDeleter)> pVertexBufferSky;
	std::unique_ptr<IDirect3DTexture9, decltype(textureDeleter)> pTextureSky[5];
};
