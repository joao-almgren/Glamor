#pragma once
#include "d3dwrap.h"
#include <memory>

//*********************************************************************************************************************

class Skybox
{
public:
	Skybox(IDirect3DDevice9* pDevice);

	bool init();
	void update(const float tick = 1.0f);
	void draw();

	void setPos(const D3DXVECTOR3& pos);

private:
	IDirect3DDevice9* mDevice;
	VertexBuffer mVertexBuffer;
	Texture mTexture[5];
	D3DXVECTOR3 mPos;
};

//*********************************************************************************************************************
