#pragma once
#include "d3dwrap.h"

//*********************************************************************************************************************

enum class SeaRenderMode { Normal, Plain, Underwater };

//*********************************************************************************************************************

class Sea
{
public:
	Sea(IDirect3DDevice9* pDevice, IDirect3DTexture9* pReflect, IDirect3DTexture9* pRefract, IDirect3DTexture9* pRefractZ, IDirect3DTexture9* pSurfaceZ, IDirect3DTexture9* pShadowZ);

	bool init();
	void update(const float tick = 1.0f);
	void draw(SeaRenderMode mode, const D3DXVECTOR3& camPos, const D3DXMATRIX& matRTTProj, const D3DXMATRIX& matLightViewProj);

private:
	IDirect3DDevice9* mDevice;
	IDirect3DTexture9* mReflect;
	IDirect3DTexture9* mRefract;
	IDirect3DTexture9* mRefractZ;
	IDirect3DTexture9* mSurfaceZ;
	IDirect3DTexture9* mShadowZ;
	VertexBuffer mVertexBuffer;
	Texture mTexture[2];
	Effect mEffect;
	VertexDeclaration mVertexDeclaration;
	float mWave;
};

//*********************************************************************************************************************
