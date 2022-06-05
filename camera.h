#pragma once
#include <d3dx9.h>

class Camera
{
public:
	Camera(IDirect3DDevice9* pDevice, const D3DXVECTOR3& pos, float pitch, float yaw);

	void rotate(const float dPitch, const float dYaw);
	void moveUp(const float scale = 1.0f);
	void moveRight(const float scale = 1.0f);
	void moveForward(const float scale = 1.0f);

	[[nodiscard]] D3DXVECTOR3 getPos() const noexcept;
	[[nodiscard]] D3DXVECTOR3 getDir() const noexcept;

	void setView() const;
	void setProjection() const;
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
