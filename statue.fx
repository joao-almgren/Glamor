extern float4x4 World;
extern float4x4 View;
extern float4x4 Projection;
extern float4x4 LightViewProj;
extern texture Texture0;
extern texture Texture1;
extern texture Texture2;
extern float3 CameraPosition;
extern int ShadowTexSize;

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

sampler Sampler2 = sampler_state
{
	Texture = (Texture2);
	MinFilter = POINT;
	MagFilter = POINT;
	MipFilter = NONE;
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = 0xffffffff;
};

struct VsInput
{
	float4 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BINORMAL;
	float2 Texcoord : TEXCOORD0;
};

struct VsOutput
{
	float4 Position : POSITION;
	float4 WorldPosition : POSITION1;
	float4 ShadowPos : POSITION2;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BINORMAL;
	float2 Texcoord : TEXCOORD0;
	float Fog : BLENDWEIGHT0;
};

struct PsInput
{
	float4 WorldPosition : POSITION1;
	float4 ShadowPos : POSITION2;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BINORMAL;
	float2 Texcoord : TEXCOORD0;
	float Fog : BLENDWEIGHT0;
};

static const float3 LightDirection = { 1, 1, 1 };
static const float4 FogColor = { 0.675, 0.875, 1, 1 };
static const float4 SpecularColor = { 0.25, 0.25, 0.2, 1 };
static const float SpecularPower = 20;
static const float texelSize = 1.0 / ShadowTexSize;
static const float2 filterKernel[4] =
{
	float2(0 * texelSize,  0 * texelSize),
	float2(1 * texelSize,  0 * texelSize),
	float2(0 * texelSize,  1 * texelSize),
	float2(1 * texelSize,  1 * texelSize)
};

VsOutput VshaderSimple(VsInput In)
{
	VsOutput Out = (VsOutput)0;

	Out.WorldPosition = mul(World, In.Position);
	float4 ViewPosition = mul(View, Out.WorldPosition);
	Out.Position = mul(Projection, ViewPosition);

	Out.Normal = normalize(mul(World, In.Normal));
	Out.Texcoord = In.Texcoord;
	Out.Fog = saturate(1 / exp(ViewPosition.z * 0.0035));

	return Out;
}

VsOutput VshaderCaster(VsInput In)
{
	VsOutput Out = (VsOutput)0;

	Out.WorldPosition = mul(World, In.Position);
	float4 ViewPosition = mul(View, Out.WorldPosition);
	Out.Position = mul(Projection, ViewPosition);

	return Out;
}

float4 PshaderSimple(PsInput In) : Color
{
	float3 ViewDir = normalize(In.WorldPosition.xyz - CameraPosition);
	float3 LightDir = normalize(LightDirection);

	float3 ReflectLightDir = reflect(LightDir, In.Normal);
	float4 specular = pow(max(dot(ReflectLightDir, ViewDir), 0), SpecularPower) * SpecularColor;

	float diffuse = dot(LightDir, normalize(In.Normal)) * 0.5 + 0.5;

	float4 color = tex2D(Sampler0, In.Texcoord);
	color = pow(color, 2) * diffuse + specular;

	return lerp(FogColor, color, In.Fog);
}

float4 PshaderCaster(PsInput In) : Color
{
	float4 color = 1;

	return color;
}

VsOutput Vshader(VsInput In)
{
	VsOutput Out = (VsOutput)0;

	Out.WorldPosition = mul(World, In.Position);
	float4 ViewPosition = mul(View, Out.WorldPosition);
	Out.Position = mul(Projection, ViewPosition);
	Out.ShadowPos = mul(LightViewProj, Out.WorldPosition);

	Out.Texcoord = In.Texcoord;

	Out.Tangent = In.Tangent;
	Out.Bitangent = In.Bitangent;
	Out.Normal = In.Normal;

	Out.Fog = saturate(1 / exp(ViewPosition.z * 0.0035));

	return Out;
}

float4 Pshader(PsInput In) : Color
{
	float3 normal = tex2D(Sampler1, In.Texcoord).xyz * 2 - 1;

	float3 T = normalize(In.Tangent);
	float3 B = normalize(In.Bitangent);
	float3 N = normalize(In.Normal);

	float3x3 TBN = transpose(float3x3(T, B, N));
	normal = mul(TBN, normal);
	normal = mul(World, normal);
	normal = normalize(normal);

	float3 LightDir = normalize(LightDirection);
	float diffuse = dot(LightDir, normal) * 0.5 + 0.5;

	float3 ViewDir = normalize(In.WorldPosition.xyz - CameraPosition);
	float3 ReflectLightDir = reflect(LightDir, normal);
	float4 specular = pow(max(dot(ReflectLightDir, ViewDir), 0), SpecularPower) * SpecularColor;

	float4 color = tex2D(Sampler0, In.Texcoord);

	float2 shadeUV = {
		In.ShadowPos.x / In.ShadowPos.w * 0.5 + 0.5,
		-In.ShadowPos.y / In.ShadowPos.w * 0.5 + 0.5
	};

	float pointDepth = (In.ShadowPos.z / In.ShadowPos.w) - 0.0025;
	float shade = 0.0;

	for (int i = 0; i < 4; i++)
	{
		float shadow = step(pointDepth, tex2D(Sampler2, shadeUV + filterKernel[i]).r);
		shade += shadow * 0.25;
	}

	color = shade * specular + (0.5 * shade + 0.5) * diffuse * pow(color, 2);

	return lerp(FogColor, color, In.Fog);
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

technique Reflect
{
	pass Pass0
	{
		CullMode = CCW;

		VertexShader = compile vs_3_0 VshaderSimple();
		PixelShader = compile ps_3_0 PshaderSimple();
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
