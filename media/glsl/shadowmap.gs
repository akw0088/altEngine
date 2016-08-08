#version 440 core

#define	MAX_LIGHTS 16

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

in VertexData {
	vec4		vary_position;
	vec2		vary_TexCoord;
	vec2		vary_LightCoord;
	vec3		vary_normal;
	flat int	vary_color;
	vec3		vary_tangent;
} VertexIn[3];

 
out VertexDataOut {
    vec4 vary_position;
    vec2 vary_TexCoord;
    vec2 vary_LightCoord;
    vec3 vary_normal;
    flat int vary_color;
    vec3 vary_tangent;
} VertexOut;

uniform mat4	mvp[6];
 

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) // for each triangle's vertices
        {
            VertexOut.vary_position = gl_in[i].gl_Position;
            gl_Position = mvp[face] * VertexOut.vary_position;
            EmitVertex();
        }    
        EndPrimitive();
    }
}  
