#version 440 core

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
	vec2		vary_TexCoord;
	vec2		vary_LightCoord;
	vec3		vary_normal;
	flat int	vary_color;
	vec4		vary_tangent;
} Vertex;


void main()
{
	gl_Position = vec4(attr_position, 1.0);
	Vertex.att_position = gl_Position.xyz;
	Vertex.vary_color = attr_color;
	Vertex.vary_normal = attr_normal; 
	Vertex.vary_tangent = attr_tangent; 

// Will pass in normal.xyz
//	size
//	life
//	type
}