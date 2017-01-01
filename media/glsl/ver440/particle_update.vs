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
out	vec3		vary_position;
out	vec2		vary_TexCoord;
out	vec2		vary_LightCoord;
out	vec3		vary_velocity;
out	flat int	vary_color;
out	vec4		vary_tangent;
//	float		vary_life;
//	float		vary_size;
//	float		vary_type;
//	float		vary_pad0;




void main()
{
	vary_position = attr_position;
	gl_Position = vary_position;
	vary_velocity = attr_velocity;
	vary_color = attr_color;
	vary_tangent = attr_tangent;

//	vary_life = attr_life;
//	vary_size = attr_size;
//	vary_type = attr_type;
}