#include "camera.h"

//*********************************************************************************************************************

Camera::Camera()
	: pos{0, 0, 0}
	, pitch(0)
	, yaw(0)
	, roll(0)
{
	rotate(0, 0, 0);
}

//*********************************************************************************************************************

Camera::Camera(const D3DXVECTOR3 pos, const float pitch, const float yaw, const float roll)
	: pos(pos)
	, pitch(pitch)
	, yaw(yaw)
	, roll(roll)
{
	rotate(0, 0, 0);
}

//*********************************************************************************************************************

void Camera::rotate(const float dPitch, const float dYaw, const float dRoll)
{
	pitch += dPitch;
	yaw += dYaw;
	roll += dRoll;

	const float HALF_PI = 0.99f * (3.1415f / 2.0f);
	if (pitch > HALF_PI)
		pitch = HALF_PI;
	if (pitch < -HALF_PI)
		pitch = -HALF_PI;

	D3DXMATRIX matY, matX, matZ, matRot;
	D3DXMatrixRotationY(&matY, yaw);
	D3DXMatrixRotationX(&matX, pitch);
	D3DXMatrixRotationZ(&matZ, roll);
	matRot = matY * matX * matZ;
	dir = D3DXVECTOR3(matRot._13, matRot._23, matRot._33);

	D3DXMATRIX view;
	D3DXVECTOR3 at(pos + dir);
	D3DXVECTOR3 yup(0, 1, 0);
	D3DXMatrixLookAtLH(&view, &pos, &at, &yup);

	right = D3DXVECTOR3(view._11, view._21, view._31);
	up = D3DXVECTOR3(view._12, view._22, view._32);
	dir = D3DXVECTOR3(view._13, view._23, view._33);
}

//*********************************************************************************************************************

void Camera::moveRight(const float scale)
{
	pos += scale * right;
}

//*********************************************************************************************************************

void Camera::moveForward(const float scale)
{
	pos += scale * dir;
}

//*********************************************************************************************************************

void Camera::moveUp(const float scale)
{
	pos += scale * up;
}

//*********************************************************************************************************************

void Camera::setView(IDirect3DDevice9* pDevice)
{
	D3DXMATRIX view;
	D3DXVECTOR3 at(pos + dir);
	D3DXMatrixLookAtLH(&view, &pos, &at, &up);
	pDevice->SetTransform(D3DTS_VIEW, &view);
}

//*********************************************************************************************************************

void Camera::setOrientation(IDirect3DDevice9* pDevice)
{
	D3DXMATRIX view;
	D3DXVECTOR3 eye(0, 0, 0);
	D3DXMatrixLookAtLH(&view, &eye, &dir, &up);
	pDevice->SetTransform(D3DTS_VIEW, &view);
}

//*********************************************************************************************************************
