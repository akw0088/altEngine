#version 440 core

// Per vertex attribute input
in vec3 attr_position;
in vec2 attr_TexCoord;
in vec2 attr_LightCoord;
in vec3 attr_normal;
in uint	attr_color;
in vec4	attr_tangent;

// interpolated output
out	vec3		iatt_position;
out	vec2		ivary_TexCoord;
out	vec2		ivary_LightCoord;
out	vec3		ivary_normal;
flat out highp uint	ivary_color;
out	vec4		ivary_tangent;

uniform mat4 u_mvp;
uniform float u_xshift;
uniform float u_yshift;


void main()
{
	gl_Position = vec4(	attr_position.x + u_xshift,
				attr_position.y + u_yshift,
				attr_position.z,
				1.0);
	iatt_position = gl_Position.xyz;
	ivary_color = attr_color;
	ivary_normal = attr_normal; 
	ivary_tangent = attr_tangent; 

// Will pass in normal.xyz
//	size
//	life
//	type
}