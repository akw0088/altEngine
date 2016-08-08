#version 440 core

// Per vertex interpolated program input
in vec4 vary_position;
flat in int vary_color;
in vec2 vary_TexCoord;

// Final fragment color output
layout(location = 0) out vec4 Fragment;


//out vec4 Fragment;

// constant program input
uniform mat4		mvp;
layout(binding=0) uniform sampler2D texture0;

void main(void)
{
	Fragment = texture(texture0, vary_TexCoord);
//	Fragment.rgb *= Fragment.a;
}
