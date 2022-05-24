extern const float4x4 View;
extern const float4x4 Projection;
extern const float4x4 LightViewProj;
extern const texture TextureDiffuse;
extern const texture TextureDepthShadow;
extern const texture TextureNormal;
extern const float3 CameraPosition;
extern const int ShadowTexSize;

sampler SamplerDiffuse = sampler_state
{
	Texture = (TextureDiffuse);
	MinFilter = ANISOTROPIC;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = WRAP;
	AddressV = WRAP;
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

sampler SamplerNormal = sampler_state
{
	Texture = (TextureNormal);
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};

struct VsInput
{
	float4 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BINORMAL;
	float2 Texcoord : TEXCOORD0;
	float4 Row0 : TEXCOORD1;
	float4 Row1 : TEXCOORD2;
	float4 Row2 : TEXCOORD3;
	float4 Row3 : TEXCOORD4;
};

struct VsOutput
{
	float4 Position : POSITION0;
	float4 WorldPosition : POSITION1;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BINORMAL;
	float2 Texcoord : TEXCOORD0;
	float4x4 World : TEXCOORD1;
	float Fog : BLENDWEIGHT0;
};

struct VsOutputSimple
{
	float4 Position : POSITION0;
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD0;
	float Fog : BLENDWEIGHT0;
	float Height : BLENDWEIGHT1;
};

struct PsInput
{
	float4 WorldPosition : POSITION1;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BINORMAL;
	float2 Texcoord : TEXCOORD0;
	float4x4 World : TEXCOORD1;
	float Fog : BLENDWEIGHT0;
};

struct PsInputSimple
{
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD0;
	float Fog : BLENDWEIGHT0;
	float Height : BLENDWEIGHT1;
};

static const float3 LightDirection = { 1, 2, 1 };
static const float4 FogColor = { 0.675, 0.875, 1, 1 };
static const float4 WaterColor = { 0, 0.125, 0.1, 1 };
static const float4 RockColor = { 0.95, 1, 0.9, 1 };
static const float4 SpecularColor = { 0.15, 0.15, 0.15, 1 };
static const float SpecularPower = 50;
static const float texelSize = 1.0 / ShadowTexSize;
static const float2 filterKernel[4] =
{
	float2(0 * texelSize,  0 * texelSize),
	float2(1 * texelSize,  0 * texelSize),
	float2(0 * texelSize,  1 * texelSize),
	float2(1 * texelSize,  1 * texelSize)
};

VsOutputSimple VshaderSimple(VsInput In)
{
	VsOutputSimple Out = (VsOutputSimple)0;

	float4x4 World = { In.Row0, In.Row1, In.Row2, In.Row3 };

	float4 WorldPosition = mul(World, In.Position);
	float4 ViewPosition = mul(View, WorldPosition);
	Out.Position = mul(Projection, ViewPosition);

	Out.Normal = mul(World, In.Normal);
	Out.Texcoord = In.Texcoord;
	Out.Fog = saturate(1 / exp(ViewPosition.z * 0.0035));
	Out.Height = WorldPosition.y;

	return Out;
}

float4 VshaderCaster(VsInput In) : POSITION
{
	float4x4 World = { In.Row0, In.Row1, In.Row2, In.Row3 };

	float4 WorldPosition = mul(World, In.Position);
	float4 ViewPosition = mul(View, WorldPosition);
	float4 Pos = mul(Projection, ViewPosition);

	return Pos;
}

VsOutput Vshader(VsInput In)
{
	VsOutput Out = (VsOutput)0;

	float4x4 World = { In.Row0, In.Row1, In.Row2, In.Row3 };
	Out.World = World;

	Out.WorldPosition = mul(World, In.Position);
	float4 ViewPosition = mul(View, Out.WorldPosition);
	Out.Position = mul(Projection, ViewPosition);

	Out.Texcoord = In.Texcoord;

	Out.Tangent = In.Tangent;
	Out.Bitangent = In.Bitangent;
	Out.Normal = In.Normal;

	Out.Fog = saturate(1 / exp(ViewPosition.z * 0.0035));

	return Out;
}

float4 CalcColorSimple(PsInputSimple In)
{
	float diffuse = dot(normalize(LightDirection), normalize(In.Normal)) * 0.5 + 0.5;
	float4 color = tex2D(SamplerDiffuse, In.Texcoord) * RockColor;
	return diffuse * color;
}

float4 Pshader(PsInput In) : COLOR
{
	float3 normal = tex2D(SamplerNormal, In.Texcoord).xyz * 2 - 1;

	float3 T = normalize(In.Tangent);
	float3 B = normalize(In.Bitangent);
	float3 N = normalize(In.Normal);

	float3x3 TBN = transpose(float3x3(T, B, N));
	normal = mul(TBN, normal);
	normal = mul(In.World, normal);
	normal = normalize(normal);

	float3 LightDir = normalize(LightDirection);
	float diffuse = dot(LightDir, normal) * 0.5 + 0.5;

	float3 ViewDir = normalize(In.WorldPosition.xyz - CameraPosition);
	float3 ReflectLightDir = reflect(LightDir, normal);
	float4 specular = pow(max(dot(ReflectLightDir, ViewDir), 0), SpecularPower) * SpecularColor;

	float4 ShadowPos = mul(LightViewProj, In.WorldPosition);

	float2 shadeUV = {
		ShadowPos.x / ShadowPos.w * 0.5 + 0.5,
		-ShadowPos.y / ShadowPos.w * 0.5 + 0.5
	};

	float pointDepth = (ShadowPos.z / ShadowPos.w) - 0.0025;
	float shade = 0.0;

	for (int i = 0; i < 4; i++)
	{
		float shadow = step(pointDepth, tex2D(SamplerDepthShadow, shadeUV + filterKernel[i]).r);
		shade += shadow * 0.25;
	}

	float4 color = tex2D(SamplerDiffuse, In.Texcoord) * RockColor;
	color = shade * specular + (0.5 * shade + 0.5) * diffuse * color;

	return lerp(FogColor, color, In.Fog);
}

float4 PshaderSimple(PsInputSimple In) : COLOR
{
	return lerp(FogColor, CalcColorSimple(In), In.Fog);
}

float4 PshaderCaster() : COLOR
{
	return 0;
}

float4 PshaderReflect(PsInputSimple In) : COLOR
{
	clip(In.Height);
	return lerp(FogColor, CalcColorSimple(In), In.Fog);
}

float4 PshaderRefract(PsInputSimple In) : COLOR
{
	clip(-In.Height + 0.1);
	float d = smoothstep(0.9, 1, In.Fog);
	return lerp(WaterColor, CalcColorSimple(In), d);
}

float4 PshaderUnderwaterReflect(PsInputSimple In) : COLOR
{
	clip(-In.Height);
	float d = smoothstep(0.9, 1, In.Fog);
	return lerp(WaterColor, CalcColorSimple(In), d);
}

technique Normal
{
	pass Pass0
	{
		CullMode = CW;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 Pshader();
	}
}

technique Simple
{
	pass Pass0
	{
		CullMode = CW;

		VertexShader = compile vs_3_0 VshaderSimple();
		PixelShader = compile ps_3_0 PshaderSimple();
	}
}

technique Caster
{
	pass Pass0
	{
		CullMode = CW;

		VertexShader = compile vs_3_0 VshaderCaster();
		PixelShader = compile ps_3_0 PshaderCaster();
	}
}

technique Reflect
{
	pass Pass0
	{
		CullMode = None;

		VertexShader = compile vs_3_0 VshaderSimple();
		PixelShader = compile ps_3_0 PshaderReflect();
	}
}

technique Refract
{
	pass Pass0
	{
		CullMode = CW;

		VertexShader = compile vs_3_0 VshaderSimple();
		PixelShader = compile ps_3_0 PshaderRefract();
	}
}

technique UnderwaterReflect
{
	pass Pass0
	{
		CullMode = None;

		VertexShader = compile vs_3_0 VshaderSimple();
		PixelShader = compile ps_3_0 PshaderUnderwaterReflect();
	}
}
