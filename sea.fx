extern texture Texture0;
extern texture Texture1;
extern texture Texture2;
extern texture Texture3;
extern texture Texture4;
extern texture Texture5;
extern float4x4 World;
extern float4x4 View;
extern float4x4 Projection;
extern float4x4 RTTProjection;
extern float Wave;
extern float3 CameraPosition;

sampler Sampler0 = sampler_state
{
	Texture = (Texture0);
	MinFilter = ANISOTROPIC;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = MIRROR;
	AddressV = MIRROR;
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
	AddressU = CLAMP;
	AddressV = CLAMP;
};

sampler Sampler3 = sampler_state
{
	Texture = (Texture3);
	MinFilter = ANISOTROPIC;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

sampler Sampler4 = sampler_state
{
	Texture = (Texture4);
	MinFilter = ANISOTROPIC;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};

sampler Sampler5 = sampler_state
{
	Texture = (Texture5);
	MinFilter = ANISOTROPIC;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};

struct VsInput
{
	float4 Position : POSITION;
	float2 Texcoord : TEXCOORD0;
};

struct VsOutput
{
	float4 Position : POSITION0;
	float4 View : POSITION1;
	float4 World : POSITION2;
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
	float4 World : POSITION2;
	float2 Texcoord : TEXCOORD0;
	float4 RTTexcoord : TEXCOORD1;
	float3 ViewVector : TEXCOORD2;
};

struct PsInputPlain
{
	float4 View : POSITION1;
};

struct PsOutput
{
	float4 Color : COLOR0;
};

static const float3 LightDirection = { 1, 1, 1 };
static const float4 WaterColor = { 0, 0.2, 0.1, 1 };
static const float4 SpecularColor = { 1, 0.85, 0.6, 1 };
static const float4 DiffuseColor = { 0, 0.1, 0.2, 1 };

VsOutput Vshader(VsInput In)
{
	VsOutput Out = (VsOutput)0;

	Out.World = mul(World, In.Position);
	Out.View = mul(View, Out.World);
	Out.Position = mul(Projection, Out.View);
	Out.Texcoord = In.Texcoord;
	Out.RTTexcoord = mul(RTTProjection, Out.View);

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
	const float n = 1.0;
	const float f = 1000.0;

	return n / (1 - (d * (f - n) / f));
}

PsOutput Pshader(PsInput In)
{
	PsOutput Out = (PsOutput)0;

	float2 rttUV;
	rttUV.x = In.RTTexcoord.x / In.RTTexcoord.w * 0.5 + 0.5;
	rttUV.y = -In.RTTexcoord.y / In.RTTexcoord.w * 0.5 + 0.5;

	float refract_depth = tex2D(Sampler2, rttUV).z;
	float surface_depth = tex2D(Sampler3, rttUV).z;
	float depth = LinearDepth(refract_depth) - LinearDepth(surface_depth);
	float depthfactor = saturate(pow(depth / 4, 2));

	float2 offset = (tex2D(Sampler4, In.Texcoord + Wave).xy * 2 - 1) * 0.02 * depthfactor;

	float3 vecNormal = tex2D(Sampler5, 0.5 * In.Texcoord + offset).xzy;
	vecNormal.x = vecNormal.x * 2 - 1;
	vecNormal.y = vecNormal.y * 2;
	vecNormal.z = vecNormal.z * 2 - 1;
	vecNormal = normalize(vecNormal);

	float3 vecView = normalize(In.World.xyz - CameraPosition);
	float3 vecLight = normalize(LightDirection);
	float3 vecReflectLight = reflect(vecLight, vecNormal);
	float4 specular = pow(max(dot(vecReflectLight, vecView), 0), 40) * SpecularColor * depthfactor;
	float4 diffuse = saturate(dot(vecLight, vecNormal)) * DiffuseColor;

	float4 reflect = tex2D(Sampler0, rttUV + offset) * 0.65 + diffuse;
	float4 refract = lerp(tex2D(Sampler1, rttUV + offset) * 0.65, WaterColor, saturate(depth / 35));
	float fresnel = dot(-vecView, vecNormal);
	Out.Color = lerp(reflect, refract, fresnel) + specular;

	Out.Color.a = depthfactor;

	return Out;
}

PsOutput PshaderPlain(PsInput In)
{
	PsOutput Out = (PsOutput)0;
	Out.Color = float4(1, 1, 1, 1);
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

technique Technique1
{
	pass Pass0
	{
		CullMode = CW;

		VertexShader = compile vs_3_0 VshaderPlain();
		PixelShader = compile ps_3_0 PshaderPlain();
	}
}
