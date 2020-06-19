#pragma once
#include "d3dwrap.h"
#include <functional>

//*********************************************************************************************************************

class Grass
{
public:
	Grass(IDirect3DDevice9* pDevice);

	bool init(std::function<float(float, float)> height, std::function<float(float, float)> angle);
	void update(const float tick = 1.0f);
	void draw();

private:
	bool loadObject();
	bool createInstances(std::function<float(float, float)> height, std::function<float(float, float)> angle);

	IDirect3DDevice9* mDevice;
	VertexBuffer mVertexBuffer;
	IndexBuffer mIndexBuffer;
	VertexBuffer mInstanceBuffer;
	Texture mTexture;
	Effect mEffect;
	Declaration mVertexDeclaration;
	int mIndexCount;
};

//*********************************************************************************************************************
