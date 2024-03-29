#version 410
#define	MAX_LIGHTS 32


// Per vertex attribute input
in vec3 attr_position;
in vec2 attr_TexCoord;
in vec2 attr_LightCoord;
in vec3 attr_normal;
in int	attr_color;
in vec3	attr_tangent;

// interpolated output
out VertexData {
	vec4		vary_position;
	vec2		vary_TexCoord;
	vec2		vary_LightCoord;
	vec3		vary_normal;
	flat int	vary_color;
	vec3		vary_tangent;
} Vertex;

// uniform primitive constant input
uniform vec3	u_position[MAX_LIGHTS];
uniform vec4	u_color[MAX_LIGHTS];
uniform int	u_num_lights;
uniform mat4	mvp;

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
}
