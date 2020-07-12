#pragma once
#include "d3dwrap.h"

//*********************************************************************************************************************

class Camera;

//*********************************************************************************************************************

class Butterfly
{
public:
	Butterfly(IDirect3DDevice9* pDevice, Camera* pCamera);

	bool init();
	void update(const float tick = 1.0f);
	void draw();

private:
	IDirect3DDevice9* mDevice;
	Camera* mCamera;
	VertexBuffer mVertexBuffer;
	Texture mTexture;
	Effect mEffect;
	VertexDeclaration mVertexDeclaration;
	D3DXVECTOR3 mPos;
	float mFlap, mFlapDir, mFlapPower;
	float mRoll, mRollDir, mPitch, mPitchDir, mYaw;
	float mAngle;
};

//*********************************************************************************************************************
