extern matrix View;
extern matrix Projection;
extern matrix LightViewProj;
extern texture Texture0;
extern texture Texture1;
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
	AddressU = CLAMP;
	AddressV = CLAMP;
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
	float4 Position : POSITION0;
	float4 ShadowPos : POSITION1;
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD;
	float Fog : BLENDWEIGHT0;
};

struct PsInput
{
	float4 ShadowPos : POSITION1;
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD;
	float Fog : BLENDWEIGHT0;
};

static const float3 LightDirection = { 1, 1, 1 };
static const float4 FogColor = { 0.675, 0.875, 1, 1 };
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

	float4x4 World = { In.Row0, In.Row1, In.Row2, In.Row3 };

	float4 WorldPosition = mul(World, In.Position);
	float4 ViewPosition = mul(View, WorldPosition);
	Out.Position = mul(Projection, ViewPosition);
	Out.ShadowPos = mul(LightViewProj, WorldPosition);

	Out.Normal = mul(World, In.Normal);
	Out.Texcoord = In.Texcoord;
	Out.Fog = saturate(1 / exp(ViewPosition.z * 0.0035));

	return Out;
}

float4 Pshader(PsInput In) : Color
{
	float2 shadeUV = {
		In.ShadowPos.x / In.ShadowPos.w * 0.5 + 0.5,
		-In.ShadowPos.y / In.ShadowPos.w * 0.5 + 0.5
	};

	float pointDepth = (In.ShadowPos.z / In.ShadowPos.w) - 0.0005;
	float shade = 0.0;

	for (int i = 0; i < 4; i++)
	{
		float shadow = step(pointDepth, tex2D(Sampler1, shadeUV + filterKernel[i]).r);
		shade += shadow * 0.25;
	}

	float diffuse = dot(normalize(LightDirection), normalize(In.Normal)) * 0.5 + 0.5;
	float4 color = tex2D(Sampler0, In.Texcoord);
	color.rgb *= diffuse * (0.5 * shade + 0.5);
	return lerp(FogColor, color, In.Fog);
}

technique Trunk
{
	pass Pass0
	{
		CullMode = CW;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 Pshader();
	}
}

technique BlendLeaves
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

technique PlainLeaves
{
	pass Pass0
	{
		CullMode = None;

		AlphaTestEnable = True;
		AlphaFunc = Greater;
		AlphaRef = 128;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 Pshader();
	}
}
