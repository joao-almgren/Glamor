extern texture Texture0;
extern texture Texture1;
extern float4x4 WorldViewProj;
extern float4x4 ReflectWorldViewProj;

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
	AddressU = CLAMP;
	AddressV = CLAMP;
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

struct PsInputReflect
{
	float4 Position : POSITION;
	float2 Texcoord0 : TEXCOORD0;
	float4 Texcoord1 : TEXCOORD1;
};

PsInputReflect reflectVshader(VsInput In)
{
	PsInputReflect Out = (PsInputReflect)0;

	Out.Position = mul(WorldViewProj, float4(In.Position, 1));
	Out.Texcoord0 = In.Texcoord;
	Out.Texcoord1 = mul(ReflectWorldViewProj, float4(In.Position, 1));

	return Out;
}

PsOutput reflectPshader(PsInputReflect In)
{
	PsOutput Out = (PsOutput)0;

	float2 flecTex;
	flecTex.x = In.Texcoord1.x / In.Texcoord1.w * 0.5 + 0.5;
	flecTex.y = -In.Texcoord1.y / In.Texcoord1.w * 0.5 + 0.5;
	float4 reflect = tex2D(Sampler1, flecTex);

	float4 refract = tex2D(Sampler0, In.Texcoord0);

	Out.Color = 0.75 * reflect + 0.25 * refract;
	Out.Color.a = 0.75;

	return Out;
}

technique Technique1
{
	pass Pass0
	{
		CullMode = CW;

		AlphaBlendEnable = True;
		SrcBlend = SRCALPHA;
		DestBlend = INVSRCALPHA;

		VertexShader = compile vs_3_0 reflectVshader();
		PixelShader = compile ps_3_0 reflectPshader();
	}
}
