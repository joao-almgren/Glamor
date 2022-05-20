#pragma once
#include "d3dwrap.h"
#include <vector>

class Camera;

struct ScapeLod
{
	VertexBuffer mVertexBuffer[2];
	unsigned int mVertexCount[2];

	ScapeLod()
		: mVertexBuffer{ makeVertexBuffer(), makeVertexBuffer() }
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

enum class ScapeRenderMode { SHADOW, REFLECT, UNDERWATER, UNDERWATER_REFLECT, SIMPLE, CASTER };

class Scape
{
public:
	Scape(IDirect3DDevice9* pDevice, Camera* pCamera, IDirect3DTexture9* pShadowZ);

	bool init();
	void update(float tick = 1.0f);
	void draw(ScapeRenderMode mode, const D3DXMATRIX& matLightViewProj);

	[[nodiscard]] float height(float x, float z) const;
	[[nodiscard]] float angle(float x, float z) const;

private:
	bool loadHeightmap(const unsigned int size, const float scale, const float sealevel);
	[[nodiscard]] float getHeight(const unsigned int offset, const int x, const int y, const int scale) const;
	[[nodiscard]] D3DXVECTOR3 getNormal(const unsigned int offset, const int x, const int y) const;

	unsigned int generateIndices(IndexBuffer& indexBuffer, const unsigned int size) const;
	bool generateVertices(ScapeLod& lod, const int size, const int scale, const unsigned int offset) const;

	[[nodiscard]] float getInnerHeight(unsigned int offset, int x, int y, int scale, int size) const;
	bool generateSkirt(ScapeLod& lod, const int size, const int scale, const unsigned int offset);

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
