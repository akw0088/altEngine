#version 330

uniform sampler2D u_texture0;

// per vertex interpolated program input
in    vec3 att_position;
in    vec2 vary_TexCoord;
in    vec2 vary_LightCoord;
in    vec3 vary_normal;
in    int vary_color;
in    vec4 vary_tangent;

// Final fragment color output
out vec4 Fragment;

void main()
{
  vec4 vTexColor = texture2D(u_texture0, vary_TexCoord);
  Fragment = vec4(vTexColor.xyz, 1.0) * vary_tangent;
}