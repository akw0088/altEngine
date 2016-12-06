#version 440 core

#define	MAX_LIGHTS 32
#define MAX_SHADOW 4

// Per vertex attribute input
in vec3 attr_position;
in vec2 attr_TexCoord;
in vec2 attr_LightCoord;
in vec3 attr_normal;
in int	attr_color;
in vec3	attr_tangent;

// interpolated output
out VertexData {
	vec3		att_position;
	vec4		vary_position;
	vec2		vary_TexCoord;
	vec2		vary_LightCoord;
	vec3		vary_normal;
	flat int	vary_color;
	vec3		vary_tangent;
	vec4 shadowpos[24];
} Vertex;

// uniform primitive constant input
uniform vec3	u_position[MAX_LIGHTS];
uniform vec4	u_color[MAX_LIGHTS];
uniform int	u_num_lights;
uniform mat4	mvp;




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
	//pass through to fragment shader
	Vertex.vary_normal = attr_normal;
	Vertex.vary_TexCoord = attr_TexCoord;
	Vertex.vary_LightCoord = attr_LightCoord;
	Vertex.vary_color = attr_color;
	Vertex.vary_tangent = attr_tangent;

	gl_Position = mvp * vec4(attr_position, 1.0);
	Vertex.vary_position = gl_Position;
	Vertex.att_position = attr_position;


	vec4 bias_right;
	vec4 bias_up;
	vec4 bias_forward;
	vec4 bias_translation;

	bias_right		= vec4(0.5f, 0.0f, 0.0f, 0.0f);
	bias_up			= vec4(0.0f, 0.5f, 0.0f, 0.0f);
	bias_forward		= vec4(0.0f, 0.0f, 0.5f, 0.0f);
	bias_translation	= vec4(0.5f, 0.5f, 0.5f, 1.0f);
	mat4 bias = mat4(bias_right, bias_up, bias_forward, bias_translation);


	vec3 right;
	vec3 up;
	vec3 forward;
	vec3 translation;



	for(int i = 0; i < min(MAX_SHADOW, u_num_lights); i++)
	{
		mright(u_position[i], right, up, forward, translation);
		mat4 mat_right = mat4(		right.x, up.x, forward.x, 0.0f,
				 		right.y, up.y, forward.y, 0.0f,
						right.z, up.z, forward.z, 0.0f,
						translation.x, translation.y, translation.z, 1.0f);
	
		mleft(u_position[i], right, up, forward, translation);
		mat4 mat_left = mat4(		right.x, up.x, forward.x, 0.0f,
				 		right.y, up.y, forward.y, 0.0f,
						right.z, up.z, forward.z, 0.0f,
						translation.x, translation.y, translation.z, 1.0f);
	
		mforward(u_position[i], right, up, forward, translation);
		mat4 mat_forward = mat4(	right.x, up.x, forward.x, 0.0f,
				 		right.y, up.y, forward.y, 0.0f,
						right.z, up.z, forward.z, 0.0f,
						translation.x, translation.y, translation.z, 1.0f);
	
		mbackward(u_position[i], right, up, forward, translation);
		mat4 mat_backward = mat4(	right.x, up.x, forward.x, 0.0f,
				 		right.y, up.y, forward.y, 0.0f,
						right.z, up.z, forward.z, 0.0f,
						translation.x, translation.y, translation.z, 1.0f);
	
		mtop(u_position[i], right, up, forward, translation);
		mat4 mat_top = mat4(		right.x, up.x, forward.x, 0.0f,
				 		right.y, up.y, forward.y, 0.0f,
						right.z, up.z, forward.z, 0.0f,
						translation.x, translation.y, translation.z, 1.0f);
	
		mbottom(u_position[i], right, up, forward, translation);
		mat4 mat_bottom = mat4(		right.x, up.x, forward.x, 0.0f,
				 		right.y, up.y, forward.y, 0.0f,
						right.z, up.z, forward.z, 0.0f,
						translation.x, translation.y, translation.z, 1.0f);


		Vertex.shadowpos[0 + 6 * i] = (mat_right * bias)	* vec4(attr_position, 1.0);
		Vertex.shadowpos[1 + 6 * i] = (mat_left * bias)		* vec4(attr_position, 1.0);
		Vertex.shadowpos[2 + 6 * i] = (mat_top * bias)		* vec4(attr_position, 1.0);
		Vertex.shadowpos[3 + 6 * i] = (mat_bottom * bias)	* vec4(attr_position, 1.0);
		Vertex.shadowpos[4 + 6 * i] = (mat_forward * bias)	* vec4(attr_position, 1.0);
		Vertex.shadowpos[5 + 6 * i] = (mat_backward * bias)	* vec4(attr_position, 1.0);
	}

}