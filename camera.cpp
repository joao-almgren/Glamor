#include "camera.h"

//*********************************************************************************************************************

Camera::Camera()
	: mPos{0, 0, 0}
	, mPitch(0)
	, mYaw(0)
{
	rotate(0, 0);
}

//*********************************************************************************************************************

Camera::Camera(const D3DXVECTOR3 pos, const float pitch, const float yaw)
	: mPos(pos)
	, mPitch(pitch)
	, mYaw(yaw)
{
	rotate(0, 0);
}

//*********************************************************************************************************************

void Camera::rotate(const float dPitch, const float dYaw)
{
	mPitch += dPitch;
	mYaw += dYaw;

	const float HALF_PI = 3.14f * 0.5f;
	if (mPitch > HALF_PI)
		mPitch = HALF_PI;
	else if (mPitch < -HALF_PI)
		mPitch = -HALF_PI;

	D3DXMATRIX matY, matX, matRot;
	D3DXMatrixRotationY(&matY, mYaw);
	D3DXMatrixRotationX(&matX, mPitch);
	matRot = matY * matX;
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
	D3DXVECTOR3 yup(0, 1, 0);
	D3DXMatrixLookAtLH(&view, &mPos, &at, &yup);
	pDevice->SetTransform(D3DTS_VIEW, &view);
}

//*********************************************************************************************************************

D3DXVECTOR3 Camera::getPos()
{
	return mPos;
}

//*********************************************************************************************************************

D3DXVECTOR3 Camera::getDir()
{
	return mDir;
}

//*********************************************************************************************************************
