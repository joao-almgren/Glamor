extern float Angle;
extern matrix World;
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
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = NONE;
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = 0xffffffff;
};

struct VsInput
{
	float4 Position : POSITION;
	float2 Texcoord : TEXCOORD0;
};

struct VsOutput
{
	float4 Position : POSITION0;
	float4 ShadowPos : POSITION1;
	float2 Texcoord : TEXCOORD;
};

struct PsInput
{
	float4 ShadowPos : POSITION1;
	float2 Texcoord : TEXCOORD;
};

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

	float4 ModelPosition = In.Position;

	float y = cos(Angle);
	float x = abs(sin(Angle));

	ModelPosition.y = abs(ModelPosition.x) * y;
	ModelPosition.x = ModelPosition.x * x;

	float4 WorldPosition = mul(World, ModelPosition);
	float4 ViewPosition = mul(View, WorldPosition);
	Out.Position = mul(Projection, ViewPosition);

	Out.ShadowPos = mul(LightViewProj, WorldPosition);

	Out.Texcoord = In.Texcoord;

	return Out;
}

float4 Pshader(PsInput In) : Color
{
	float2 shadeUV = {
		In.ShadowPos.x / In.ShadowPos.w * 0.5 + 0.5,
		-In.ShadowPos.y / In.ShadowPos.w * 0.5 + 0.5
	};

	float pointDepth = (In.ShadowPos.z / In.ShadowPos.w) - 0.0025;
	float shade = 0.0;

	for (int i = 0; i < 4; i++)
	{
		float shadow = step(pointDepth, tex2D(Sampler1, shadeUV + filterKernel[i]).r);
		shade += shadow * 0.25;
	}

	float4 color = tex2D(Sampler0, In.Texcoord).grga;
	color.rgb *= (0.5 * shade + 0.5);

	return color;
}

technique Normal
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
