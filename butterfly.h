#pragma once
#include "d3dwrap.h"
#include <memory>

class Camera;

class Butterfly
{
public:
	Butterfly(std::shared_ptr<IDirect3DDevice9> pDevice, Camera* pCamera, IDirect3DTexture9* pShadowZ);

	bool init();
	void update([[maybe_unused]] float tick = 1.0f) noexcept;
	void draw(const D3DXMATRIX& matLightViewProj) const;

private:
	std::shared_ptr<IDirect3DDevice9> mDevice;
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
