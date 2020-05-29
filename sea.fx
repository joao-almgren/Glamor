extern texture Texture0;
extern texture Texture1;
extern texture Texture2;
extern texture Texture3;
extern float4x4 World;
extern float4x4 View;
extern float4x4 Projection;
extern float4x4 RTTProjection;
extern float3 WorldCamPos;
extern float3 ViewSeaNormal;

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

sampler Sampler3 = sampler_state
{
	Texture = (Texture3);
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

struct VsOutput
{
	float4 Position : POSITION0;
	float4 View : POSITION1;
	float2 Texcoord : TEXCOORD0;
	float4 RTTexcoord : TEXCOORD1;
};

struct PsInput
{
	float4 View : POSITION1;
	float2 Texcoord : TEXCOORD0;
	float4 RTTexcoord : TEXCOORD1;
};

VsOutput Vshader(VsInput In)
{
	VsOutput Out = (VsOutput)0;

	float4 WorldPosition = mul(World, float4(In.Position, 1));
	Out.View = mul(View, WorldPosition);
	Out.Position = mul(Projection, Out.View);

	Out.Texcoord = In.Texcoord;
	Out.RTTexcoord = mul(RTTProjection, Out.View);

	return Out;
}

float LinearDepth(float d)
{
	float n = 1.0; // near
	float f = 1000.0; // far

	return 1.0 / ((1.0 / n) - (d * (f - n) / (n * f)));
//	return (2.0 * n) / (f + n - d * (f - n));
}

float4 Pshader(PsInput In) : Color
{
	float2 rttUV;
	rttUV.x = In.RTTexcoord.x / In.RTTexcoord.w * 0.5 + 0.5;
	rttUV.y = -In.RTTexcoord.y / In.RTTexcoord.w * 0.5 + 0.5;

	float4 reflect = tex2D(Sampler1, rttUV);
	float4 refract = tex2D(Sampler2, rttUV);
	//float depth = tex2D(Sampler3, rttUV).r;

	float fresnel = dot(normalize(-In.View.xyz), ViewSeaNormal);
	float4 color = lerp(reflect, refract, fresnel);

	//float4 color = 1 - depth;
	//float4 color = 1 - (depth - In.View.z / 20);

	return color;
}

technique Technique0
{
	pass Pass0
	{
		CullMode = CW;

		//AlphaBlendEnable = True;
		//SrcBlend = SRCALPHA;
		//DestBlend = INVSRCALPHA;

		VertexShader = compile vs_3_0 Vshader();
		PixelShader = compile ps_3_0 Pshader();
	}
}
