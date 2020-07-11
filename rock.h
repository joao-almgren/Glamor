#pragma once
#include "d3dwrap.h"
#include <functional>
#include <string>

//*********************************************************************************************************************

struct RockLod
{
	VertexBuffer mVertexBuffer;
	IndexBuffer mIndexBuffer;
	int mIndexCount;
	VertexBuffer mInstanceBuffer;
	int mInstanceCount;

	RockLod()
		: mVertexBuffer{ MakeVertexBuffer() }
		, mIndexBuffer{ MakeIndexBuffer() }
		, mIndexCount{ 0 }
		, mInstanceBuffer{ MakeVertexBuffer() }
		, mInstanceCount{ 0 }
	{
	}
};

//*********************************************************************************************************************

enum class RockRenderMode { Normal, Reflect, Refract, UnderwaterReflect };

//*********************************************************************************************************************

class Rock
{
public:
	Rock(IDirect3DDevice9* pDevice, IDirect3DTexture9* pShadowZ);

	bool init(std::function<float(float, float)> height, std::function<float(float, float)> angle);
	void update(const D3DXVECTOR3& camPos, const float tick = 1.0f);
	void draw(RockRenderMode mode, const D3DXVECTOR3& camPos, const D3DXMATRIX& matLightViewProj);

private:
	void createInstances();

	IDirect3DDevice9* mDevice;
	IDirect3DTexture9* mShadowZ;
	RockLod mLod[3];
	Texture mTexture[2];
	Effect mEffect;
	VertexDeclaration mVertexDeclaration;
	D3DXVECTOR3 mCamPos;
	std::function<float(float, float)> mHeight;
	std::function<float(float, float)> mAngle;
};

//*********************************************************************************************************************
