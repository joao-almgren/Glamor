#pragma once
#include "d3dwrap.h"
#include <functional>
#include <string>

//*********************************************************************************************************************

class Fish
{
public:
	Fish(IDirect3DDevice9* pDevice);

	bool init();
	void update(const float tick = 1.0f);
	void draw();

private:
	bool loadObject(std::string filename, VertexBuffer& vertexbuffer, IndexBuffer& indexbuffer);
	bool createInstances();

	IDirect3DDevice9* mDevice;
	VertexBuffer mVertexBuffer;
	IndexBuffer mIndexBuffer;
	VertexBuffer mInstanceBuffer;
	Texture mTexture;
	Effect mEffect;
	Declaration mVertexDeclaration;
	int mIndexCount;
	int mAngle;
};

//*********************************************************************************************************************
