extern texture Texture0;
extern float3x3 World;
extern float4x4 WorldViewProj;

sampler Sampler0 = sampler_state
{
	Texture = (Texture0);
	MinFilter = ANISOTROPIC;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};

struct VsInput
{
	float3 Position : POSITION;
	float2 Texcoord : TEXCOORD0;
};

struct PsInput
{
	float4 Position : POSITION;
	float2 Texcoord : TEXCOORD0;
};

struct PsOutput
{
	float4 Color : COLOR;
};

PsInput Vshader(VsInput In)
{
	PsInput Out = (PsInput)0;

	Out.Position = mul(WorldViewProj, float4(In.Position, 1));
	Out.Texcoord = In.Texcoord;

	return Out;
}

PsOutput Pshader(PsInput In)
{
	PsOutput Out = (PsOutput)0;

	Out.Color = 0.6 * tex2D(Sampler0, In.Texcoord);
	Out.Color.a = 0.8;

	return Out;
}

technique Technique0
{
	pass Pass0
	{
		CullMode = CW;

		AlphaBlendEnable = True;
		SrcBlend = SRCALPHA;
		DestBlend = INVSRCALPHA;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 Pshader();
	}
}
