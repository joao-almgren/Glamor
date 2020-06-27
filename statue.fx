extern float4x4 World;
extern float4x4 View;
extern float4x4 Projection;
extern texture Texture0;
extern texture Texture1;
extern float3 CameraPosition;

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
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BINORMAL;
	float2 Texcoord : TEXCOORD0;
	float Fog : BLENDWEIGHT0;
};

struct PsInput
{
	float4 WorldPosition : POSITION1;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BINORMAL;
	float2 Texcoord : TEXCOORD0;
	float Fog : BLENDWEIGHT0;
};

static const float3 LightDirection = { 1, 1, 1 };
static const float4 FogColor = { 0.675, 0.875, 1, 1 };

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

float4 PshaderSimple(PsInput In) : Color
{
	float3 ViewDir = normalize(In.WorldPosition.xyz - CameraPosition);
	float3 LightDir = normalize(LightDirection);

	float3 ReflectLightDir = reflect(LightDir, In.Normal);
	float4 specular = pow(max(dot(ReflectLightDir, ViewDir), 0), 50) * float4(1, 1, 1, 0);

	float diffuse = dot(LightDir, normalize(In.Normal)) * 0.5 + 0.5;
	float4 color = tex2D(Sampler0, In.Texcoord) * diffuse + specular;

	return lerp(FogColor, color, In.Fog);
}

VsOutput Vshader(VsInput In)
{
	VsOutput Out = (VsOutput)0;

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
	float4 specular = pow(max(dot(ReflectLightDir, ViewDir), 0), 50) * float4(1, 1, 1, 0);

	float4 color = tex2D(Sampler0, In.Texcoord) * diffuse + specular;
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

technique Refract
{
	pass Pass0
	{
		CullMode = CW;

		VertexShader = compile vs_3_0 VshaderSimple();
		PixelShader = compile ps_3_0 PshaderSimple();
	}
}

struct VsInputLines
{
	float4 Position : POSITION;
	float4 Color : COLOR;
};

struct VsOutputLines
{
	float4 Position : POSITION;
	float4 Color : COLOR;
};

struct PsInputLines
{
	float4 Color : COLOR;
};

VsOutputLines VshaderLines(VsInputLines In)
{
	VsOutputLines Out = (VsOutputLines)0;

	float4 WorldPosition = mul(World, In.Position);
	float4 ViewPosition = mul(View, WorldPosition);
	Out.Position = mul(Projection, ViewPosition);

	Out.Color = In.Color;

	return Out;
}

float4 PshaderLines(PsInputLines In) : Color
{
	return In.Color;
}

technique Lines
{
	pass Pass0
	{
		CullMode = NONE;

		VertexShader = compile vs_3_0 VshaderLines();
		PixelShader = compile ps_3_0 PshaderLines();
	}
}
