#pragma once
#include "d3dwrap.h"

//*********************************************************************************************************************

enum class PostRenderMode { Pass, Blur };

//*********************************************************************************************************************

class Post
{
public:
	Post(IDirect3DDevice9* pDevice);

	bool init();
	void draw(PostRenderMode mode, IDirect3DTexture9* pTexture);

private:
	IDirect3DDevice9* mDevice;
	VertexBuffer mVertexBuffer;
	Effect mEffect;
};

//*********************************************************************************************************************
