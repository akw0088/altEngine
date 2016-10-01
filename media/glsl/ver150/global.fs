#version 150

// Per vertex interpolated program input
in vec4 vary_position;
flat in int vary_color;
in vec2 vary_TexCoord;

// Final fragment color output
out vec4 Fragment;

// constant program input
uniform mat4		mvp;
uniform sampler2D	texture0;

void main(void)
{
	Fragment = texture2D(texture0, vary_TexCoord);
//	Fragment.rgb *= Fragment.a;
}
