#pragma once
#include "d3dwrap.h"

//*********************************************************************************************************************

class Butterfly
{
public:
	Butterfly(IDirect3DDevice9* pDevice);

	bool init();
	void update(const float tick = 1.0f);
	void draw();

private:
	IDirect3DDevice9* mDevice;
	VertexBuffer mVertexBuffer;
	Texture mTexture;
	Effect mEffect;
	int mFlap, mFlapDir;
};

//*********************************************************************************************************************
