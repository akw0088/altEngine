#version 440 core
#define	MAX_LIGHTS 64
#define MAX_SHADOW 1

// Per vertex attribute input
in vec3 attr_position;
in vec2 attr_TexCoord;
in vec2 attr_LightCoord;
in vec3 attr_normal;
in int	attr_color;
in vec4	attr_tangent;

// interpolated output
out VertexData {
	vec3		att_position;
	vec4		vary_position;
	vec2		vary_TexCoord;
	vec2		vary_TexCoord0;
	vec2		vary_TexCoord1;
	vec2		vary_TexCoord2;
	vec2		vary_TexCoord3;
	vec2		vary_LightCoord;
	vec3		vary_normal;
	flat int	vary_color;
	vec4		vary_tangent;
	vec4 shadowpos[24];
} Vertex;

// uniform primitive constant input
uniform vec4	u_position[MAX_LIGHTS]; // alph is attenuation cofactor
uniform vec4	u_color[MAX_LIGHTS]; //alpha is intensity
uniform int	u_num_lights;
uniform mat4	mvp;

uniform mat4	shadow_matrix0;
uniform mat4	shadow_matrix1;
uniform mat4	shadow_matrix2;
uniform mat4	shadow_matrix3;
uniform mat4	shadow_matrix4;
uniform mat4	shadow_matrix5;
uniform mat4	shadow_matrix6;
uniform mat4	shadow_matrix7;
uniform mat4	shadow_matrix8;
uniform mat4	shadow_matrix9;
uniform mat4	shadow_matrix10;
uniform mat4	shadow_matrix11;
uniform mat4	shadow_matrix12;
uniform mat4	shadow_matrix13;
uniform mat4	shadow_matrix14;
uniform mat4	shadow_matrix15;
uniform mat4	shadow_matrix16;
uniform mat4	shadow_matrix17;

uniform vec2		u_tcmod_scroll0;
uniform vec2		u_tcmod_scroll1;
uniform vec2		u_tcmod_scroll2;
uniform vec2		u_tcmod_scroll3;
uniform vec2		u_tcmod_scroll4;
uniform vec2		u_tcmod_scroll5;
uniform vec2		u_tcmod_scroll6;
uniform vec2		u_tcmod_scroll7;

uniform vec2		u_tcmod_scale0;
uniform vec2		u_tcmod_scale1;
uniform vec2		u_tcmod_scale2;
uniform vec2		u_tcmod_scale3;
uniform vec2		u_tcmod_scale4;
uniform vec2		u_tcmod_scale5;
uniform vec2		u_tcmod_scale6;
uniform vec2		u_tcmod_scale7;

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

void main(void)
{

	mat2 mRot0 = mat2( u_tcmod_cos0, -u_tcmod_sin0, u_tcmod_sin0,  u_tcmod_cos0);
	mat2 mRot1 = mat2( u_tcmod_cos1, -u_tcmod_sin1, u_tcmod_sin1,  u_tcmod_cos1);
	mat2 mRot2 = mat2( u_tcmod_cos2, -u_tcmod_sin2, u_tcmod_sin2,  u_tcmod_cos2);
	mat2 mRot3 = mat2( u_tcmod_cos3, -u_tcmod_sin3, u_tcmod_sin3,  u_tcmod_cos3);


	vec2 bias = vec2(0.5, -0.5);
	Vertex.vary_TexCoord0 = ((u_tcmod_scale0 * (attr_TexCoord - bias)) * mRot0) + u_tcmod_scroll0 + bias;
	Vertex.vary_TexCoord1 = ((u_tcmod_scale1 * (attr_TexCoord - bias)) * mRot1) + u_tcmod_scroll1 + bias;
	Vertex.vary_TexCoord2 = ((u_tcmod_scale2 * (attr_TexCoord - bias)) * mRot2) + u_tcmod_scroll2 + bias;
	Vertex.vary_TexCoord3 = ((u_tcmod_scale3 * (attr_TexCoord - bias)) * mRot3) + u_tcmod_scroll3 + bias;

	//pass through to fragment shader
	Vertex.vary_normal = attr_normal;
	Vertex.vary_TexCoord = attr_TexCoord;
	Vertex.vary_LightCoord = attr_LightCoord;
	Vertex.vary_color = attr_color;
	Vertex.vary_tangent = attr_tangent;

	gl_Position = mvp * vec4(attr_position, 1.0);
	Vertex.att_position = attr_position;
	Vertex.vary_position = gl_Position;

	Vertex.shadowpos[0] = shadow_matrix0 * vec4(attr_position, 1.0);
	Vertex.shadowpos[1] = shadow_matrix1 * vec4(attr_position, 1.0);
	Vertex.shadowpos[2] = shadow_matrix2 * vec4(attr_position, 1.0);
	Vertex.shadowpos[3] = shadow_matrix3 * vec4(attr_position, 1.0);
	Vertex.shadowpos[4] = shadow_matrix4 * vec4(attr_position, 1.0);
	Vertex.shadowpos[5] = shadow_matrix5 * vec4(attr_position, 1.0);
	Vertex.shadowpos[6] = shadow_matrix6 * vec4(attr_position, 1.0);
	Vertex.shadowpos[7] = shadow_matrix7 * vec4(attr_position, 1.0);
	Vertex.shadowpos[8] = shadow_matrix8 * vec4(attr_position, 1.0);
	Vertex.shadowpos[9] = shadow_matrix9 * vec4(attr_position, 1.0);
	Vertex.shadowpos[10] = shadow_matrix10 * vec4(attr_position, 1.0);
	Vertex.shadowpos[11] = shadow_matrix11 * vec4(attr_position, 1.0);
	Vertex.shadowpos[12] = shadow_matrix12 * vec4(attr_position, 1.0);
	Vertex.shadowpos[13] = shadow_matrix13 * vec4(attr_position, 1.0);
	Vertex.shadowpos[14] = shadow_matrix14 * vec4(attr_position, 1.0);
	Vertex.shadowpos[15] = shadow_matrix15 * vec4(attr_position, 1.0);
	Vertex.shadowpos[16] = shadow_matrix16 * vec4(attr_position, 1.0);
	Vertex.shadowpos[17] = shadow_matrix17 * vec4(attr_position, 1.0);

}