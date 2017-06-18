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
uniform int u_depth;
layout(binding=0) uniform sampler2D texture0;

void main(void)
{
	if (u_depth > 0)
	{
		float z = texture(texture0, vary_TexCoord).r;      // fetch the z-value from our depth texture
		float n = 1.0;                                // the near plane
		float f = 2001.0;                               // the far plane
		float c = (2.0 * n) / (f + n - z * (f - n));  // convert to linear values 
 
		Fragment.rgb = vec3(c);                      // linear
		return;
	}

	Fragment = texture(texture0, vary_TexCoord);
	Fragment.rgb *= Fragment.a;
}
