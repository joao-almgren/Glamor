#pragma once
#include "d3dwrap.h"
#include <functional>
#include <string>

class Camera;

enum class GrassRenderMode { BLEND, PLAIN };

class Grass
{
public:
	Grass(IDirect3DDevice9* pDevice, Camera* pCamera, IDirect3DTexture9* pShadowZ);

	bool init(const std::function<float(float, float)>& height, const std::function<float(float, float)>& angle);
	void update(float tick = 1.0f);
	void draw(GrassRenderMode mode, const D3DXMATRIX& matLightViewProj) const;

private:
	bool loadObject(const std::string& filename, VertexBuffer& vertexbuffer, IndexBuffer& indexbuffer);
	void createInstances();

	IDirect3DDevice9* mDevice;
	Camera* mCamera;
	IDirect3DTexture9* mShadowZ;
	VertexBuffer mVertexBuffer;
	IndexBuffer mIndexBuffer;
	VertexBuffer mInstanceBuffer;
	Texture mTexture;
	Effect mEffect;
	VertexDeclaration mVertexDeclaration;
	int mIndexCount;
	D3DXVECTOR3 mCamPos;
	D3DXVECTOR3 mCamDir;
	int mInstanceCount;
	D3DXVECTOR4 mSphere;
	std::function<float(float, float)> mHeight;
	std::function<float(float, float)> mAngle;
};
