#pragma once
#include "d3dwrap.h"
#include <functional>
#include <memory>

class Camera;

struct TreeLod
{
	VertexBuffer mVertexBuffer[2];
	IndexBuffer mIndexBuffer[2];
	int mIndexCount[2];
	VertexBuffer mInstanceBuffer;
	int mInstanceCount;
	D3DXVECTOR4 mSphere[2];

	TreeLod() noexcept
		: mVertexBuffer{ makeVertexBuffer(), makeVertexBuffer() }
		, mIndexBuffer{ makeIndexBuffer(), makeIndexBuffer() }
		, mIndexCount{ 0, 0 }
		, mInstanceBuffer{ makeVertexBuffer() }
		, mInstanceCount{ 0 }
		, mSphere{ { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }
	{
	}
};

enum class TreeRenderMode { ALPHA_CLIP, ALPHA_BLEND, CASTER };

class Tree
{
public:
	Tree(std::shared_ptr<IDirect3DDevice9> pDevice, Camera* pCamera, IDirect3DTexture9* pShadowZ);

	bool init(const std::function<float(float, float)>& height, const std::function<float(float, float)>& angle);
	void update([[maybe_unused]] float tick = 1.0f);
	void draw(TreeRenderMode mode, const D3DXMATRIX& matLightViewProj) const;

private:
	void createInstances();

	std::shared_ptr<IDirect3DDevice9> mDevice;
	Camera* mCamera;
	IDirect3DTexture9* mShadowZ;
	TreeLod mLod[3];
	Texture mTexture[3];
	Effect mEffect;
	VertexDeclaration mVertexDeclaration;
	D3DXVECTOR3 mCamPos;
	D3DXVECTOR3 mCamDir;
	std::function<float(float, float)> mHeight;
	std::function<float(float, float)> mAngle;
};
