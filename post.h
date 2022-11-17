#pragma once
#include "d3dwrap.h"
#include <vector>
#include <memory>

enum class PostRenderMode { PASS, BLUR, ADD, DOWN };

class Post
{
public:
	explicit Post(std::shared_ptr<IDirect3DDevice9> pDevice) noexcept;

	bool init();
	void draw(PostRenderMode mode, const std::vector<IDirect3DTexture9*>& pTexture) const;

private:
	std::shared_ptr<IDirect3DDevice9> mDevice;
	VertexBuffer mVertexBuffer;
	Effect mEffect;
	VertexDeclaration mVertexDeclaration;
};
