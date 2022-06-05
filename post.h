#pragma once
#include "d3dwrap.h"
#include <vector>

enum class PostRenderMode { PASS, BLUR, ADD, DOWN };

class Post
{
public:
	explicit Post(IDirect3DDevice9* pDevice);

	bool init();
	void draw(const PostRenderMode mode, const std::vector<IDirect3DTexture9*>& pTexture) const;

private:
	IDirect3DDevice9* mDevice;
	VertexBuffer mVertexBuffer;
	Effect mEffect;
	VertexDeclaration mVertexDeclaration;
};
