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
	Grass(IDirect3DDevice9* pDevice);

	bool init(std::function<float(float, float)> height, std::function<float(float, float)> angle);
	void update(D3DXVECTOR3 camPos, const float tick = 1.0f);
	void draw(GrassRenderMode mode);

private:
	bool loadObject(std::string filename, VertexBuffer& vertexbuffer, IndexBuffer& indexbuffer);
	void createInstances();

	IDirect3DDevice9* mDevice;
	VertexBuffer mVertexBuffer;
	IndexBuffer mIndexBuffer;
	VertexBuffer mInstanceBuffer;
	Texture mTexture;
	Effect mEffect;
	Declaration mVertexDeclaration;
	int mIndexCount;
	D3DXVECTOR3 mPos;
	int mPlacedCount;

	std::function<float(float, float)> mHeight;
	std::function<float(float, float)> mAngle;
};

//*********************************************************************************************************************
