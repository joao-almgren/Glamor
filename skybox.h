#pragma once
#include "d3dwrap.h"

class Camera;

class Skybox
{
public:
	Skybox(IDirect3DDevice9* pDevice, Camera* pCamera);

	bool init();
	void update([[maybe_unused]] float tick = 1.0f) noexcept;
	void draw() const;

private:
	IDirect3DDevice9* mDevice;
	Camera* mCamera;
	VertexBuffer mVertexBuffer;
	Texture mTexture[5];
	Effect mEffect;
	VertexDeclaration mVertexDeclaration;
};
