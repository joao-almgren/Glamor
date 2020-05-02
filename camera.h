#pragma once
#include <math.h>
#include <d3d9.h>
#include <d3dx9.h>

class Camera
{
public:
	Camera();
	Camera(const D3DXVECTOR3 pos, const float pitch, const float yaw, const float roll);

	void rotate(const float dPitch, const float dYaw, const float dRoll);
	void moveUp(const float scale = 1.0f);
	void moveRight(const float scale = 1.0f);
	void moveForward(const float scale = 1.0f);

	void setView(IDirect3DDevice9 * pDevice);
	void setViewOrientation(IDirect3DDevice9 * pDevice, D3DXVECTOR3 eye = D3DXVECTOR3(0, 0, 0));

private:
	float pitch, yaw, roll;
	D3DXVECTOR3 pos;
	D3DXVECTOR3 dir, right, up;
};
