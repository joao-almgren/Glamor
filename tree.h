#pragma once
#include "d3dwrap.h"
#include <functional>

//*********************************************************************************************************************

enum class TreeRenderMode { Blend, Plain };

//*********************************************************************************************************************

class Tree
{
public:
	Tree(IDirect3DDevice9* pDevice);

	bool init(std::function<float(float, float)> height, std::function<float(float, float)> angle);
	void update(const float tick = 1.0f);
	void draw(TreeRenderMode mode);

private:
	bool loadObject(std::string filename, VertexBuffer& vertexbuffer, IndexBuffer& indexbuffer);
	bool createInstances(std::function<float(float, float)> height, std::function<float(float, float)> angle);

	IDirect3DDevice9* mDevice;
	VertexBuffer mVertexBuffer[2];
	IndexBuffer mIndexBuffer[2];
	VertexBuffer mInstanceBuffer;
	Texture mTexture[2];
	Effect mEffect;
	Declaration mVertexDeclaration;
	int mIndexCount;
};

//*********************************************************************************************************************
