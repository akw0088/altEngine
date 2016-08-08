#version 440 core
#define	MAX_LIGHTS 16

// per vertex interpolated program input
in VertexDataOut {
    vec4 vary_position; // vertex position
    vec2 vary_TexCoord;
    vec2 vary_LightCoord;
    vec3 vary_normal;
    flat int vary_color;
    vec3 vary_tangent;
} Vertex;

// Final fragment color output
out vec4 Fragment;

// constant program input
uniform vec3		u_position[MAX_LIGHTS]; // light position, world coordinates
uniform vec4		u_color[MAX_LIGHTS];
uniform int		u_num_lights;
uniform mat4		mvp[6];
layout(binding=0) uniform sampler2D texture0;
layout(binding=1) uniform sampler2D texture1;
layout(binding=2) uniform sampler2D texture2;
uniform int		far_plane;


// was originally varying, but couldnt pass through geometry shader
vec4 vary_light;

void main()
{
    // get distance between fragment and light source
    float lightDistance = length(Vertex.vary_position.xyz - u_position[0]);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;
    
    // Write this as modified depth
    gl_FragDepth = lightDistance;
}  

