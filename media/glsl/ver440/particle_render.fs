#version 330

uniform sampler2D u_texture0;

// per vertex interpolated program input
in VertexDataOut {
    vec3 att_position;
    vec2 vary_TexCoord;
    vec2 vary_LightCoord;
    vec3 vary_normal;
    flat int vary_color;
    vec4 vary_tangent;
} Vertex;

// Final fragment color output
out vec4 Fragment;

void main()
{
  vec4 vTexColor = texture2D(u_texture0, Vertex.vary_TexCoord);
  Fragment = vec4(vTexColor.xyz, 1.0) * Vertex.vary_tangent;
}