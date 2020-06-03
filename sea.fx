extern texture Texture0;
extern texture Texture1;
extern texture Texture2;
extern texture Texture3;
extern texture Texture4;
extern float4x4 World;
extern float4x4 View;
extern float4x4 Projection;
extern float4x4 RTTProjection;
extern float3 ViewSeaNormal;

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
	AddressU = MIRROR;
	AddressV = MIRROR;
};

sampler Sampler2 = sampler_state
{
	Texture = (Texture2);
	MinFilter = ANISOTROPIC;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = MIRROR;
	AddressV = MIRROR;
};

sampler Sampler3 = sampler_state
{
	Texture = (Texture3);
	MinFilter = ANISOTROPIC;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = MIRROR;
	AddressV = MIRROR;
};

sampler Sampler4 = sampler_state
{
	Texture = (Texture4);
	MinFilter = ANISOTROPIC;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = MIRROR;
	AddressV = MIRROR;
};

struct VsInput
{
	float3 Position : POSITION;
	float2 Texcoord : TEXCOORD0;
};

struct VsOutput
{
	float4 Position : POSITION0;
	float4 View : POSITION1;
	float2 Texcoord : TEXCOORD0;
	float4 RTTexcoord : TEXCOORD1;
};

struct VsOutputPlain
{
	float4 Position : POSITION0;
};

struct PsInput
{
	float4 View : POSITION1;
	float2 Texcoord : TEXCOORD0;
	float4 RTTexcoord : TEXCOORD1;
};

struct PsInputPlain
{
	float4 View : POSITION1;
};

VsOutput Vshader(VsInput In)
{
	VsOutput Out = (VsOutput)0;

	float4 WorldPosition = mul(World, float4(In.Position, 1));
	Out.View = mul(View, WorldPosition);
	Out.Position = mul(Projection, Out.View);
	Out.Texcoord = In.Texcoord;
	Out.RTTexcoord = mul(RTTProjection, Out.View);

	return Out;
}

VsOutputPlain VshaderPlain(VsInput In)
{
	VsOutputPlain Out = (VsOutputPlain)0;

	float4 WorldPosition = mul(World, float4(In.Position, 1));
	float4 ViewPosition = mul(View, WorldPosition);
	Out.Position = mul(Projection, ViewPosition);

	return Out;
}

float LinearDepth(float d)
{
	const float n = 1.0;
	const float f = 1000.0;

	return n / (1 - (d * (f - n) / f));
}

float4 Pshader(PsInput In) : Color
{
	float2 rttUV;
	rttUV.x = In.RTTexcoord.x / In.RTTexcoord.w * 0.5 + 0.5;
	rttUV.y = -In.RTTexcoord.y / In.RTTexcoord.w * 0.5 + 0.5;

	float4 reflect = tex2D(Sampler1, rttUV);
	float4 refract = tex2D(Sampler2, rttUV);
	float refract_depth = tex2D(Sampler3, rttUV).z;
	float surface_depth = tex2D(Sampler4, rttUV).z;

	float fresnel = dot(normalize(-In.View.xyz), ViewSeaNormal);
	float4 color = lerp(reflect, refract, fresnel);

	color.a = saturate((LinearDepth(refract_depth) - LinearDepth(surface_depth)) / 5);

	return color;
}

float4 PshaderPlain(PsInput In) : Color
{
	return float4(1, 1, 1, 1);
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

technique Technique1
{
	pass Pass0
	{
		CullMode = CW;

		VertexShader = compile vs_3_0 VshaderPlain();
		PixelShader = compile ps_3_0 PshaderPlain();
	}
}
