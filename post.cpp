#include "post.h"
#include "config.h"

namespace
{
	constexpr float O = -0.5f;
	float W;
	float H;

	constexpr D3DVERTEXELEMENT9 VERTEX_ELEMENT[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 },
		{ 0, 4 * 4, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	struct Vertex
	{
		[[maybe_unused]] D3DXVECTOR4 position;
		[[maybe_unused]] D3DXVECTOR2 texcoord;
	};

	Vertex SCREEN[4];
}

Post::Post(std::shared_ptr<IDirect3DDevice9> pDevice) noexcept
	: mDevice{ pDevice }
	, mVertexBuffer{ makeVertexBuffer() }
	, mEffect{ makeEffect() }
	, mVertexDeclaration{ makeVertexDeclaration() }
{
	W = Config::SCREEN_WIDTH + O;
	H = Config::SCREEN_HEIGHT + O;

	SCREEN[0] = { { O, O, 0, 1 }, { 0, 0 } };
	SCREEN[1] = { { W, O, 0, 1 }, { 1, 0 } };
	SCREEN[2] = { { O, H, 0, 1 }, { 0, 1 } };
	SCREEN[3] = { { W, H, 0, 1 }, { 1, 1 } };
}

bool Post::init()
{
	mVertexBuffer.reset(loadVertexBuffer(mDevice.get(), SCREEN, sizeof(Vertex), 4, 0));
	if (!mVertexBuffer)
		return false;

	mVertexDeclaration.reset(loadVertexDeclaration(mDevice.get(), VERTEX_ELEMENT));
	if (!mVertexDeclaration)
		return false;

	mEffect.reset(loadEffect(&*mDevice, L"post.fx"));
	if (!mEffect)
		return false;

	return true;
}

void Post::draw(const PostRenderMode mode, const std::vector<IDirect3DTexture9*>& pTexture) const
{
	if (mode == PostRenderMode::DOWN)
	{
		mEffect->SetTechnique("Down");
		mEffect->SetFloat("SourceWidth", (float)Config::SCREEN_WIDTH);
		mEffect->SetFloat("SourceHeight", (float)Config::SCREEN_HEIGHT);
		mEffect->SetFloat("TargetWidth", (float)Config::BOUNCE_TEX_SIZE);
		mEffect->SetFloat("TargetHeight", (float)Config::BOUNCE_TEX_SIZE);
		mEffect->SetTexture("Texture0", pTexture[0]);
	}
	else if (mode == PostRenderMode::ADD)
	{
		mEffect->SetTechnique("Add");
		mEffect->SetFloat("SourceWidth", (float)Config::BOUNCE_TEX_SIZE);
		mEffect->SetFloat("SourceHeight", (float)Config::BOUNCE_TEX_SIZE);
		mEffect->SetTexture("Texture0", pTexture[0]);
		mEffect->SetTexture("Texture1", pTexture[1]);
	}
	else if (mode == PostRenderMode::BLUR)
	{
		mEffect->SetTechnique("Blur");
		mEffect->SetFloat("SourceWidth", (float)Config::SCREEN_WIDTH);
		mEffect->SetFloat("SourceHeight", (float)Config::SCREEN_HEIGHT);
		mEffect->SetTexture("Texture0", pTexture[0]);
	}
	else
	{
		mEffect->SetTechnique("Passthrough");
		mEffect->SetTexture("Texture0", pTexture[0]);
	}

	mDevice->SetVertexDeclaration(mVertexDeclaration.get());
	mDevice->SetStreamSource(0, mVertexBuffer.get(), 0, sizeof(Vertex));

	renderEffect(mEffect.get(), [this]()
	{
		mDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	});
}
