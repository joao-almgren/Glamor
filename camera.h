#pragma once
#include <d3dx9.h>

//*********************************************************************************************************************

class Camera
{
public:
	Camera(IDirect3DDevice9* pDevice, const D3DXVECTOR3& pos, float pitch, float yaw);

	void rotate(float dPitch, float dYaw);
	void moveUp(float scale = 1.0f);
	void moveRight(float scale = 1.0f);
	void moveForward(float scale = 1.0f);
	D3DXVECTOR3 getPos();

	void setView();
	void setProjection();
	void setFrustum();

	bool pointInFrustum(const D3DXVECTOR3& point);
	bool cubeInFrustum(float xCenter, float yCenter, float zCenter, float radius);
	bool sphereInFrustum(const D3DXVECTOR3& point, float radius);
	bool cuboidInFrustum(float xCenter, float yCenter, float zCenter, float xSize, float ySize, float zSize);

private:
	IDirect3DDevice9* mDevice;
	float mPitch, mYaw;
	D3DXVECTOR3 mPos;
	D3DXVECTOR3 mDir, mRight, mUp;
	D3DXPLANE mFrustum[6];
};

//*********************************************************************************************************************
