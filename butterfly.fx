extern float Angle;
extern matrix World;
extern matrix View;
extern matrix Projection;
extern texture Texture0;

sampler Sampler0 = sampler_state
{
	Texture = (Texture0);
	MinFilter = ANISOTROPIC;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};

struct VsInput
{
	float4 Position : POSITION;
	float2 Texcoord : TEXCOORD0;
};

struct VsOutput
{
	float4 Position : POSITION;
	float2 Texcoord : TEXCOORD;
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

	Out.Texcoord = In.Texcoord;

	return Out;
}

float4 Pshader(float2 Texcoord : TEXCOORD) : Color
{
	return tex2D(Sampler0, Texcoord).grga;
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
