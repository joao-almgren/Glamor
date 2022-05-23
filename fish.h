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
	void update(float tick = 1.0f);
	void draw(FishRenderMode mode) const;

private:
	bool loadObject(const std::string& filename, VertexBuffer& vertexbuffer, IndexBuffer& indexbuffer);
	bool createInstances();

	IDirect3DDevice9* mDevice;
	VertexBuffer mVertexBuffer;
	IndexBuffer mIndexBuffer;
	VertexBuffer mInstanceBuffer;
	Texture mTexture;
	Effect mEffect;
	VertexDeclaration mVertexDeclaration;
	int mIndexCount;
	int mAngle;
	D3DXVECTOR4 mSphere;
};
