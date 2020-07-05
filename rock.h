#pragma once
#include "d3dwrap.h"
#include <functional>
#include <string>

//*********************************************************************************************************************

enum class RockRenderMode { Normal, Reflect, Refract, UnderwaterReflect };

//*********************************************************************************************************************

class Rock
{
public:
	Rock(IDirect3DDevice9* pDevice, IDirect3DTexture9* pShadowZ);

	bool init(std::function<float(float, float)> height, std::function<float(float, float)> angle);
	void update(const float tick = 1.0f);
	void draw(RockRenderMode mode, const D3DXMATRIX& matLightViewProj);

private:
	bool loadObject(std::string filename, VertexBuffer& vertexbuffer, IndexBuffer& indexbuffer);
	bool createInstances(std::function<float(float, float)> height, std::function<float(float, float)> angle);

	IDirect3DDevice9* mDevice;
	IDirect3DTexture9* mShadowZ;
	VertexBuffer mVertexBuffer;
	IndexBuffer mIndexBuffer;
	VertexBuffer mInstanceBuffer;
	Texture mTexture;
	Effect mEffect;
	Declaration mVertexDeclaration;
	int mIndexCount;
};

//*********************************************************************************************************************
