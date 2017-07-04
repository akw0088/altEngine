#define MAX_LIGHTS 64

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

uniform float4x4	mvp;

uniform sampler		tex0, tex1, tex2, tex3, tex4, tex5, tex6, tex7, texture_lightmap, texture_normalmap;


uniform float3		u_position[MAX_LIGHTS];
uniform float4		u_color[MAX_LIGHTS];
uniform int		u_num_lights;


vary_t main(in attr_t attr)
{
	vary_t output;

	float4 vpos = float4(attr.position, 1.0);

	output.position = mul(vpos, mvp);
	output.texCoord0 = attr.texCoord0;
	output.texCoord1 = attr.texCoord1;
	output.normal = attr.normal;
	output.color = attr.color;
	output.tangent = attr.tangent;

	return output;
}


