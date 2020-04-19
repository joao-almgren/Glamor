#pragma once
#include "d3dwrap.h"
#include "imesh.h"
#include <memory>

class Skybox : public iMesh
{
public:
	Skybox();
	~Skybox() = default;

	bool init(IDirect3DDevice9* p3DDevice) override;
	void draw() override;

private:
	IDirect3DDevice9* pDevice;
	std::unique_ptr<IDirect3DVertexBuffer9, decltype(vertexDeleter)> pVertexBufferSky;
	std::unique_ptr<IDirect3DTexture9, decltype(textureDeleter)> pTextureSky[5];
};
