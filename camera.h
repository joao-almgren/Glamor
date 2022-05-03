#pragma once
#include <d3dx9.h>

class Camera
{
public:
	Camera(IDirect3DDevice9* pDevice, const D3DXVECTOR3& pos, float pitch, float yaw);

	void rotate(float dPitch, float dYaw);
	void moveUp(float scale = 1.0f);
	void moveRight(float scale = 1.0f);
	void moveForward(float scale = 1.0f);

	[[nodiscard]] D3DXVECTOR3 getPos() const;
	[[nodiscard]] D3DXVECTOR3 getDir() const;

	void setView();
	void setProjection();
	void setFrustum();

	[[nodiscard]] bool isPointInFrustum(const D3DXVECTOR3& point) const;
	[[nodiscard]] bool isCubeInFrustum(float xCenter, float yCenter, float zCenter, float radius) const;
	[[nodiscard]] bool isSphereInFrustum(const D3DXVECTOR3& point, float radius) const;
	[[nodiscard]] bool isCuboidInFrustum(float xCenter, float yCenter, float zCenter, float xSize, float ySize, float zSize) const;

private:
	IDirect3DDevice9* mDevice;
	float mPitch, mYaw;
	D3DXVECTOR3 mPos;
	D3DXVECTOR3 mDir, mRight, mUp;
	D3DXPLANE mFrustum[6];
};
