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

struct PsInput
{
	float4 Position : POSITION;
	float2 Texcoord : TEXCOORD0;
	float Fog : FOG;
	float Blend0 : BLENDWEIGHT0;
	float Blend1 : BLENDWEIGHT1;
};

struct PsOutput
{
	float4 Color : COLOR;
};

PsInput Vshader(VsInput In)
{
	PsInput Out = (PsInput)0;

	float4 pos = mul(World, float4(In.Position.xyz, 1));
	float4 camPos = mul(View, pos);
	Out.Position = mul(Projection, camPos);
	Out.Texcoord = In.Texcoord;
	Out.Fog = saturate(1 / exp(camPos.z * 0.000015));
	Out.Blend0 = pow((In.Normal.y - 0.5) * 2, 2);
	Out.Blend1 = saturate(In.Position.y - 5);

	return Out;
}

PsOutput Pshader(PsInput In)
{
	PsOutput Out = (PsOutput)0;

	float4 grass = lerp(tex2D(Sampler1, In.Texcoord), tex2D(Sampler0, In.Texcoord), In.Blend0);
	float4 land = lerp(0.5 * tex2D(Sampler2, In.Texcoord), grass, In.Blend1);
	Out.Color = lerp(float4(192, 224, 255, 1), land, In.Fog);

	return Out;
}

technique Technique0
{
	pass Pass0
	{
		CullMode = CW;
		FillMode = Solid;
		//CullMode = None;
		//FillMode = WireFrame;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 Pshader();
	}
}
