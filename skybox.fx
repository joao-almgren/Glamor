extern matrix WorldViewProjection;
extern texture TextureDiffuse;

sampler SamplerDiffuse = sampler_state
{
	Texture = (TextureDiffuse);
	MinFilter = POINT;
	MagFilter = POINT;
	MipFilter = POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

struct VsInput
{
	float4 Position : POSITION;
	float2 Texcoord : TEXCOORD;
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
	Out.Position = mul(WorldViewProjection, In.Position);
	Out.Texcoord = In.Texcoord;
	return Out;
}

float4 Pshader(PsInput In) : Color
{
	float4 color = tex2D(SamplerDiffuse, In.Texcoord);
	return color;
}

technique Normal
{
	pass Pass0
	{
		ZWriteEnable = FALSE;
		CullMode = NONE;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 Pshader();
	}
}
