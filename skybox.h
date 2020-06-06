#pragma once
#include "d3dwrap.h"

//*********************************************************************************************************************

class Skybox
{
public:
	Skybox(IDirect3DDevice9* pDevice);

	bool init();
	void update(const float tick = 1.0f);
	void draw(const D3DXVECTOR3& camPos);

private:
	IDirect3DDevice9* mDevice;
	VertexBuffer mVertexBuffer;
	Texture mTexture[5];
};

//*********************************************************************************************************************
