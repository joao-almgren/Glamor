#pragma once
#include <d3dx9.h>

//*********************************************************************************************************************

class Camera
{
public:
	Camera();
	Camera(const D3DXVECTOR3 pos, const float pitch, const float yaw);

	void rotate(const float dPitch, const float dYaw);
	void moveUp(const float scale = 1.0f);
	void moveRight(const float scale = 1.0f);
	void moveForward(const float scale = 1.0f);

	void setView(IDirect3DDevice9* pDevice);
	D3DXVECTOR3 getPos();
	D3DXVECTOR3 getDir();

private:
	float mPitch, mYaw;
	D3DXVECTOR3 mPos;
	D3DXVECTOR3 mDir, mRight, mUp;
};

//*********************************************************************************************************************
