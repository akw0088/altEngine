#define MAX_LIGHTS 4

struct attr_t
{
	float4 position		: POSITION;
	float4 color		: COLOR0; 
	float4 normal		: NORMAL;
	float2 texCoord0	: TEXCOORD0;
	float2 texCoord1	: TEXCOORD1;
	float2 texCoord2	: TEXCOORD2;
};

struct vary_t
{
	float4 position			: POSITION;
	float4 color			: COLOR0;
	float2 texCoord0		: TEXCOORD0;
	float2 texCoord1		: TEXCOORD1;
	float2 texCoord2		: TEXCOORD2;

	float4 normal			: TEXCOORD3;
	float4 light[MAX_LIGHTS]	: TEXCOORD4;
};

uniform float3		u_position[MAX_LIGHTS];
uniform float4		u_color[MAX_LIGHTS];
uniform int		u_num_lights;
uniform float4x4	mvp;
uniform sampler tex0, tex1, tex2;


vary_t main(in attr_t attr)
{
	vary_t output;

	output.position = mul(attr.position, mvp);
	output.position.w = 1.0; 
	output.texCoord0 = attr.texCoord0;
	output.texCoord1 = attr.texCoord1;
	output.texCoord2 = attr.texCoord2;
	output.color = attr.color;
	output.normal = attr.normal;

	for(int i = 0; i < u_num_lights; i++)
	{
		float4 csPos = mul(float4(u_position[i].x, u_position[i].y, u_position[i].z, 1.0), mvp);
		output.light[i].xyz = normalize(csPos.xyz - output.position.xyz);
		output.light[i].w = length(csPos.xyz - output.position.xyz);
	}
	return output;
}


