#pragma once
#include "d3dwrap.h"
#include <functional>
#include <string>

//*********************************************************************************************************************

enum class StatueRenderMode { Normal, Reflect, Refract };

//*********************************************************************************************************************

class Statue
{
public:
	Statue(IDirect3DDevice9* pDevice);

	bool init();
	void update(const float tick = 1.0f);
	void draw(StatueRenderMode mode, const D3DXVECTOR3& camPos);

private:
	bool loadObject(std::string filename, VertexBuffer& vertexbuffer, IndexBuffer& indexbuffer);

	IDirect3DDevice9* mDevice;
	VertexBuffer mVertexBuffer;
	IndexBuffer mIndexBuffer;
	Texture mTexture[2];
	Effect mEffect;
	Declaration mVertexDeclaration;
	int mIndexCount;
};

//*********************************************************************************************************************
