#pragma once
#include <d3dx9.h>

//*********************************************************************************************************************

class Camera
{
public:
	Camera();
	Camera(const D3DXVECTOR3 pos, const float pitch, const float yaw, const float roll);

	void rotate(const float dPitch, const float dYaw, const float dRoll);
	void moveUp(const float scale = 1.0f);
	void moveRight(const float scale = 1.0f);
	void moveForward(const float scale = 1.0f);

	void setView(IDirect3DDevice9* pDevice);
	void setOrientation(IDirect3DDevice9* pDevice);

private:
	float mPitch, mYaw, roll;
	D3DXVECTOR3 mPos;
	D3DXVECTOR3 mDir, mRight, mUp;
};

//*********************************************************************************************************************
