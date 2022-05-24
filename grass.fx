extern const float4x4 View;
extern const float4x4 Projection;
extern const float4x4 LightViewProj;
extern const texture TextureDiffuse;
extern const texture TextureDepthShadow;
extern const int ShadowTexSize;

sampler SamplerDiffuse = sampler_state
{
	Texture = (TextureDiffuse);
	MinFilter = ANISOTROPIC;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

sampler SamplerDepthShadow = sampler_state
{
	Texture = (TextureDepthShadow);
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = NONE;
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = 0xffffffff;
};

struct VsInput
{
	float4 Position : POSITION;
	float2 Texcoord : TEXCOORD0;
	float4 Row0 : TEXCOORD1;
	float4 Row1 : TEXCOORD2;
	float4 Row2 : TEXCOORD3;
	float4 Row3 : TEXCOORD4;
};

struct VsOutput
{
	float4 Position : POSITION0;
	float4 ShadowPos : POSITION1;
	float2 Texcoord : TEXCOORD;
	float Distance : BLENDWEIGHT;
};

struct PsInput
{
	float4 ShadowPos : POSITION1;
	float2 Texcoord : TEXCOORD;
	float Distance : BLENDWEIGHT;
};

static const float4 GrassColor = { 1.2, 1, 0.8, 1 };
static const float texelSize = 1.0 / ShadowTexSize;
static const float2 filterKernel[4] =
{
	float2(0 * texelSize,  0 * texelSize),
	float2(1 * texelSize,  0 * texelSize),
	float2(0 * texelSize,  1 * texelSize),
	float2(1 * texelSize,  1 * texelSize)
};

VsOutput VshaderPlain(VsInput In)
{
	VsOutput Out = (VsOutput)0;

	float4x4 World = { In.Row0, In.Row1, In.Row2, In.Row3 };

	float4 WorldPosition = mul(World, In.Position);
	float4 ViewPosition = mul(View, WorldPosition);
	Out.Position = mul(Projection, ViewPosition);

	Out.Texcoord = In.Texcoord;
	Out.Distance = 1 - smoothstep(25, 30, ViewPosition.z);

	return Out;
}

VsOutput Vshader(VsInput In)
{
	VsOutput Out = (VsOutput)0;

	float4x4 World = { In.Row0, In.Row1, In.Row2, In.Row3 };

	float4 WorldPosition = mul(World, In.Position);
	float4 ViewPosition = mul(View, WorldPosition);
	Out.Position = mul(Projection, ViewPosition);

	Out.ShadowPos = mul(LightViewProj, WorldPosition);

	Out.Texcoord = In.Texcoord;
	Out.Distance = 1 - smoothstep(25, 30, ViewPosition.z);

	return Out;
}

float4 PshaderPlain(PsInput In) : COLOR
{
	float4 color = tex2D(SamplerDiffuse, In.Texcoord) * GrassColor;
	color.a *= In.Distance;

	return color;
}

float4 Pshader(PsInput In) : COLOR
{
	float2 shadeUV = {
		In.ShadowPos.x / In.ShadowPos.w * 0.5 + 0.5,
		-In.ShadowPos.y / In.ShadowPos.w * 0.5 + 0.5
	};

	float pointDepth = (In.ShadowPos.z / In.ShadowPos.w) - 0.0025;
	float shade = 0.0;

	for (int i = 0; i < 4; i++)
	{
		float shadow = step(pointDepth, tex2D(SamplerDepthShadow, shadeUV + filterKernel[i]).r);
		shade += shadow * 0.25;
	}

	float4 color = tex2D(SamplerDiffuse, In.Texcoord) * GrassColor;
	color.rgb *= (0.5 * shade + 0.5);
	color.a *= In.Distance;

	return color;
}

technique Plain
{
	pass Pass0
	{
		CullMode = None;

		AlphaTestEnable = True;
		AlphaFunc = Greater;
		AlphaRef = 128;

		VertexShader = compile vs_3_0 VshaderPlain();
		PixelShader = compile ps_3_0 PshaderPlain();
	}
}

technique Blend
{
	pass Pass1
	{
		CullMode = None;
		ZWriteEnable = False;

		AlphaBlendEnable = True;
		SrcBlend = SRCALPHA;
		DestBlend = INVSRCALPHA;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 Pshader();
	}
}
