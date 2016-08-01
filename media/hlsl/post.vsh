struct attr_t
{
	float4 position		: POSITION;
	float4 color		: COLOR0; 
	float2 texCoord0	: TEXCOORD0;
	float2 texCoord1	: TEXCOORD1;
};

struct vary_t
{
	float4 position		: POSITION;
	float4 color		: COLOR0;
	float2 texCoord0	: TEXCOORD0;
	float2 texCoord1	: TEXCOORD1;
};

vary_t main(in attr_t attr)
{
	vary_t output;

	output.position = attr.position; 
	output.texCoord0 = attr.texCoord0;
	output.texCoord1 = attr.texCoord1;
	output.color = attr.color;
	return output;
}