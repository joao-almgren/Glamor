extern texture Texture0;

sampler Sampler0 = sampler_state
{
	Texture = (Texture0);
	MinFilter = POINT;
	MagFilter = POINT;
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

float4 Pshader(PsInput In) : Color
{
	return tex2D(Sampler0, In.Texcoord);
}

static const float w = 1.0 / (1024);
static const float h = 1.0 / (768);
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

float4 PshaderBlur(PsInput In) : Color
{
	float4 total = 0;
	for (int i = 0; i < blurCount; i++)
	{
		total += tex2D(Sampler0, In.Texcoord + blur[i]);
	}
	return (total / blurCount);
}

technique Technique0
{
	pass Pass0
	{
		CullMode = NONE;
		ZEnable = FALSE;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 Pshader();
	}
}

technique Technique1
{
	pass Pass0
	{
		CullMode = NONE;
		ZEnable = FALSE;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 PshaderBlur();
	}
}
