struct attr_t
{
	float3 position		: POSITION;
	float2 texCoord0	: TEXCOORD0;
	float2 texCoord1	: TEXCOORD1;
	float3 normal		: NORMAL;
	uint   color		: COLOR;
	float4 tangent		: TANGENT;
};

struct vary_t
{
	float4 position			: SV_POSITION;
	float2 texCoord0		: TEXCOORD0;
	float2 texCoord1		: TEXCOORD1;
	float3 normal			: NORMAL;
	uint   color			: COLOR;
	float4 tangent			: TANGENT;
};
uniform float4x4 mvp;
Texture2D  tex0; 

SamplerState DiffuseSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

vary_t VS(in attr_t attr)
{
	vary_t output;
	float4 pos;
	float temp;

	pos.xyz =  attr.position;
	pos.w = 1.0;

	output.position = mul(mvp, pos);
	output.texCoord0 = attr.texCoord0;
	output.texCoord1 = attr.texCoord1;
	output.normal = attr.normal;
	output.color = attr.color;
	output.tangent = attr.tangent;


	return output;
}




float4 PS( in vary_t vary ) : SV_Target
{
	float4 color;

	color = tex0.Sample(DiffuseSampler, vary.texCoord0);
	return color;
}

