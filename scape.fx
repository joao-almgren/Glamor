extern texture Texture0;
extern texture Texture1;
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

sampler Sampler1 = sampler_state
{
	Texture = (Texture1);
	MinFilter = ANISOTROPIC;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};

struct VsInput
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD0;
};

struct PsInput
{
	float4 Position : POSITION;
	float2 Texcoord : TEXCOORD0;
	float Blend : BLENDWEIGHT;
};

struct PsOutput
{
	float4 Color : COLOR;
};

static const float3 Light = { 0, 1, 0 };

PsInput Vshader(VsInput In)
{
	PsInput Out = (PsInput)0;

	Out.Position = mul(WorldViewProj, float4(In.Position, 1));
	Out.Texcoord = In.Texcoord;
	Out.Blend = 2 * dot(Light, mul(World, In.Normal)) - 1;

	return Out;
}

PsOutput Pshader(PsInput In)
{
	PsOutput Out = (PsOutput)0;

	Out.Color = tex2D(Sampler0, In.Texcoord) * In.Blend
		+ tex2D(Sampler1, In.Texcoord) * (1 - In.Blend);

	return Out;
}

technique Technique0
{
	pass Pass0
	{
		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 Pshader();
	}
}
