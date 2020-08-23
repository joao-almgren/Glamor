#pragma once
#include "d3dwrap.h"
#include <string>

class Camera;

enum class FishRenderMode { Normal, Reflect };

class Fish
{
public:
	Fish(IDirect3DDevice9* pDevice, Camera* pCamera);

	bool init();
	void update(const float tick = 1.0f);
	void draw(FishRenderMode mode);

private:
	bool loadObject(std::string filename, VertexBuffer& vertexbuffer, IndexBuffer& indexbuffer);
	bool createInstances();

	IDirect3DDevice9* mDevice;
	Camera* mCamera;
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
