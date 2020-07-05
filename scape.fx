extern texture Texture0;
extern texture Texture1;
extern texture Texture2;
extern texture Texture3;
extern texture Texture4;
extern float4x4 World;
extern float4x4 View;
extern float4x4 Projection;
extern float4x4 LightViewProj;
extern float Wave;

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
	MinFilter = ANISOTROPIC;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};

sampler Sampler3 = sampler_state
{
	Texture = (Texture3);
	MinFilter = ANISOTROPIC;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};

sampler Sampler4 = sampler_state
{
	Texture = (Texture4);
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
};

struct VsOutput
{
	float4 Position : POSITION0;
	float4 ShadowPos : POSITION1;
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD0;
	float Fog : BLENDWEIGHT0;
	float Angle : BLENDWEIGHT1;
	float Height : BLENDWEIGHT2;
};

struct PsInput
{
	float4 ShadowPos : POSITION1;
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD0;
	float Fog : BLENDWEIGHT0;
	float Angle : BLENDWEIGHT1;
	float Height : BLENDWEIGHT2;
};

static const float3 LightDirection = { 1, 1, 1 };
static const float4 FogColor = { 0.675, 0.875, 1, 1 };
static const float4 WaterColor = { 0, 0.125, 0.1, 1 };
static const float texelSize = 1.0 / 4096.0;
static const float2 filterKernel[4] =
{
	float2( 0 * texelSize,  0 * texelSize),
	float2( 1 * texelSize,  0 * texelSize),
	float2( 0 * texelSize,  1 * texelSize),
	float2( 1 * texelSize,  1 * texelSize)
};

VsOutput Vshader(VsInput In)
{
	VsOutput Out = (VsOutput)0;

	float4 worldPos = mul(World, In.Position);
	float4 viewPos = mul(View, worldPos);
	Out.Position = mul(Projection, viewPos);
	Out.ShadowPos = mul(LightViewProj, worldPos);
	Out.Normal = mul(World, In.Normal);
	Out.Texcoord = In.Texcoord;
	Out.Fog = saturate(1 / exp(viewPos.z * 0.0035));
	Out.Angle = pow((In.Normal.y - 0.5) * 2, 2);
	Out.Height = worldPos.y;

	return Out;
}

float4 CalcColor(PsInput In)
{
	float4 grass = tex2D(Sampler0, In.Texcoord);
	float4 rock = tex2D(Sampler1, In.Texcoord);
	float4 land = lerp(rock, grass, In.Angle);
	float4 mud = 0.5 * tex2D(Sampler2, In.Texcoord);
	float4 caustic = lerp(1, tex2D(Sampler3, 2 * In.Texcoord + Wave), smoothstep(-1, -2, In.Height));
	float4 color = lerp(mud * caustic, land, smoothstep(-0.5, 0.5, In.Height));
	float diffuse = dot(normalize(LightDirection), normalize(In.Normal)) * 0.5 + 0.5;
	return color * diffuse;
}

float4 Pshader(PsInput In) : Color
{
	return lerp(FogColor, CalcColor(In), In.Fog);
}

float4 PshaderReflect(PsInput In) : Color
{
	clip(In.Height);
	return lerp(FogColor, CalcColor(In), In.Fog);
}

float4 PshaderUnderwater(PsInput In) : Color
{
	float d = smoothstep(0.9, 1, In.Fog);
	return lerp(WaterColor, CalcColor(In), d);
}

float4 PshaderUnderwaterReflect(PsInput In) : Color
{
	clip(-In.Height);
	float d = smoothstep(0.9, 1, In.Fog);
	return lerp(WaterColor, CalcColor(In), d);
}

float4 PshaderShadow(PsInput In) : Color
{
	float2 shadeUV = {
		In.ShadowPos.x / In.ShadowPos.w * 0.5 + 0.5,
		-In.ShadowPos.y / In.ShadowPos.w * 0.5 + 0.5
	};

	float pointDepth = (In.ShadowPos.z / In.ShadowPos.w) - 0.0005;
	float shade = 0.0;

	for (int i = 0; i < 4; i++)
	{
		float shadow = step(pointDepth, tex2D(Sampler4, shadeUV + filterKernel[i]).r);
		shade += shadow * 0.25;
	}

	float4 color = CalcColor(In) * (0.5 * shade + 0.5);
	return lerp(FogColor, color, In.Fog);
}

technique Simple
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
		PixelShader = compile ps_3_0 PshaderReflect();
	}
}

technique Underwater
{
	pass Pass0
	{
		CullMode = CW;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 PshaderUnderwater();
	}
}

technique UnderwaterReflect
{
	pass Pass0
	{
		CullMode = CCW;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 PshaderUnderwaterReflect();
	}
}

technique Shadow
{
	pass Pass0
	{
		CullMode = CW;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 PshaderShadow();
	}
}
