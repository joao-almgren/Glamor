#pragma once
#include "d3dwrap.h"
#include <vector>

class Camera;

struct ScapeLod
{
	VertexBuffer mVertexBuffer[2];
	unsigned int mVertexCount[2];

	ScapeLod()
		: mVertexBuffer{ MakeVertexBuffer(), MakeVertexBuffer() }
		, mVertexCount{ 0, 0 }
	{
	}
};

struct ScapeChunk
{
	ScapeLod mLod[4];
	float mPosX, mPosY;

	ScapeChunk()
		: mLod{}
		, mPosX{ 0.0f }
		, mPosY{ 0.0f }
	{
	}
};

enum class ScapeRenderMode { Shadow, Reflect, Underwater, UnderwaterReflect, Simple, Caster };

class Scape
{
public:
	Scape(IDirect3DDevice9* pDevice, Camera* pCamera, IDirect3DTexture9* pShadowZ);

	bool init();
	void update(const float tick = 1.0f);
	void draw(ScapeRenderMode mode, const D3DXMATRIX& matLightViewProj);

	float height(float x, float z);
	float angle(float x, float z);

private:
	bool loadHeightmap(const int size, const float scale, const float sealevel);
	float getHeight(const int offset, const int x, const int y, const int scale);
	D3DXVECTOR3 getNormal(const int offset, const int x, const int y);

	unsigned int generateIndices(IndexBuffer& indexBuffer, const int size);
	bool generateVertices(ScapeLod& lod, const unsigned int size, const int scale, const int offset);

	float getInnerHeight(int offset, int x, int y, int scale, int size);
	bool generateSkirt(ScapeLod& lod, const int size, const int scale, const int offset);

	IDirect3DDevice9* mDevice;
	Camera* mCamera;
	IDirect3DTexture9* mShadowZ;
	Texture mTexture[3];
	Texture mCaustic[32];
	Effect mEffect;
	VertexDeclaration mVertexDeclaration;
	std::vector<float> mHeightmap;
	const unsigned int mHeightmapSize;
	std::vector<ScapeChunk> mChunk;
	IndexBuffer mIndexBuffer[5];
	unsigned int mIndexCount[5];
	int mCausticIndex;
	float mWave;
};
