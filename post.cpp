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

void Post::draw(PostRenderMode mode, const std::vector<IDirect3DTexture9*>& pTexture)
{
	if (mode == PostRenderMode::Down)
	{
		mEffect->SetTechnique("Down");
		mEffect->SetFloat("SourceWidth", 1024);
		mEffect->SetFloat("SourceHeight", 768);
		mEffect->SetFloat("TargetWidth", 256);
		mEffect->SetFloat("TargetHeight", 256);
		mEffect->SetTexture("Texture0", pTexture[0]);
	}
	else if (mode == PostRenderMode::Add)
	{
		mEffect->SetTechnique("Add");
		mEffect->SetFloat("SourceWidth", 256);
		mEffect->SetFloat("SourceHeight", 256);
		mEffect->SetTexture("Texture0", pTexture[0]);
		mEffect->SetTexture("Texture1", pTexture[1]);
	}
	else if (mode == PostRenderMode::Blur)
	{
		mEffect->SetTechnique("Blur");
		mEffect->SetFloat("SourceWidth", 1024);
		mEffect->SetFloat("SourceHeight", 768);
		mEffect->SetTexture("Texture0", pTexture[0]);
	}
	else
	{
		mEffect->SetTechnique("Passthrough");
		mEffect->SetTexture("Texture0", pTexture[0]);
	}

	mDevice->SetFVF(vertexFVF);
	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(Vertex));

	RenderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	});
}

//*********************************************************************************************************************
