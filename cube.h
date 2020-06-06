#pragma once
#include "d3dwrap.h"

//*********************************************************************************************************************

class Cube
{
public:
	Cube(IDirect3DDevice9* pDevice);

	bool init();
	void update(const float tick = 1.0f);
	void draw();

	void setPos(const D3DXVECTOR3& pos);

private:
	IDirect3DDevice9* mDevice;
	VertexBuffer mVertexBuffer;
	IndexBuffer mIndexBuffer;
	Texture mTexture;
	Effect mEffect;
	float mAngle;
	D3DXVECTOR3 mPos;
};

//*********************************************************************************************************************
