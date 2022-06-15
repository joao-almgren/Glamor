#pragma once
#include "d3dwrap.h"
#include <memory>

class Camera;

class Skybox
{
public:
	Skybox(std::shared_ptr<IDirect3DDevice9> pDevice, Camera* pCamera) noexcept;

	bool init();
	void update([[maybe_unused]] float tick = 1.0f) noexcept;
	void draw() const;

private:
	std::shared_ptr<IDirect3DDevice9> mDevice;
	Camera* mCamera;
	VertexBuffer mVertexBuffer;
	Texture mTexture[5];
	Effect mEffect;
	VertexDeclaration mVertexDeclaration;
};
