#version 440 core

// Per vertex attribute input
in vec3 attr_position;
in vec2 attr_TexCoord;
in vec2 attr_LightCoord;
in vec3 attr_normal;
in uint	attr_color;
in vec4	attr_tangent;

// interpolated output
out	vec3		att_position;
out	vec2		vary_TexCoord;
out	vec2		vary_LightCoord;
out	vec3		vary_normal;
flat out	uint	vary_color;
out	vec4		vary_tangent;

uniform mat4 u_mvp;

void main()
{
	gl_Position = u_mvp * vec4(attr_position, 1.0);
	att_position = gl_Position.xyz;
	vary_color = attr_color;
	vary_normal = attr_normal; 
	vary_tangent = attr_tangent; 

// Will pass in normal.xyz
//	size
//	life
//	type
}