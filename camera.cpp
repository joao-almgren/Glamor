#include "camera.h"

//*********************************************************************************************************************

Camera::Camera()
	: mPos{0, 0, 0}
	, mPitch(0)
	, mYaw(0)
	, roll(0)
{
	rotate(0, 0, 0);
}

//*********************************************************************************************************************

Camera::Camera(const D3DXVECTOR3 pos, const float pitch, const float yaw, const float roll)
	: mPos(pos)
	, mPitch(pitch)
	, mYaw(yaw)
	, roll(roll)
{
	rotate(0, 0, 0);
}

//*********************************************************************************************************************

void Camera::rotate(const float dPitch, const float dYaw, const float dRoll)
{
	mPitch += dPitch;
	mYaw += dYaw;
	roll += dRoll;

	const float HALF_PI = 0.99f * (3.1415f / 2.0f);
	if (mPitch > HALF_PI)
		mPitch = HALF_PI;
	if (mPitch < -HALF_PI)
		mPitch = -HALF_PI;

	D3DXMATRIX matY, matX, matZ, matRot;
	D3DXMatrixRotationY(&matY, mYaw);
	D3DXMatrixRotationX(&matX, mPitch);
	D3DXMatrixRotationZ(&matZ, roll);
	matRot = matY * matX * matZ;
	mDir = D3DXVECTOR3(matRot._13, matRot._23, matRot._33);

	D3DXMATRIX view;
	D3DXVECTOR3 at(mPos + mDir);
	D3DXVECTOR3 yup(0, 1, 0);
	D3DXMatrixLookAtLH(&view, &mPos, &at, &yup);

	mRight = D3DXVECTOR3(view._11, view._21, view._31);
	mUp = D3DXVECTOR3(view._12, view._22, view._32);
	mDir = D3DXVECTOR3(view._13, view._23, view._33);
}

//*********************************************************************************************************************

void Camera::moveRight(const float scale)
{
	mPos += scale * mRight;
}

//*********************************************************************************************************************

void Camera::moveForward(const float scale)
{
	mPos += scale * mDir;
}

//*********************************************************************************************************************

void Camera::moveUp(const float scale)
{
	mPos += scale * mUp;
}

//*********************************************************************************************************************

void Camera::setView(IDirect3DDevice9* pDevice)
{
	D3DXMATRIX view;
	D3DXVECTOR3 at(mPos + mDir);
	D3DXMatrixLookAtLH(&view, &mPos, &at, &mUp);
	pDevice->SetTransform(D3DTS_VIEW, &view);
}

//*********************************************************************************************************************

void Camera::setOrientation(IDirect3DDevice9* pDevice)
{
	D3DXMATRIX view;
	D3DXVECTOR3 eye(0, 0, 0);
	D3DXMatrixLookAtLH(&view, &eye, &mDir, &mUp);
	pDevice->SetTransform(D3DTS_VIEW, &view);
}

//*********************************************************************************************************************
