#pragma once
#include "d3dwrap.h"
#include <vector>

//*********************************************************************************************************************

enum class PostRenderMode { Pass, Blur, Add, Down };

//*********************************************************************************************************************

class Post
{
public:
	Post(IDirect3DDevice9* pDevice);

	bool init();
	void draw(PostRenderMode mode, const std::vector<IDirect3DTexture9*>& pTexture);

private:
	IDirect3DDevice9* mDevice;
	VertexBuffer mVertexBuffer;
	Effect mEffect;
	Declaration mVertexDeclaration;
};

//*********************************************************************************************************************
