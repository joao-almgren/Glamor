#pragma once
#include "d3dwrap.h"
#include "imesh.h"
#include <memory>

//*********************************************************************************************************************

class Sea : public iMesh
{
public:
	Sea(IDirect3DDevice9* pDevice);
	~Sea() = default;

	bool init() override;
	void update(const float tick = 1.0f) override;
	void draw() override;

private:
	VertexBuffer mVertexBuffer;
	Texture mTexture;
};

//*********************************************************************************************************************
