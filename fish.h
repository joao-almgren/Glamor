#pragma once
#include "d3dwrap.h"
#include <string>

class Camera;

enum class FishRenderMode { NORMAL, REFLECT };

class Fish
{
public:
	explicit Fish(IDirect3DDevice9* pDevice);

	bool init();
	void update([[maybe_unused]] float tick = 1.0f) noexcept;
	void draw(const FishRenderMode mode) const;

private:
	bool createInstances();

	IDirect3DDevice9* mDevice;
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
