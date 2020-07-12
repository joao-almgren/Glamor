#pragma once
#include "d3dwrap.h"

//*********************************************************************************************************************

class Camera;

//*********************************************************************************************************************

enum class StatueRenderMode { Normal, Reflect, Simple, Caster };

//*********************************************************************************************************************

class Statue
{
public:
	Statue(IDirect3DDevice9* pDevice, Camera* pCamera, IDirect3DTexture9* pShadowZ);

	bool init();
	void update(const float tick = 1.0f);
	void draw(StatueRenderMode mode, const D3DXMATRIX& matLightViewProj);

private:
	IDirect3DDevice9* mDevice;
	Camera* mCamera;
	IDirect3DTexture9* mShadowZ;
	VertexBuffer mVertexBuffer;
	IndexBuffer mIndexBuffer;
	Texture mTexture[2];
	Effect mEffect;
	VertexDeclaration mVertexDeclaration;
	int mIndexCount;
	D3DXVECTOR4 mSphere;
};

//*********************************************************************************************************************
