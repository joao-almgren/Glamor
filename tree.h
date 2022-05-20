#pragma once
#include "d3dwrap.h"
#include <functional>

class Camera;

struct TreeLod
{
	VertexBuffer mVertexBuffer[2];
	IndexBuffer mIndexBuffer[2];
	int mIndexCount[2];
	VertexBuffer mInstanceBuffer;
	int mInstanceCount;
	D3DXVECTOR4 mSphere[2];

	TreeLod()
		: mVertexBuffer{ MakeVertexBuffer(), MakeVertexBuffer() }
		, mIndexBuffer{ MakeIndexBuffer(), MakeIndexBuffer() }
		, mIndexCount{ 0, 0 }
		, mInstanceBuffer{ MakeVertexBuffer() }
		, mInstanceCount{ 0 }
		, mSphere{ { 0, 0, 0, 0 }, { 0, 0, 0, 0 }}
	{
	}
};

enum class TreeRenderMode { AlphaClip, AlphaBlend, Caster };

class Tree
{
public:
	Tree(IDirect3DDevice9* pDevice, Camera* pCamera, IDirect3DTexture9* pShadowZ);

	bool init(const std::function<float(float, float)>& height, const std::function<float(float, float)>& angle);
	void update(const float tick = 1.0f);
	void draw(TreeRenderMode mode, const D3DXMATRIX& matLightViewProj);

private:
	void createInstances();

	IDirect3DDevice9* mDevice;
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
