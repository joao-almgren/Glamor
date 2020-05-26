extern texture Texture0;
extern texture Texture1;
extern texture Texture2;
extern float4x4 World;
extern float4x4 View;
extern float4x4 Proj;
extern float4x4 RTTProj;
extern float3 CamPos;

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
	AddressU = MIRROR;
	AddressV = MIRROR;
};

sampler Sampler2 = sampler_state
{
	Texture = (Texture2);
	MinFilter = ANISOTROPIC;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = MIRROR;
	AddressV = MIRROR;
};

struct VsInput
{
	float3 Position : POSITION;
	float2 Texcoord : TEXCOORD0;
};

struct PsInput
{
	float4 Position : POSITION;
	float4 World : POSITION1;
	float2 Texcoord0 : TEXCOORD0;
	float4 Texcoord1 : TEXCOORD1;
};

struct PsOutput
{
	float4 Color : COLOR;
};

PsInput Vshader(VsInput In)
{
	PsInput Out = (PsInput)0;

	Out.World = mul(World, float4(In.Position, 1));
	float4 ViewPosition = mul(View, Out.World);
	Out.Position = mul(Proj, ViewPosition);

	Out.Texcoord0 = In.Texcoord;
	Out.Texcoord1 = mul(RTTProj, ViewPosition);

	return Out;
}

PsOutput Pshader(PsInput In)
{
	PsOutput Out = (PsOutput)0;

	float2 rttTex;
	rttTex.x = In.Texcoord1.x / In.Texcoord1.w * 0.5 + 0.5;
	rttTex.y = -In.Texcoord1.y / In.Texcoord1.w * 0.5 + 0.5;

	float4 reflect = tex2D(Sampler1, rttTex);
	float4 refract = tex2D(Sampler2, rttTex);

	float fresnel = dot(normalize(CamPos - In.World.xyz), float3(0, 1, 0));
	Out.Color = lerp(reflect, refract, fresnel);

	return Out;
}

technique Technique0
{
	pass Pass0
	{
		CullMode = CW;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 Pshader();
	}
}
