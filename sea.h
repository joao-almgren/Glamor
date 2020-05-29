#pragma once
#include "d3dwrap.h"
#include <memory>

//*********************************************************************************************************************

class Sea
{
public:
	Sea(IDirect3DDevice9* pDevice, IDirect3DTexture9* pReflect, IDirect3DTexture9* pRefract, IDirect3DTexture9* pDepth);

	bool init();
	void update(const float tick = 1.0f);
	void draw(const D3DXMATRIX& matRTTProj, const D3DXVECTOR3& camPos);

private:
	IDirect3DDevice9* mDevice;
	IDirect3DTexture9* mReflect;
	IDirect3DTexture9* mRefract;
	IDirect3DTexture9* mDepth;
	VertexBuffer mVertexBuffer;
	Texture mTexture;
	Effect mEffect;
};

//*********************************************************************************************************************