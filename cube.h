#pragma once
#include "d3dwrap.h"
#include "imesh.h"
#include <memory>

//*********************************************************************************************************************

class Cube : public iMesh
{
public:
	Cube(IDirect3DDevice9* pDevice);
	~Cube() = default;

	bool init() override;
	void update(const float tick = 1.0f) override;
	void draw() override;

	void setPos(const D3DXVECTOR3& pos);

private:
	VertexBuffer mVertexBuffer;
	IndexBuffer mIndexBuffer;
	Texture mTexture;
	Effect mEffect;
	float mAngle;
	D3DXVECTOR3 mPos;
};

//*********************************************************************************************************************
