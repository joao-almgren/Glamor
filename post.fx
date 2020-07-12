extern float SourceWidth;
extern float SourceHeight;
extern float TargetWidth;
extern float TargetHeight;
extern texture Texture0;
extern texture Texture1;

sampler Sampler0 = sampler_state
{
	Texture = (Texture0);
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

sampler Sampler1 = sampler_state
{
	Texture = (Texture1);
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

struct VsInput
{
	float4 Position : POSITION;
	float2 Texcoord : TEXCOORD0;
};

struct VsOutput
{
	float4 Position : POSITION;
	float2 Texcoord : TEXCOORD;
};

struct PsInput
{
	float2 Texcoord : TEXCOORD;
};

VsOutput Vshader(VsInput In)
{
	VsOutput Out = (VsOutput)0;
	Out.Position = In.Position;
	Out.Texcoord = In.Texcoord;
	return Out;
}

static const float w = 1.0 / (SourceWidth);
static const float h = 1.0 / (SourceHeight);
static const int blurCount = 9;
static const float2 blur[blurCount] =
{
	{ 0, 0 },
	{ -w, -h },
	{  w, -h },
	{ -w,  h },
	{  w,  h },
	{ -2 * w, 0 },
	{  2 * w, 0 },
	{  0, -2 * h },
	{  0,  2 * h }
};
static const float2 ratio = { SourceWidth / TargetWidth, SourceHeight / TargetHeight };

float4 Pshader(PsInput In) : Color
{
	return tex2D(Sampler0, In.Texcoord);
}

float4 PshaderBlur(PsInput In) : Color
{
	float4 total = 0;
	for (int i = 0; i < blurCount; i++)
	{
		total += tex2D(Sampler0, In.Texcoord + blur[i]);
	}
	return (total / blurCount);
}

float4 PshaderAdd(PsInput In) : Color
{
	float4 total = 0;
	for (int i = 0; i < blurCount; i++)
	{
		total += tex2D(Sampler1, In.Texcoord + blur[i]);
	}

	float4 c = tex2D(Sampler0, In.Texcoord);
	return (c + total);
}

float4 PshaderDown(PsInput In) : Color
{
	float4 total = 0;
	for (int i = 0; i < blurCount; i++)
	{
		total += tex2D(Sampler0, In.Texcoord * ratio + blur[i]);
	}
	return (total / blurCount);
}

technique Passthrough
{
	pass Pass0
	{
		CullMode = NONE;
		ZEnable = FALSE;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 Pshader();
	}
}

technique Blur
{
	pass Pass0
	{
		CullMode = NONE;
		ZEnable = FALSE;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 PshaderBlur();
	}
}

technique Add
{
	pass Pass0
	{
		CullMode = NONE;
		ZEnable = FALSE;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 PshaderAdd();
	}
}

technique Down
{
	pass Pass0
	{
		CullMode = NONE;
		ZEnable = FALSE;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 PshaderDown();
	}
}
