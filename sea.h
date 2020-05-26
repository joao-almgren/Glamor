#pragma once
#include "d3dwrap.h"
#include <memory>

//*********************************************************************************************************************

class Sea
{
public:
	Sea(IDirect3DDevice9* pDevice, IDirect3DTexture9* pReflect);

	bool init();
	void update(const float tick = 1.0f);
	void draw(const D3DXMATRIX& matReflectProj);

private:
	IDirect3DDevice9* mDevice;
	IDirect3DTexture9* mReflect;
	VertexBuffer mVertexBuffer;
	Texture mTexture;
	Effect mEffect;
};

//*********************************************************************************************************************