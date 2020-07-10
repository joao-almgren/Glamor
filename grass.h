#pragma once
#include "d3dwrap.h"
#include <functional>
#include <string>

//*********************************************************************************************************************

enum class GrassRenderMode { Blend, Plain };

//*********************************************************************************************************************

class Grass
{
public:
	Grass(IDirect3DDevice9* pDevice, IDirect3DTexture9* pShadowZ);

	bool init(std::function<float(float, float)> height, std::function<float(float, float)> angle);
	void update(const D3DXVECTOR3& camPos, const float tick = 1.0f);
	void draw(GrassRenderMode mode, const D3DXMATRIX& matLightViewProj);

private:
	bool loadObject(std::string filename, VertexBuffer& vertexbuffer, IndexBuffer& indexbuffer);
	void createInstances();

	IDirect3DDevice9* mDevice;
	IDirect3DTexture9* mShadowZ;
	VertexBuffer mVertexBuffer;
	IndexBuffer mIndexBuffer;
	VertexBuffer mInstanceBuffer;
	Texture mTexture;
	Effect mEffect;
	VertexDeclaration mVertexDeclaration;
	int mIndexCount;
	D3DXVECTOR3 mCamPos;
	int mInstanceCount;
	std::function<float(float, float)> mHeight;
	std::function<float(float, float)> mAngle;
};

//*********************************************************************************************************************
