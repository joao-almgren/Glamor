#pragma once
#include "d3dwrap.h"

//*********************************************************************************************************************

class Rock
{
public:
	Rock(IDirect3DDevice9* pDevice);

	bool init();
	void update(const float tick = 1.0f);
	void draw();

private:
	bool loadObject();
	bool createInstances();

	IDirect3DDevice9* mDevice;
	VertexBuffer mVertexBuffer;
	IndexBuffer mIndexBuffer;
	VertexBuffer mInstanceBuffer;
	Texture mTexture;
	Effect mEffect;
	Declaration mVertexDeclaration;
	int vertexCount;
};

//*********************************************************************************************************************
