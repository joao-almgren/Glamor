#pragma once
#include "d3dwrap.h"

class Camera;

enum class SeaRenderMode { NORMAL, PLAIN, UNDERWATER };

class Sea
{
public:
	Sea(IDirect3DDevice9* pDevice, Camera* pCamera, IDirect3DTexture9* pReflect, IDirect3DTexture9* pRefract,
		IDirect3DTexture9* pRefractZ, IDirect3DTexture9* pSurfaceZ, IDirect3DTexture9* pShadowZ);

	bool init();
	void update([[maybe_unused]] float tick = 1.0f) noexcept;
	void draw(const SeaRenderMode mode, const D3DXMATRIX& matRTTProj, const D3DXMATRIX& matLightViewProj) const;

private:
	IDirect3DDevice9* mDevice;
	Camera* mCamera;
	IDirect3DTexture9* mReflect;
	IDirect3DTexture9* mRefract;
	IDirect3DTexture9* mRefractZ;
	IDirect3DTexture9* mSurfaceZ;
	IDirect3DTexture9* mShadowZ;
	VertexBuffer mVertexBuffer;
	Texture mTexture[2];
	Effect mEffect;
	VertexDeclaration mVertexDeclaration;
	float mWave;
};
