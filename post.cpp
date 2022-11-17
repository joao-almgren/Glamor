#include "post.h"
#include "config.h"

namespace
{
	constexpr float O{ -0.5f };

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

	Vertex screen[4];
}

Post::Post(std::shared_ptr<IDirect3DDevice9> pDevice) noexcept
	: mDevice{ std::move(pDevice) }
	, mVertexBuffer{ makeVertexBuffer() }
	, mEffect{ makeEffect() }
	, mVertexDeclaration{ makeVertexDeclaration() }
{
	const float w = static_cast<float>(Config::SCREEN_WIDTH) + O;
	const float h = static_cast<float>(Config::SCREEN_HEIGHT) + O;

	screen[0] = { { O, O, 0, 1 }, { 0, 0 } };
	screen[1] = { { w, O, 0, 1 }, { 1, 0 } };
	screen[2] = { { O, h, 0, 1 }, { 0, 1 } };
	screen[3] = { { w, h, 0, 1 }, { 1, 1 } };
}

bool Post::init()
{
	mVertexBuffer.reset(loadVertexBuffer(mDevice.get(), screen, sizeof(Vertex), 4, 0));
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
		mEffect->SetFloat("SourceWidth", static_cast<float>(Config::SCREEN_WIDTH));
		mEffect->SetFloat("SourceHeight", static_cast<float>(Config::SCREEN_HEIGHT));
		mEffect->SetFloat("TargetWidth", static_cast<float>(Config::BOUNCE_TEX_SIZE));
		mEffect->SetFloat("TargetHeight", static_cast<float>(Config::BOUNCE_TEX_SIZE));
		mEffect->SetTexture("Texture0", pTexture[0]);
	}
	else if (mode == PostRenderMode::ADD)
	{
		mEffect->SetTechnique("Add");
		mEffect->SetFloat("SourceWidth", static_cast<float>(Config::BOUNCE_TEX_SIZE));
		mEffect->SetFloat("SourceHeight", static_cast<float>(Config::BOUNCE_TEX_SIZE));
		mEffect->SetTexture("Texture0", pTexture[0]);
		mEffect->SetTexture("Texture1", pTexture[1]);
	}
	else if (mode == PostRenderMode::BLUR)
	{
		mEffect->SetTechnique("Blur");
		mEffect->SetFloat("SourceWidth", static_cast<float>(Config::SCREEN_WIDTH));
		mEffect->SetFloat("SourceHeight", static_cast<float>(Config::SCREEN_HEIGHT));
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
