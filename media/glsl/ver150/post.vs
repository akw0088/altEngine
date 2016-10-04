#version 150

// Per vertex attribute input
in vec3 attr_position;
in vec2 attr_TexCoord;

// output
out vec4 vary_position;
out vec2 vary_TexCoord;

void main(void)
{
	//pass through to fragment shader
	vary_TexCoord = attr_TexCoord;
	gl_Position = vec4(attr_position, 1.0);
}