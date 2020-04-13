
float4x4 worldViewProj : WorldViewProjection;

texture mytex;
sampler mysamp = sampler_state
{
	Texture = (mytex);
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float2 texture0 : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 hposition : POSITION;
	float2 texture0 : TEXCOORD0;
	float4 color : COLOR0;
};

struct PS_OUTPUT
{
	float4 color : COLOR;
};

VS_OUTPUT myvs(VS_INPUT IN)
{
	VS_OUTPUT OUT;

	OUT.hposition = mul(worldViewProj, float4(IN.position, 1));

	OUT.color = IN.color;

	OUT.texture0 = IN.texture0;

	return OUT;
}

PS_OUTPUT myps(VS_OUTPUT IN)
{
	PS_OUTPUT OUT;

	OUT.color = tex2D(mysamp, IN.texture0) * IN.color;

	return OUT;
}

technique Technique0
{
	pass Pass0
	{
		Lighting = FALSE;

		Sampler[0] = (mysamp);

		VertexShader = compile vs_2_0 myvs();
		PixelShader = compile ps_2_0 myps();
	}
}
