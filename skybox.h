#pragma once
#include "d3dwrap.h"
#include <memory>

class Skybox
{
public:
	Skybox();

	bool init(IDirect3DDevice9* pDevice);
	void draw(IDirect3DDevice9* pDevice);

private:
	std::unique_ptr<IDirect3DVertexBuffer9, decltype(vertexDeleter)> pVertexBufferSky;
	std::unique_ptr<IDirect3DTexture9, decltype(textureDeleter)> pTextureSky[5];
};
