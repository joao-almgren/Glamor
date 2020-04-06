
//-----------------------------------------------------------------------------
// Effect File Variables
//-----------------------------------------------------------------------------

float4x4 worldViewProj : WorldViewProjection;

texture testTexture;

sampler Sampler = sampler_state
{
    Texture   = (testTexture);
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

//-----------------------------------------------------------------------------
// Vertex Definitions
//-----------------------------------------------------------------------------

struct VS_INPUT
{
    float3 position	: POSITION;
	float2 texture0 : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 hposition : POSITION;
	float2 texture0  : TEXCOORD0;
    float4 color	 : COLOR0;
};

struct PS_OUTPUT
{
	float4 color : COLOR;
};

//-----------------------------------------------------------------------------
// Vertex Shader
//-----------------------------------------------------------------------------

VS_OUTPUT myvs( VS_INPUT IN )
{
    VS_OUTPUT OUT;

	OUT.hposition = mul( worldViewProj, float4(IN.position, 1) );

	OUT.color = float4( 1.0, 1.0, 1.0, 1.0 );

	OUT.texture0 = IN.texture0;

	return OUT;
}

//-----------------------------------------------------------------------------
// Pixel Shader
//-----------------------------------------------------------------------------

PS_OUTPUT myps( VS_OUTPUT IN )
{
    PS_OUTPUT OUT;

	OUT.color = tex2D( Sampler, IN.texture0 ) * IN.color;

    return OUT;
}

//-----------------------------------------------------------------------------
// Effect
//-----------------------------------------------------------------------------

technique Technique0
{
    pass Pass0
    {
		Lighting = FALSE;

		Sampler[0] = (Sampler);

		VertexShader = compile vs_2_0 myvs();
		PixelShader  = compile ps_2_0 myps();
    }
}
