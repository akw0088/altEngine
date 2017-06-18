#version 440 core

// Per vertex attribute input
in vec3 attr_position;
in vec2 attr_TexCoord;
in int attr_color;

// output
out vec4 vary_position;
out vec2 vary_TexCoord;
out int vary_color;

// uniform primitive constant input
uniform mat4		mvp;
uniform int u_depth;

void main(void)
{
	//pass through to fragment shader
	vary_TexCoord = attr_TexCoord;
	vary_color = attr_color;

	gl_Position = mvp * vec4(attr_position, 1.0);
	gl_PointSize = 5.0;
}