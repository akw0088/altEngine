struct attr_t
{
	float4 position		: POSITION;
	float4 color		: COLOR0; 
	float4 Normal		: NORMAL;
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

uniform float	u_col;
uniform float	u_row;
uniform float	u_xpos;
uniform float	u_ypos;
uniform float	u_scale;

vary_t main(in attr_t attr)
{
	vary_t output;

	output.position = attr.position * u_scale;
	output.position.x = u_xpos;
	output.position.y = u_ypos;
	output.position.z = 0.0;
	output.position.w = 1.0f;
	output.texCoord0 = attr.texCoord0 * (16.0 / 256.0);
	output.texCoord0.x += u_col;
	output.texCoord0.y += u_row;
	output.texCoord1 = attr.texCoord0 * (16.0 / 256.0);
	output.texCoord1.x += u_col;
	output.texCoord1.y += u_row;
	output.color = attr.color;
	return output;
}


