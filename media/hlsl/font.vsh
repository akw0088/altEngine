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
	float4 position		: POSITION;
	float2 texCoord0	: TEXCOORD0;
	float2 texCoord1	: TEXCOORD1;
	float3 normal		: NORMAL;
	int color		: COLOR0;
	float4 tangent		: TANGENT;
};

uniform float	u_col;
uniform float	u_row;
uniform float	u_xpos;
uniform float	u_ypos;
uniform float	u_scale;


vary_t main(in attr_t attr)
{
	vary_t output;
	float4 vpos = float4(attr.position, 1.0);

	output.position.x = vpos.x * u_scale - 1.0;
	output.position.y = vpos.y * u_scale + 1.0;
	output.position.z = vpos.z;
	output.position.w = 1.0f;
	output.position.x += u_xpos;
	output.position.y -= u_ypos;



	output.texCoord0 = attr.texCoord0 * (16.0 / 256.0);
	output.texCoord0.x += u_col;
	output.texCoord0.y += u_row;
	output.texCoord1 = attr.texCoord0 * (16.0 / 256.0);
	output.texCoord1.x += u_col;
	output.texCoord1.y += u_row;
	output.normal = attr.normal;
	output.tangent = attr.tangent;
	output.color = attr.color;
	return output;
}


