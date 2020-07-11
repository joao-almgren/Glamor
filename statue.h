#pragma once
#include "d3dwrap.h"
#include <functional>
#include <string>

//*********************************************************************************************************************

enum class StatueRenderMode { Normal, Reflect, Simple };

//*********************************************************************************************************************

class Statue
{
public:
	Statue(IDirect3DDevice9* pDevice, IDirect3DTexture9* pShadowZ);

	bool init();
	void update(const float tick = 1.0f);
	void draw(StatueRenderMode mode, const D3DXVECTOR3& camPos, const D3DXMATRIX& matLightViewProj);

private:
	IDirect3DDevice9* mDevice;
	IDirect3DTexture9* mShadowZ;
	VertexBuffer mVertexBuffer;
	IndexBuffer mIndexBuffer;
	Texture mTexture[2];
	Effect mEffect;
	VertexDeclaration mVertexDeclaration;
	int mIndexCount;
};

//*********************************************************************************************************************
