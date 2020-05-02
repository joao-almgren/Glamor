extern float4x4 worldViewProj;
extern float3x3 world;
extern texture myTexture;

sampler mySampler = sampler_state
{
	Texture = (myTexture);
	MinFilter = ANISOTROPIC;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};

struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texture0 : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 position : POSITION;
	float4 color : COLOR;
	float2 texture0 : TEXCOORD0;
};

struct PS_OUTPUT
{
	float4 color : COLOR;
};

static const float3 light = { 0, 1, 0 };

VS_OUTPUT myVS(VS_INPUT IN)
{
	VS_OUTPUT OUT;

	OUT.position = mul(worldViewProj, float4(IN.position, 1));

	OUT.color = dot(light, normalize(mul(world, IN.normal)));

	OUT.texture0 = IN.texture0;

	return OUT;
}

PS_OUTPUT myPS(VS_OUTPUT IN)
{
	PS_OUTPUT OUT;

	OUT.color = tex2D(mySampler, IN.texture0) * IN.color;

	return OUT;
}

technique Technique0
{
	pass Pass0
	{
		Lighting = FALSE;

		Sampler[0] = (mySampler);

		VertexShader = compile vs_2_0 myVS();
		PixelShader = compile ps_2_0 myPS();
	}
}
