#version 410

// Per vertex interpolated program input
in vec4 vary_position;
flat in int vary_color;
in vec2 vary_TexCoord;

// Final fragment color output
out vec4 Fragment;

// uniform primitive constant input
uniform vec3	u_color;
uniform float	u_col;
uniform float	u_row;
uniform float	u_xpos;
uniform float	u_ypos;

uniform sampler2D texture0;


void main(void)
{
	Fragment = texture(texture0, vary_TexCoord);
	Fragment.rgb = u_color * Fragment.rgb * Fragment.a;
}
