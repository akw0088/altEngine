#define MAX_LIGHTS 4

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
	float2 texCoord2		: TEXCOORD2;
	float2 texCoord3		: TEXCOORD3;
	float3 normal			: NORMAL;
	int color			: COLOR;
	float4 tangent			: TANGENT;
};

uniform float4x4	mvp;
uniform sampler		tex0, tex1, tex2, tex3;
uniform float3		u_position[MAX_LIGHTS];
uniform float4		u_color[MAX_LIGHTS];
uniform int		u_num_lights;

uniform float2		u_tcmod_scroll0;
uniform float2		u_tcmod_scroll1;
uniform float2		u_tcmod_scroll2;
uniform float2		u_tcmod_scroll3;
uniform float2		u_tcmod_scroll4;
uniform float2		u_tcmod_scroll5;
uniform float2		u_tcmod_scroll6;
uniform float2		u_tcmod_scroll7;

uniform float2		u_tcmod_scale0;
uniform float2		u_tcmod_scale1;
uniform float2		u_tcmod_scale2;
uniform float2		u_tcmod_scale3;
uniform float2		u_tcmod_scale4;
uniform float2		u_tcmod_scale5;
uniform float2		u_tcmod_scale6;
uniform float2		u_tcmod_scale7;

uniform float	u_tcmod_sin0;
uniform float	u_tcmod_sin1;
uniform float	u_tcmod_sin2;
uniform float	u_tcmod_sin3;
uniform float	u_tcmod_sin4;
uniform float	u_tcmod_sin5;
uniform float	u_tcmod_sin6;
uniform float	u_tcmod_sin7;


uniform float	u_tcmod_cos0;
uniform float	u_tcmod_cos1;
uniform float	u_tcmod_cos2;
uniform float	u_tcmod_cos3;
uniform float	u_tcmod_cos4;
uniform float	u_tcmod_cos5;
uniform float	u_tcmod_cos6;
uniform float	u_tcmod_cos7;


vary_t main(in attr_t attr)
{
	vary_t output;

	float4 vpos = float4(attr.position, 1.0);



	float2x2 mRot0 = float2x2( u_tcmod_cos0, -u_tcmod_sin0, u_tcmod_sin0,  u_tcmod_cos0);
	float2x2 mRot1 = float2x2( u_tcmod_cos1, -u_tcmod_sin1, u_tcmod_sin1,  u_tcmod_cos1);
	float2x2 mRot2 = float2x2( u_tcmod_cos2, -u_tcmod_sin2, u_tcmod_sin2,  u_tcmod_cos2);
	float2x2 mRot3 = float2x2( u_tcmod_cos3, -u_tcmod_sin3, u_tcmod_sin3,  u_tcmod_cos3);


	float2 bias = float2(0.5, -0.5);
	float2 bias0 = mul(u_tcmod_scale0, attr.texCoord0 - bias);
        float2 bias1 = mul(u_tcmod_scale1, attr.texCoord0 - bias);
        float2 bias2 = mul(u_tcmod_scale2, attr.texCoord0 - bias);
        float2 bias3 = mul(u_tcmod_scale3, attr.texCoord0 - bias);


	float2 temp0 = mul(mRot0, bias0 );
        float2 temp1 = mul(mRot1, bias1 );
        float2 temp2 = mul(mRot2, bias2 );
        float2 temp3 = mul(mRot3, bias3 );


	output.texCoord0 = temp0 + u_tcmod_scroll0 + bias;
	output.texCoord1 = temp1 + u_tcmod_scroll1 + bias;
	output.texCoord2 = temp2 + u_tcmod_scroll2 + bias;
	output.texCoord3 = temp3 + u_tcmod_scroll3 + bias;

	output.position = mul(vpos, mvp);
//	output.texCoord0 = attr.texCoord0;
//	output.texCoord1 = attr.texCoord1;
	output.normal = attr.normal;
	output.color = attr.color;
	output.tangent = attr.tangent;


	return output;
}


