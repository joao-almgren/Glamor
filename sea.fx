extern texture Texture0;
extern texture Texture1;
extern texture Texture2;
extern texture Texture3;
extern texture Texture4;
extern texture Texture5;
extern texture Texture6;
extern float4x4 World;
extern float4x4 View;
extern float4x4 Projection;
extern float4x4 RTTProjection;
extern float4x4 LightViewProj;
extern float Wave;
extern float3 CameraPosition;
extern float NearPlane;
extern float FarPlane;
extern int ShadowTexSize;

sampler Sampler0 = sampler_state
{
	Texture = (Texture0);
	MinFilter = ANISOTROPIC;
	MagFilter = LINEAR;
	MipFilter = NONE;
	AddressU = MIRROR;
	AddressV = MIRROR;
};

sampler Sampler1 = sampler_state
{
	Texture = (Texture1);
	MinFilter = ANISOTROPIC;
	MagFilter = LINEAR;
	MipFilter = NONE;
	AddressU = MIRROR;
	AddressV = MIRROR;
};

sampler Sampler2 = sampler_state
{
	Texture = (Texture2);
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = NONE;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

sampler Sampler3 = sampler_state
{
	Texture = (Texture3);
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = NONE;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

sampler Sampler4 = sampler_state
{
	Texture = (Texture4);
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};

sampler Sampler5 = sampler_state
{
	Texture = (Texture5);
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};

sampler Sampler6 = sampler_state
{
	Texture = (Texture6);
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
};

struct VsOutput
{
	float4 Position : POSITION0;
	float4 World : POSITION1;
	float4 ShadowPos : POSITION2;
	float2 Texcoord : TEXCOORD0;
	float4 RTTexcoord : TEXCOORD1;
};

struct VsOutputPlain
{
	float4 Position : POSITION0;
};

struct PsInput
{
	float4 World : POSITION1;
	float4 ShadowPos : POSITION2;
	float2 Texcoord : TEXCOORD0;
	float4 RTTexcoord : TEXCOORD1;
	float3 ViewVector : TEXCOORD2;
};

struct PsOutput
{
	float4 Color0 : COLOR0;
	float4 Color1 : COLOR1;
};

static const float3 LightDirection = { 1, 2, 1 };
static const float4 WaterColor = { 0, 0.125, 0.1, 1 };
static const float4 SpecularColor = { 0.8, 0.6, 0.3, 1 };
static const float4 DiffuseColor = { 1, 1.1, 1.2, 1 };
static const float texelSize = 1.0 / ShadowTexSize;
static const float2 filterKernel[4] =
{
	float2(0 * texelSize,  0 * texelSize),
	float2(1 * texelSize,  0 * texelSize),
	float2(0 * texelSize,  1 * texelSize),
	float2(1 * texelSize,  1 * texelSize)
};

VsOutput Vshader(VsInput In)
{
	VsOutput Out = (VsOutput)0;

	Out.World = mul(World, In.Position);
	float4 ViewPosition = mul(View, Out.World);
	Out.Position = mul(Projection, ViewPosition);
	Out.ShadowPos = mul(LightViewProj, Out.World);
	Out.Texcoord = In.Texcoord;
	Out.RTTexcoord = mul(RTTProjection, ViewPosition);

	return Out;
}

VsOutputPlain VshaderPlain(VsInput In)
{
	VsOutputPlain Out = (VsOutputPlain)0;

	float4 WorldPosition = mul(World, In.Position);
	float4 ViewPosition = mul(View, WorldPosition);
	Out.Position = mul(Projection, ViewPosition);

	return Out;
}

float LinearDepth(float d)
{
	return NearPlane / (1 - (d * (FarPlane - NearPlane) / FarPlane));
}

PsOutput Pshader(PsInput In)
{
	PsOutput Out = (PsOutput)0;

	float2 shadeUV = {
		In.ShadowPos.x / In.ShadowPos.w * 0.5 + 0.5,
		-In.ShadowPos.y / In.ShadowPos.w * 0.5 + 0.5
	};

	float pointDepth = (In.ShadowPos.z / In.ShadowPos.w) - 0.0025;
	float shade = 0.0;

	for (int i = 0; i < 4; i++)
	{
		float shadow = step(pointDepth, tex2D(Sampler6, shadeUV + filterKernel[i]).r);
		shade += shadow * 0.25;
	}

	float2 rttUV = {
		In.RTTexcoord.x / In.RTTexcoord.w * 0.5 + 0.5,
		-In.RTTexcoord.y / In.RTTexcoord.w * 0.5 + 0.5
	};

	float refract_depth = LinearDepth(tex2D(Sampler2, rttUV).z);
	float surface_depth = LinearDepth(tex2D(Sampler3, rttUV).z);
	float depth = refract_depth - surface_depth;
	float depthfactor = saturate(depth);

	float2 offset = tex2D(Sampler4, In.Texcoord + Wave).xy + tex2D(Sampler4, 2 * In.Texcoord + 0.5 * Wave).yx;
	offset = (offset * 2 - 1) * 0.01;

	float3 vecNormal = tex2D(Sampler5, 0.5 * In.Texcoord + offset).xzy;
	vecNormal.x = vecNormal.x * 2 - 1;
	vecNormal.y = vecNormal.y * 1.5;
	vecNormal.z = vecNormal.z * 2 - 1;
	vecNormal = lerp(float3(0, 0, 0), vecNormal, depthfactor);
	vecNormal = normalize(vecNormal);

	float3 vecView = normalize(In.World.xyz - CameraPosition);
	float3 vecLight = normalize(LightDirection);
	float3 vecReflectLight = reflect(vecLight, vecNormal);
	float4 specular = pow(max(dot(vecReflectLight, vecView), 0), 50) * SpecularColor * shade;
	float4 diffuse = (0.5 + 0.5 * dot(vecLight, vecNormal)) * DiffuseColor * (0.3 * shade + 0.7);

	float4 reflect = 0.65 * tex2D(Sampler0, rttUV + offset) * diffuse;
	float4 refract = lerp(0.65 * tex2D(Sampler1, rttUV + offset), WaterColor, saturate(depth / 15));
	float fresnel = dot(-vecView, vecNormal);

	Out.Color0.rgb = lerp(reflect, refract, fresnel).rgb;
	Out.Color0.a = depthfactor;

	Out.Color1.rgb = specular;
	Out.Color1.a = 1;

	return Out;
}

float4 PshaderPlain() : COLOR
{
	return 0;
}

PsOutput PshaderUnderwater(PsInput In)
{
	PsOutput Out = (PsOutput)0;

	float2 rttUV;
	rttUV.x = In.RTTexcoord.x / In.RTTexcoord.w * 0.5 + 0.5;
	rttUV.y = -In.RTTexcoord.y / In.RTTexcoord.w * 0.5 + 0.5;

	float2 offset = (tex2D(Sampler4, In.Texcoord + Wave).xy * 2 - 1) * 0.01;

	float3 vecNormal = tex2D(Sampler5, 0.5 * In.Texcoord + offset).xzy;
	vecNormal.x = vecNormal.x * 2 - 1;
	vecNormal.y = vecNormal.y * 1.5;
	vecNormal.z = vecNormal.z * 2 - 1;
	vecNormal = normalize(vecNormal);

	float3 vecView = normalize(In.World.xyz - CameraPosition);

	float4 reflect = tex2D(Sampler0, rttUV + offset);
	float4 refract = 0.75 * tex2D(Sampler1, rttUV + offset);
	float fresnel = dot(vecView, vecNormal);

	Out.Color0.rgb = lerp(reflect, refract, fresnel).rgb;
	Out.Color0.a = 1;

	Out.Color1.rgb = 0;
	Out.Color1.a = 1;

	return Out;
}

technique Normal
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

technique Plain
{
	pass Pass0
	{
		CullMode = CW;

		VertexShader = compile vs_3_0 VshaderPlain();
		PixelShader = compile ps_3_0 PshaderPlain();
	}
}

technique Underwater
{
	pass Pass0
	{
		CullMode = CCW;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 PshaderUnderwater();
	}
}
