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

uniform mat4	shadow_matrix[18];
uniform vec2	u_tcmod_scroll[8];
uniform vec2	u_tcmod_scale[8];
uniform float	u_tcmod_sin[8];
uniform float	u_tcmod_cos[8];

void main(void)
{

	mat2 mRot0 = mat2( u_tcmod_cos[0], -u_tcmod_sin[0], u_tcmod_sin[0],  u_tcmod_cos[0]);
	mat2 mRot1 = mat2( u_tcmod_cos[1], -u_tcmod_sin[1], u_tcmod_sin[1],  u_tcmod_cos[1]);
	mat2 mRot2 = mat2( u_tcmod_cos[2], -u_tcmod_sin[2], u_tcmod_sin[2],  u_tcmod_cos[2]);
	mat2 mRot3 = mat2( u_tcmod_cos[3], -u_tcmod_sin[3], u_tcmod_sin[3],  u_tcmod_cos[3]);


	vec2 bias = vec2(0.5, -0.5);
	Vertex.vary_TexCoord0 = ((u_tcmod_scale[0] * (attr_TexCoord - bias)) * mRot0) + u_tcmod_scroll[0] + bias;
	Vertex.vary_TexCoord1 = ((u_tcmod_scale[1] * (attr_TexCoord - bias)) * mRot1) + u_tcmod_scroll[1] + bias;
	Vertex.vary_TexCoord2 = ((u_tcmod_scale[2] * (attr_TexCoord - bias)) * mRot2) + u_tcmod_scroll[2] + bias;
	Vertex.vary_TexCoord3 = ((u_tcmod_scale[3] * (attr_TexCoord - bias)) * mRot3) + u_tcmod_scroll[3] + bias;

	//pass through to fragment shader
	Vertex.vary_normal = attr_normal;
	Vertex.vary_TexCoord = attr_TexCoord;
	Vertex.vary_LightCoord = attr_LightCoord;
	Vertex.vary_color = attr_color;
	Vertex.vary_tangent = attr_tangent;

	gl_Position = mvp * vec4(attr_position, 1.0);
	Vertex.att_position = attr_position;
	Vertex.vary_position = gl_Position;

	Vertex.shadowpos[0] = shadow_matrix[0] * vec4(attr_position, 1.0);
	Vertex.shadowpos[1] = shadow_matrix[1] * vec4(attr_position, 1.0);
	Vertex.shadowpos[2] = shadow_matrix[2] * vec4(attr_position, 1.0);
	Vertex.shadowpos[3] = shadow_matrix[3] * vec4(attr_position, 1.0);
	Vertex.shadowpos[4] = shadow_matrix[4] * vec4(attr_position, 1.0);
	Vertex.shadowpos[5] = shadow_matrix[5] * vec4(attr_position, 1.0);
	Vertex.shadowpos[6] = shadow_matrix[6] * vec4(attr_position, 1.0);
	Vertex.shadowpos[7] = shadow_matrix[7] * vec4(attr_position, 1.0);
	Vertex.shadowpos[8] = shadow_matrix[8] * vec4(attr_position, 1.0);
	Vertex.shadowpos[9] = shadow_matrix[9] * vec4(attr_position, 1.0);
	Vertex.shadowpos[10] = shadow_matrix[10] * vec4(attr_position, 1.0);
	Vertex.shadowpos[11] = shadow_matrix[11] * vec4(attr_position, 1.0);
	Vertex.shadowpos[12] = shadow_matrix[12] * vec4(attr_position, 1.0);
	Vertex.shadowpos[13] = shadow_matrix[13] * vec4(attr_position, 1.0);
	Vertex.shadowpos[14] = shadow_matrix[14] * vec4(attr_position, 1.0);
	Vertex.shadowpos[15] = shadow_matrix[15] * vec4(attr_position, 1.0);
	Vertex.shadowpos[16] = shadow_matrix[16] * vec4(attr_position, 1.0);
	Vertex.shadowpos[17] = shadow_matrix[17] * vec4(attr_position, 1.0);

}