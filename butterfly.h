#pragma once
#include "d3dwrap.h"

class Camera;

class Butterfly
{
public:
	Butterfly(IDirect3DDevice9* pDevice, Camera* pCamera, IDirect3DTexture9* pShadowZ);

	bool init();
	void update(float tick = 1.0f) noexcept;
	void draw(const D3DXMATRIX& matLightViewProj) const;

private:
	IDirect3DDevice9* mDevice;
	Camera* mCamera;
	IDirect3DTexture9* mShadowZ;
	VertexBuffer mVertexBuffer;
	Texture mTexture;
	Effect mEffect;
	VertexDeclaration mVertexDeclaration;
	D3DXVECTOR3 mPos;
	float mFlap, mFlapDir, mFlapPower;
	float mRoll, mRollDir, mPitch, mPitchDir, mYaw;
	float mAngle;
};
