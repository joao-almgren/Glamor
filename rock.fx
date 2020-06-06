extern matrix ViewProjection;
extern texture Texture0;

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
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD0;
	float4 Color : COLOR;
	float4 Row0 : TEXCOORD1;
	float4 Row1 : TEXCOORD2;
	float4 Row2 : TEXCOORD3;
	float4 Row3 : TEXCOORD4;
};

struct VsOutput
{
	float4 Position : POSITION;
	float4 Color : COLOR;
	float2 Texcoord : TEXCOORD;
};

struct PsInput
{
	float4 Color : COLOR;
	float2 Texcoord : TEXCOORD;
};

VsOutput Vshader(VsInput In)
{
	VsOutput Out = (VsOutput)0;

	float4x4 World = { In.Row0, In.Row1, In.Row2, In.Row3 };

	float4 WorldPosition = mul(World, float4(In.Position, 1));
	Out.Position = mul(ViewProjection, WorldPosition);

	Out.Color = In.Color;
	Out.Texcoord = In.Texcoord;

	return Out;
}

float4 Pshader(PsInput In) : Color
{
	float4 color = In.Color * tex2D(Sampler0, In.Texcoord);

	return color;
}

technique Technique0
{
	pass Pass0
	{
		CullMode = CCW;

		VertexShader = compile vs_2_0 Vshader();
		PixelShader = compile ps_2_0 Pshader();
	}
}
