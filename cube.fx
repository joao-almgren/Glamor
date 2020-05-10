extern float4x4 worldViewProj;
extern texture myTexture;

sampler mySampler = sampler_state
{
	Texture = (myTexture);
	MinFilter = POINT;
	MagFilter = POINT;
	MipFilter = POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
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
	float4 position : POSITION;
	float4 color : COLOR0;
	float2 texture0 : TEXCOORD0;
};

struct PS_OUTPUT
{
	float4 color : COLOR;
};

VS_OUTPUT myVS(VS_INPUT IN)
{
	VS_OUTPUT OUT;

	OUT.position = mul(worldViewProj, float4(IN.position, 1));

	OUT.color = IN.color;

	OUT.texture0 = IN.texture0;

	return OUT;
}

PS_OUTPUT myPS(VS_OUTPUT IN)
{
	PS_OUTPUT OUT;

	OUT.color = tex2D(mySampler, IN.texture0) + IN.color;

	clip(2.9 - (OUT.color.r + OUT.color.g + OUT.color.b));

	return OUT;
}

technique Technique0
{
	pass Pass0
	{
		CullMode = None;
		FillMode = Solid;

		VertexShader = compile vs_2_0 myVS();
		PixelShader = compile ps_2_0 myPS();
	}
}
