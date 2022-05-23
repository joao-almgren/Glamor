#include "camera.h"
#include "constants.h"

Camera::Camera(IDirect3DDevice9* pDevice, const D3DXVECTOR3& pos, const float pitch, const float yaw)
	: mDevice{ pDevice }
	, mPitch{ pitch }
	, mYaw{ yaw }
	, mPos{ pos }
{
	rotate(0, 0);
}

void Camera::rotate(const float dPitch, const float dYaw)
{
	mPitch += dPitch;
	mYaw += dYaw;

	constexpr float HALF_PI = 3.14f * 0.5f;
	if (mPitch > HALF_PI)
		mPitch = HALF_PI;
	else if (mPitch < -HALF_PI)
		mPitch = -HALF_PI;

	D3DXMATRIX matY, matX;
	D3DXMatrixRotationY(&matY, mYaw);
	D3DXMatrixRotationX(&matX, mPitch);
	const D3DXMATRIX matRot = matY * matX;
	mDir = D3DXVECTOR3(matRot._13, matRot._23, matRot._33);

	D3DXMATRIX view;
	const D3DXVECTOR3 at(mPos + mDir);
	const D3DXVECTOR3 yup(0, 1, 0);
	D3DXMatrixLookAtLH(&view, &mPos, &at, &yup);

	mRight = D3DXVECTOR3(view._11, view._21, view._31);
	mUp = D3DXVECTOR3(view._12, view._22, view._32);
	mDir = D3DXVECTOR3(view._13, view._23, view._33);
}

void Camera::moveRight(const float scale)
{
	mPos += scale * mRight;
}

void Camera::moveForward(const float scale)
{
	mPos += scale * mDir;
}

void Camera::moveUp(const float scale)
{
	mPos += scale * mUp;
}

void Camera::setView() const
{
	D3DXMATRIX view;
	const D3DXVECTOR3 at(mPos + mDir);
	const D3DXVECTOR3 yup(0, 1, 0);
	D3DXMatrixLookAtLH(&view, &mPos, &at, &yup);
	mDevice->SetTransform(D3DTS_VIEW, &view);
}

D3DXVECTOR3 Camera::getPos() const
{
	return mPos;
}

D3DXVECTOR3 Camera::getDir() const
{
	return mDir;
}

void Camera::setProjection() const
{
	D3DXMATRIX matProjection;
	D3DXMatrixPerspectiveFovLH
	(
		&matProjection,
		D3DXToRadian(FOV),
		static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT),
		NEAR_PLANE,
		FAR_PLANE
	);
	mDevice->SetTransform(D3DTS_PROJECTION, &matProjection);
}

void Camera::setFrustum()
{
	D3DXMATRIX matProjection;
	mDevice->GetTransform(D3DTS_PROJECTION, &matProjection);

	D3DXMATRIX matView;
	mDevice->GetTransform(D3DTS_VIEW, &matView);

	const D3DXMATRIX matFrustum = matView * matProjection;

	// Near plane
	mFrustum[0].a = matFrustum._14 + matFrustum._13;
	mFrustum[0].b = matFrustum._24 + matFrustum._23;
	mFrustum[0].c = matFrustum._34 + matFrustum._33;
	mFrustum[0].d = matFrustum._44 + matFrustum._43;
	D3DXPlaneNormalize(&mFrustum[0], &mFrustum[0]);

	// Far plane
	mFrustum[1].a = matFrustum._14 - matFrustum._13;
	mFrustum[1].b = matFrustum._24 - matFrustum._23;
	mFrustum[1].c = matFrustum._34 - matFrustum._33;
	mFrustum[1].d = matFrustum._44 - matFrustum._43;
	D3DXPlaneNormalize(&mFrustum[1], &mFrustum[1]);

	// Left plane
	mFrustum[2].a = matFrustum._14 + matFrustum._11;
	mFrustum[2].b = matFrustum._24 + matFrustum._21;
	mFrustum[2].c = matFrustum._34 + matFrustum._31;
	mFrustum[2].d = matFrustum._44 + matFrustum._41;
	D3DXPlaneNormalize(&mFrustum[2], &mFrustum[2]);

	// Right plane
	mFrustum[3].a = matFrustum._14 - matFrustum._11;
	mFrustum[3].b = matFrustum._24 - matFrustum._21;
	mFrustum[3].c = matFrustum._34 - matFrustum._31;
	mFrustum[3].d = matFrustum._44 - matFrustum._41;
	D3DXPlaneNormalize(&mFrustum[3], &mFrustum[3]);

	// Top plane
	mFrustum[4].a = matFrustum._14 - matFrustum._12;
	mFrustum[4].b = matFrustum._24 - matFrustum._22;
	mFrustum[4].c = matFrustum._34 - matFrustum._32;
	mFrustum[4].d = matFrustum._44 - matFrustum._42;
	D3DXPlaneNormalize(&mFrustum[4], &mFrustum[4]);

	// Bottom plane
	mFrustum[5].a = matFrustum._14 + matFrustum._12;
	mFrustum[5].b = matFrustum._24 + matFrustum._22;
	mFrustum[5].c = matFrustum._34 + matFrustum._32;
	mFrustum[5].d = matFrustum._44 + matFrustum._42;
	D3DXPlaneNormalize(&mFrustum[5], &mFrustum[5]);
}

bool Camera::isPointInFrustum(const D3DXVECTOR3& point) const
{
	for (const auto& plane : mFrustum)
		if (D3DXPlaneDotCoord(&plane, &point) < 0.0f)
			return false;

	return true;
}

bool Camera::isCubeInFrustum(const float xCenter, const float yCenter, const float zCenter, const float radius) const
{
	for (const auto& plane : mFrustum)
	{
		D3DXVECTOR3 a((xCenter - radius), (yCenter - radius), (zCenter - radius));
		if (D3DXPlaneDotCoord(&plane, &a) >= 0.0f)
			continue;

		D3DXVECTOR3 b((xCenter + radius), (yCenter - radius), (zCenter - radius));
		if (D3DXPlaneDotCoord(&plane, &b) >= 0.0f)
			continue;

		D3DXVECTOR3 c((xCenter - radius), (yCenter + radius), (zCenter - radius));
		if (D3DXPlaneDotCoord(&plane, &c) >= 0.0f)
			continue;

		D3DXVECTOR3 d((xCenter + radius), (yCenter + radius), (zCenter - radius));
		if (D3DXPlaneDotCoord(&plane, &d) >= 0.0f)
			continue;

		D3DXVECTOR3 e((xCenter - radius), (yCenter - radius), (zCenter + radius));
		if (D3DXPlaneDotCoord(&plane, &e) >= 0.0f)
			continue;

		D3DXVECTOR3 f((xCenter + radius), (yCenter - radius), (zCenter + radius));
		if (D3DXPlaneDotCoord(&plane, &f) >= 0.0f)
			continue;

		D3DXVECTOR3 g((xCenter - radius), (yCenter + radius), (zCenter + radius));
		if (D3DXPlaneDotCoord(&plane, &g) >= 0.0f)
			continue;

		D3DXVECTOR3 h((xCenter + radius), (yCenter + radius), (zCenter + radius));
		if (D3DXPlaneDotCoord(&plane, &h) >= 0.0f)
			continue;

		return false;
	}

	return true;
}

bool Camera::isSphereInFrustum(const D3DXVECTOR3& point, const float radius) const
{
	for (const auto& plane : mFrustum)
		if (D3DXPlaneDotCoord(&plane, &point) < -radius)
			return false;

	return true;
}

bool Camera::isCuboidInFrustum(const float xCenter, const float yCenter, const float zCenter, const float xSize, const float ySize, const float zSize) const
{
	for (const auto& plane : mFrustum)
	{
		D3DXVECTOR3 a((xCenter - xSize), (yCenter - ySize), (zCenter - zSize));
		if (D3DXPlaneDotCoord(&plane, &a) >= 0.0f)
			continue;

		D3DXVECTOR3 b((xCenter + xSize), (yCenter - ySize), (zCenter - zSize));
		if (D3DXPlaneDotCoord(&plane, &b) >= 0.0f)
			continue;

		D3DXVECTOR3 c((xCenter - xSize), (yCenter + ySize), (zCenter - zSize));
		if (D3DXPlaneDotCoord(&plane, &c) >= 0.0f)
			continue;

		D3DXVECTOR3 d((xCenter - xSize), (yCenter - ySize), (zCenter + zSize));
		if (D3DXPlaneDotCoord(&plane, &d) >= 0.0f)
			continue;

		D3DXVECTOR3 e((xCenter + xSize), (yCenter + ySize), (zCenter - zSize));
		if (D3DXPlaneDotCoord(&plane, &e) >= 0.0f)
			continue;

		D3DXVECTOR3 f((xCenter + xSize), (yCenter - ySize), (zCenter + zSize));
		if (D3DXPlaneDotCoord(&plane, &f) >= 0.0f)
			continue;

		D3DXVECTOR3 g((xCenter - xSize), (yCenter + ySize), (zCenter + zSize));
		if (D3DXPlaneDotCoord(&plane, &g) >= 0.0f)
			continue;

		D3DXVECTOR3 h((xCenter + xSize), (yCenter + ySize), (zCenter + zSize));
		if (D3DXPlaneDotCoord(&plane, &h) >= 0.0f)
			continue;

		return false;
	}

	return true;
}
