#version 410

// Per vertex attribute input
in vec3 attr_position;
in vec2 attr_TexCoord;
in int attr_color;

// output
out vec4 vary_position;
out vec2 vary_TexCoord;
out int vary_color;

// uniform primitive constant input
uniform vec3	u_color;
uniform float	u_col;
uniform float	u_row;
uniform float	u_xpos;
uniform float	u_ypos;
uniform float	u_scale;
uniform sampler2D	texture0;


void main(void)
{
	//pass through to fragment shader
	ivec2 size = ivec2(256,256);
	vary_TexCoord = attr_TexCoord * (16.0 / size.x);
	vary_TexCoord.s += u_col;
	vary_TexCoord.t += u_row;
	vary_color = attr_color;


	gl_Position = vec4(attr_position * u_scale, 1.0);

	//position
	gl_Position.x += u_xpos;
	gl_Position.y += u_ypos;
}
