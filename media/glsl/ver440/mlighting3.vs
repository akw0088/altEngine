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
	vec2		vary_newTexCoord[4];
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
	vec2 bias = vec2(0.5, -0.5);

	for(int i = 0; i < 4; i++)
	{
		mat2 mRot0 = mat2( u_tcmod_cos[i], -u_tcmod_sin[i], u_tcmod_sin[i],  u_tcmod_cos[i]);
		Vertex.vary_newTexCoord[i] = ((u_tcmod_scale[i] * (attr_TexCoord - bias)) * mRot0) + u_tcmod_scroll[i] + bias;
	}

	//pass through to fragment shader
	Vertex.vary_normal = attr_normal;
	Vertex.vary_TexCoord = attr_TexCoord;
	Vertex.vary_LightCoord = attr_LightCoord;
	Vertex.vary_color = attr_color;
	Vertex.vary_tangent = attr_tangent;

	gl_Position = mvp * vec4(attr_position, 1.0);
	Vertex.att_position = attr_position;
	Vertex.vary_position = gl_Position;


	for(int i = 0; i < 18; i++)
	{
		Vertex.shadowpos[i] = shadow_matrix[i] * vec4(attr_position, 1.0);
	}
}