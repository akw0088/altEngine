struct attr_t
{
	float3 position		: POSITION;
	float2 texCoord0	: TEXCOORD0;
	float2 texCoord1	: TEXCOORD1;
	float3 normal		: NORMAL;
	int color		: COLOR;
	float4 tangent		: TANGENT;
};

struct vary_t
{
	float4 position			: POSITION;
	float2 texCoord0		: TEXCOORD0;
	float2 texCoord1		: TEXCOORD1;
	float3 normal			: NORMAL;
	int color			: COLOR;
	float4 tangent			: TANGENT;
};
uniform float4x4 mvp;

vary_t main(in attr_t attr)
{
	vary_t output;
	float4 pos;
	float temp;

	pos.xyz =  attr.position;
	pos.w = 1.0;

	output.position = mul(pos, mvp);
	output.texCoord0 = attr.texCoord0;
	output.texCoord1 = attr.texCoord1;
	output.normal = attr.normal;
	output.color = attr.color;
	output.tangent = attr.tangent;


	return output;
}

