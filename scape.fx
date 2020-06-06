extern texture Texture0;
extern texture Texture1;
extern texture Texture2;
extern float4x4 World;
extern float4x4 View;
extern float4x4 Projection;

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

struct VsInput
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD0;
};

struct VsOutput
{
	float4 Position : POSITION;
	float2 Texcoord : TEXCOORD0;
	float Fog : FOG;
	float Angle : BLENDWEIGHT0;
	float Height : BLENDWEIGHT1;
};

struct PsInput
{
	float2 Texcoord : TEXCOORD0;
	float Fog : FOG;
	float Angle : BLENDWEIGHT0;
	float Height : BLENDWEIGHT1;
};

static const float4 FogColor = { 0.675, 0.875, 1, 1 };

VsOutput Vshader(VsInput In)
{
	VsOutput Out = (VsOutput)0;

	float4 worldPos = mul(World, float4(In.Position.xyz, 1));
	float4 viewPos = mul(View, worldPos);
	Out.Position = mul(Projection, viewPos);
	Out.Texcoord = In.Texcoord;
	Out.Fog = saturate(1 / exp(viewPos.z * 0.0035));
	Out.Angle = pow((In.Normal.y - 0.5) * 2, 2);
	Out.Height = worldPos.y;

	return Out;
}

float4 CalcColor(PsInput In)
{
	float4 grass = lerp(tex2D(Sampler1, In.Texcoord), tex2D(Sampler0, In.Texcoord), In.Angle);
	float4 land = lerp(0.5 * tex2D(Sampler2, In.Texcoord), grass, saturate(In.Height + 0.5));
	return lerp(FogColor, land, In.Fog);
}

float4 Pshader(PsInput In) : Color
{
	return CalcColor(In);
}

float4 PshaderReflect(PsInput In) : Color
{
	clip(In.Height + 0.075);
	return CalcColor(In);
}

technique Technique0
{
	pass Pass0
	{
		CullMode = CW;
		//CullMode = None;
		//FillMode = WireFrame;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 Pshader();
	}
}

technique Technique1
{
	pass Pass0
	{
		CullMode = CCW;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 PshaderReflect();
	}
}
