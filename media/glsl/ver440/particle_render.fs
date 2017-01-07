#version 330

uniform sampler2D u_texture0;

// per vertex interpolated program input
in    vec3 att_position;
in    vec2 vary_TexCoord;
in    vec2 vary_LightCoord;
in    vec3 vary_normal;
flat in    uint vary_color;
in    vec4 vary_tangent;

// Final fragment color output
out vec4 Fragment;

vec3 unpackColor(float f) 
{
    vec3 color;

    color.r = floor(f / 256.0 / 256.0);
    color.g = floor((f - color.r * 256.0 * 256.0) / 256.0);
    color.b = floor(f - color.r * 256.0 * 256.0 - color.g * 256.0);

    // now we have a vec3 with the 3 components in range [0..256]. Let's normalize it!
    return color / 256.0;
}

void main()
{
	vec4 vTexColor = texture2D(u_texture0, vary_TexCoord);
	float fcolor = vary_color;
	vec3 color = unpackColor(fcolor);


	Fragment = vTexColor;
	Fragment.xyz *= color.xyz;
	Fragment.a *= vary_tangent.x / 2500.0; // life fade out

}