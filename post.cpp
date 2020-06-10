#include "post.h"

//*********************************************************************************************************************

namespace
{
	const float o = -0.5f;
	const float w = 1024 + o;
	const float h = 768 + o;

	const auto vertexFVF{ D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) };
	struct Vertex
	{
		D3DXVECTOR4 position;
		D3DXVECTOR2 texcoord;
	};

	const Vertex screen[]
	{
		{ { o, o, 0, 1 }, { 0, 0 } },
		{ { w, o, 0, 1 }, { 1, 0 } },
		{ { o, h, 0, 1 }, { 0, 1 } },
		{ { w, h, 0, 1 }, { 1, 1 } },
	};
}

//*********************************************************************************************************************

Post::Post(IDirect3DDevice9* pDevice)
	: mDevice(pDevice)
	, mVertexBuffer(nullptr, vertexDeleter)
	, mEffect(nullptr, effectDeleter)
{
}

//*********************************************************************************************************************

bool Post::init()
{
	mVertexBuffer.reset(CreateVertexBuffer(mDevice, screen, sizeof(Vertex), 4, vertexFVF));
	if (!mVertexBuffer)
		return false;

	mEffect.reset(CreateEffect(mDevice, L"post.fx"));
	if (!mEffect)
		return false;

	return true;
}

//*********************************************************************************************************************

void Post::draw(IDirect3DTexture9* pTexture)
{
	mEffect->SetTexture("Texture0", pTexture);

	mEffect->SetTechnique("Technique1");

	mDevice->SetFVF(vertexFVF);
	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(Vertex));

	RenderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	});
}

//*********************************************************************************************************************
