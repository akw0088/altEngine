#version 330

uniform sampler2D u_texture0;

// per vertex interpolated program input
in    vec3 att_position;
in    vec2 vary_TexCoord;
in    vec2 vary_LightCoord;
in    vec3 vary_normal;
flat in   highp uint vary_color;
in    vec4 vary_tangent;

// Final fragment color output
out vec4 Fragment;

vec4 unpackColor(uint f)
{
	vec4 color;

	color.a = int(f >> 24) / 256; 
	color.b = int(f >> 16) / 256;
	color.g = int(f >> 8) / 256;
	color.r = int(f) % 256;
	return color;
}

void main()
{
	vec4 vTexColor = texture2D(u_texture0, vary_TexCoord);
	vec4 color = unpackColor(vary_color);


	Fragment = vTexColor;
//	Fragment.xyz *= color.xyz;
	Fragment.a *= vary_tangent.x / 5.0; // life fade out

}