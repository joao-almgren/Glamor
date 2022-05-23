#pragma once
#include "d3dwrap.h"
#include <functional>

class Camera;

struct RockLod
{
	VertexBuffer mVertexBuffer;
	IndexBuffer mIndexBuffer;
	int mIndexCount;
	VertexBuffer mInstanceBuffer;
	int mInstanceCount;
	D3DXVECTOR4 mSphere;

	RockLod()
		: mVertexBuffer{ makeVertexBuffer() }
		, mIndexBuffer{ makeIndexBuffer() }
		, mIndexCount{}
		, mInstanceBuffer{ makeVertexBuffer() }
		, mInstanceCount{}
	{
	}
};

enum class RockRenderMode { NORMAL, REFLECT, REFRACT, UNDERWATER_REFLECT, CASTER };

class Rock
{
public:
	Rock(IDirect3DDevice9* pDevice, Camera* pCamera, IDirect3DTexture9* pShadowZ);

	bool init(const std::function<float(float, float)>& height, const std::function<float(float, float)>& angle);
	void update(float tick = 1.0f);
	void draw(RockRenderMode mode, const D3DXMATRIX& matLightViewProj) const;

private:
	void createInstances();

	IDirect3DDevice9* mDevice;
	Camera* mCamera;
	IDirect3DTexture9* mShadowZ;
	RockLod mLod[3];
	Texture mTexture[2];
	Effect mEffect;
	VertexDeclaration mVertexDeclaration;
	D3DXVECTOR3 mCamPos;
	D3DXVECTOR3 mCamDir;
	std::function<float(float, float)> mHeight;
	std::function<float(float, float)> mAngle;
};
