#version 440 core

// Per vertex attribute input
in vec3 	attr_position;
in vec2 	attr_TexCoord;
in vec2 	attr_LightCoord;
in vec3 	attr_velocity;
in int		attr_color;
in vec4		attr_tangent;
//in float	attr_life;
//in float	attr_size;
//in float	attr_type;
//in float	pad0;

// interpolated output
out	vec3		ivary_position;
out	vec2		ivary_TexCoord;
out	vec2		ivary_LightCoord;
out	vec3		ivary_velocity;
out	flat int	ivary_color;
out	vec4		ivary_tangent;
//	float		vary_life;
//	float		vary_size;
//	float		vary_type;
//	float		vary_pad0;




void main()
{
	ivary_position = attr_position;
	gl_Position = vec4(ivary_position, 1.0);
	ivary_velocity = attr_velocity;
	ivary_TexCoord = attr_TexCoord;
	ivary_LightCoord = attr_LightCoord;
	ivary_color = attr_color;
	ivary_tangent = attr_tangent;

//	vary_life = attr_life;
//	vary_size = attr_size;
//	vary_type = attr_type;
}