#pragma once
#include "d3dwrap.h"
#include "imesh.h"
#include <memory>

//*********************************************************************************************************************

class Skybox : public iMesh
{
public:
	Skybox(IDirect3DDevice9* pDevice);
	~Skybox() = default;

	bool init() override;
	void update(const float tick) override;
	void draw() override;

private:
	std::unique_ptr<IDirect3DVertexBuffer9, decltype(vertexDeleter)> pVertexBufferSky;
	std::unique_ptr<IDirect3DTexture9, decltype(textureDeleter)> pTextureSky[5];
	float angle;
};

//*********************************************************************************************************************
