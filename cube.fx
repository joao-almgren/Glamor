extern float4x4 WorldViewProjection;
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
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float4 Color : COLOR;
	float2 Texcoord : TEXCOORD0;
};

struct VsOutput
{
	float4 Position : POSITION;
	float4 Color : COLOR0;
	float2 Texcoord : TEXCOORD0;
};

struct PsInput
{
	float4 Color : COLOR0;
	float2 Texcoord : TEXCOORD0;
};

VsOutput Vshader(VsInput In)
{
	VsOutput Out = (VsOutput)0;

	Out.Position = mul(WorldViewProjection, float4(In.Position, 1));
	Out.Color = In.Color;
	Out.Texcoord = In.Texcoord;

	return Out;
}

float4 Pshader(PsInput In) : Color
{
	float4 color = tex2D(Sampler0, In.Texcoord) + In.Color;
	clip(2.9 - (color.r + color.g + color.b));

	return color;
}

technique Technique0
{
	pass Pass0
	{
		CullMode = None;

		VertexShader = compile vs_2_0 Vshader();
		PixelShader = compile ps_2_0 Pshader();
	}
}
