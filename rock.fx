extern matrix View;
extern matrix Projection;
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
	float4 Position : POSITION;
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
	float Fog : BLENDWEIGHT0;
	float Height : BLENDWEIGHT1;
};

struct PsInput
{
	float4 Color : COLOR;
	float2 Texcoord : TEXCOORD;
	float Fog : BLENDWEIGHT0;
	float Height : BLENDWEIGHT1;
};

static const float4 FogColor = { 0.675, 0.875, 1, 1 };

VsOutput Vshader(VsInput In)
{
	VsOutput Out = (VsOutput)0;

	float4x4 World = { In.Row0, In.Row1, In.Row2, In.Row3 };

	float4 WorldPosition = mul(World, In.Position);
	float4 ViewPosition = mul(View, WorldPosition);
	Out.Position = mul(Projection, ViewPosition);

	Out.Color = In.Color;
	Out.Texcoord = In.Texcoord;
	Out.Fog = saturate(1 / exp(ViewPosition.z * 0.0035));
	Out.Height = WorldPosition.y;

	return Out;
}

float4 CalcColor(PsInput In)
{
	float4 color = In.Color * tex2D(Sampler0, In.Texcoord);
	return lerp(FogColor, color, In.Fog);
}

float4 Pshader(PsInput In) : Color
{
	return CalcColor(In);
}

float4 PshaderReflect(PsInput In) : Color
{
	clip(In.Height + 0.075);
	return CalcColor(In);
}

technique Technique0
{
	pass Pass0
	{
		CullMode = CCW;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 Pshader();
	}
}

technique Technique1
{
	pass Pass0
	{
		CullMode = CW;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 PshaderReflect();
	}
}
