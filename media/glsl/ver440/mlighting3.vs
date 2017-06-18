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
	vec4 shadowpos[8];
} Vertex;

// uniform primitive constant input
uniform vec4	u_position[MAX_LIGHTS]; // alph is attenuation cofactor
uniform vec4	u_color[MAX_LIGHTS]; //alpha is intensity
uniform int	u_num_lights;
uniform mat4	mvp;

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

void mright(in vec3 position, out vec3 right, out vec3 up, out vec3 forward, out vec3 translation)
{
	right		= vec3(-1.0f, 0.0f, 0.0f);
	up		= vec3(0.0f, 1.0f, 0.0f);
	forward		= vec3(0.0f, 0.0f, -1.0f);
	translation	= vec3(dot(-right, position), dot(-up, position), dot(-forward, position));
}

void mleft(in vec3 position, out vec3 right, out vec3 up, out vec3 forward, out vec3 translation)
{
	right		= vec3(0.0f, 0.0f, -1.0f);
	up		= vec3(0.0f, 1.0f, 0.0f);
	forward		= vec3(1.0f, 0.0f, 0.0f);
	translation	= vec3(dot(-right, position), dot(-up, position), dot(-forward, position));
}

void mforward(in vec3 position, out vec3 right, out vec3 up, out vec3 forward, out vec3 translation)
{
	right		= vec3(1.0f, 0.0f, 0.0f);
	up		= vec3(0.0f, 1.0f, 0.0f);
	forward		= vec3(0.0f, 0.0f, 1.0f);
	translation	= vec3(dot(-right, position), dot(-up, position), dot(-forward, position));
}

void mbackward(in vec3 position, out vec3 right, out vec3 up, out vec3 forward, out vec3 translation)
{
	right		= vec3(0.0f, 0.0f, 1.0f);
	up		= vec3(0.0f, 1.0f, 0.0f);
	forward		= vec3(-1.0f, 0.0f, 0.0f);
	translation	= vec3(dot(-right, position), dot(-up, position), dot(-forward, position));
}

void mtop(in vec3 position, out vec3 right, out vec3 up, out vec3 forward, out vec3 translation)
{
	right		= vec3(1.0f, 0.0f, 0.0f);
	up		= vec3(0.0f, 0.0f, 1.0f);
	forward		= vec3(0.0f, -1.0f, 0.0f);
	translation	= vec3(dot(-right, position), dot(-up, position), dot(-forward, position));
}

void mbottom(in vec3 position, out vec3 right, out vec3 up, out vec3 forward, out vec3 translation)
{
	right		= vec3(1.0f, 0.0f, 0.0f);
	up		= vec3(0.0f, 0.0f, -1.0f);
	forward		= vec3(0.0f, 1.0f, 0.0f);
	translation	= vec3(dot(-right, position), dot(-up, position), dot(-forward, position));
}

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


	vec3 right;
	vec3 up;
	vec3 forward;
	vec3 translation;



	for(int i = 0; i < min(MAX_SHADOW, u_num_lights); i++)
	{
		mright(u_position[i].xyz, right, up, forward, translation);
		mat4 mat_right = mat4(		right.x, up.x, forward.x, 0.0f,
				 		right.y, up.y, forward.y, 0.0f,
						right.z, up.z, forward.z, 0.0f,
						translation.x, translation.y, translation.z, 1.0f);
	
		mleft(u_position[i].xyz, right, up, forward, translation);
		mat4 mat_left = mat4(		right.x, up.x, forward.x, 0.0f,
				 		right.y, up.y, forward.y, 0.0f,
						right.z, up.z, forward.z, 0.0f,
						translation.x, translation.y, translation.z, 1.0f);
	
		mforward(u_position[i].xyz, right, up, forward, translation);
		mat4 mat_forward = mat4(	right.x, up.x, forward.x, 0.0f,
				 		right.y, up.y, forward.y, 0.0f,
						right.z, up.z, forward.z, 0.0f,
						translation.x, translation.y, translation.z, 1.0f);
	
		mbackward(u_position[i].xyz, right, up, forward, translation);
		mat4 mat_backward = mat4(	right.x, up.x, forward.x, 0.0f,
				 		right.y, up.y, forward.y, 0.0f,
						right.z, up.z, forward.z, 0.0f,
						translation.x, translation.y, translation.z, 1.0f);
	
		mtop(u_position[i].xyz, right, up, forward, translation);
		mat4 mat_top = mat4(		right.x, up.x, forward.x, 0.0f,
				 		right.y, up.y, forward.y, 0.0f,
						right.z, up.z, forward.z, 0.0f,
						translation.x, translation.y, translation.z, 1.0f);
	
		mbottom(u_position[i].xyz, right, up, forward, translation);
		mat4 mat_bottom = mat4(		right.x, up.x, forward.x, 0.0f,
				 		right.y, up.y, forward.y, 0.0f,
						right.z, up.z, forward.z, 0.0f,
						translation.x, translation.y, translation.z, 1.0f);


		Vertex.shadowpos[0 + 6 * i] = (mat_right)	* vec4(attr_position, 1.0);
		Vertex.shadowpos[1 + 6 * i] = (mat_left)	* vec4(attr_position, 1.0);
		Vertex.shadowpos[2 + 6 * i] = (mat_top)		* vec4(attr_position, 1.0);
		Vertex.shadowpos[3 + 6 * i] = (mat_bottom)	* vec4(attr_position, 1.0);
		Vertex.shadowpos[4 + 6 * i] = (mat_forward)	* vec4(attr_position, 1.0);
		Vertex.shadowpos[5 + 6 * i] = (mat_backward)	* vec4(attr_position, 1.0);
	}
}