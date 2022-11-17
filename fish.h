#pragma once
#include "d3dwrap.h"
#include <memory>

class Camera;

enum class FishRenderMode { NORMAL, REFLECT };

class Fish
{
public:
	explicit Fish(std::shared_ptr<IDirect3DDevice9> pDevice);

	bool init();
	void update([[maybe_unused]] float tick = 1.0f) noexcept;
	void draw(FishRenderMode mode) const;

private:
	bool createInstances();

	std::shared_ptr<IDirect3DDevice9> mDevice;
	VertexBuffer mVertexBuffer;
	IndexBuffer mIndexBuffer;
	VertexBuffer mInstanceBuffer;
	Texture mTexture;
	Effect mEffect;
	VertexDeclaration mVertexDeclaration;
	int mIndexCount;
	float mAngle;
	D3DXVECTOR4 mSphere;
};
