#pragma once
#include "d3dwrap.h"
#include <functional>
#include <string>

//*********************************************************************************************************************

struct TreeLod
{
	VertexBuffer mVertexBuffer[2];
	IndexBuffer mIndexBuffer[2];
	int mIndexCount[2];
	VertexBuffer mInstanceBuffer;
	int mInstanceCount;

	TreeLod()
		: mVertexBuffer{ MakeVertexBuffer(), MakeVertexBuffer() }
		, mIndexBuffer{ MakeIndexBuffer(), MakeIndexBuffer() }
		, mIndexCount{ 0, 0 }
		, mInstanceBuffer{ MakeVertexBuffer() }
		, mInstanceCount{ 0 }
	{
	}
};

//*********************************************************************************************************************

enum class TreeRenderMode { Blend, Plain };

//*********************************************************************************************************************

class Tree
{
public:
	Tree(IDirect3DDevice9* pDevice, IDirect3DTexture9* pShadowZ);

	bool init(std::function<float(float, float)> height, std::function<float(float, float)> angle);
	void update(const float tick = 1.0f);
	void draw(TreeRenderMode mode, const D3DXVECTOR3& camPos, const D3DXMATRIX& matLightViewProj);

private:
	bool loadObject(std::string filename, VertexBuffer& vertexbuffer, IndexBuffer& indexbuffer, int& indexCount);
	bool createInstances(std::function<float(float, float)> height, std::function<float(float, float)> angle);

	IDirect3DDevice9* mDevice;
	IDirect3DTexture9* mShadowZ;
	TreeLod mLod[3];
	Texture mTexture[3];
	Effect mEffect;
	VertexDeclaration mVertexDeclaration;
};

//*********************************************************************************************************************
