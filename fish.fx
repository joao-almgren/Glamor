extern const texture TextureDiffuse;
extern const float4x4 View;
extern const float4x4 Projection;
extern const float Angle;

sampler SamplerDiffuse = sampler_state
{
	Texture = TextureDiffuse;
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
	float4 Row0 : TEXCOORD1;
	float4 Row1 : TEXCOORD2;
	float4 Row2 : TEXCOORD3;
	float4 Row3 : TEXCOORD4;
};

struct VsOutput
{
	float4 Position : POSITION;
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD;
	float Fog : BLENDWEIGHT0;
};

struct PsInput
{
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD;
	float Fog : BLENDWEIGHT0;
};

static const float3 LightDirection = { 1, 2, 1 };
static const float4 WaterColor = { 0, 0.125, 0.1, 0 };

VsOutput Vshader(VsInput In)
{
	VsOutput Out = (VsOutput)0;

	float4x4 World = { In.Row0, In.Row1, In.Row2, In.Row3 };

	In.Position.x += 15 * sin(Angle + 0.01 * In.Position.z);

	float4 WorldPosition = mul(World, In.Position);
	float4 ViewPosition = mul(View, WorldPosition);
	Out.Position = mul(Projection, ViewPosition);

	Out.Normal = mul(World, In.Normal);
	Out.Texcoord = In.Texcoord;
	Out.Fog = saturate(1 / exp(ViewPosition.z * 0.0035));

	return Out;
}

float4 Pshader(PsInput In) : COLOR
{
	float diffuse = dot(normalize(LightDirection), normalize(In.Normal)) * 0.5 + 0.5;
	float4 color = tex2D(SamplerDiffuse, In.Texcoord) * diffuse;
	float d = smoothstep(0.9, 1, In.Fog);
	return lerp(WaterColor, color, d);
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

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 Pshader();
	}
}
