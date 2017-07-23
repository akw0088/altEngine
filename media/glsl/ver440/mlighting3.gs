#version 440 core

#define	MAX_LIGHTS 64

layout(triangles) in;
layout (triangle_strip, max_vertices=3) out;
 


in VertexData {
	vec3		att_position;
	vec4		vary_position;
	vec2		vary_TexCoord;
	vec2		vary_newTexCoord[4];
	vec2		vary_LightCoord;
	vec3		vary_normal;
	flat int	vary_color;
	vec4		vary_tangent;
	vec4 shadowpos[18];
} VertexIn[3];

 
out VertexDataOut {
    vec3 att_position;
    vec4 vary_position; // vertex position
    vec2 vary_TexCoord;
    vec2 vary_newTexCoord[4];
    vec2 vary_LightCoord;
    vec3 vary_normal;
    flat int vary_color;
    vec4 vary_tangent;
    vec4 shadowpos[18];
} VertexOut;
 
void main()
{
	//calculate normal
	vec3 a = gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz;
	vec3 b = gl_in[0].gl_Position.xyz - gl_in[2].gl_Position.xyz;
	vec3 normal = normalize(cross(b,a));

	//calculate tangent, these values are just constants
	float s1 = VertexIn[0].vary_TexCoord.x - VertexIn[1].vary_TexCoord.x; 
	float s2 = VertexIn[0].vary_TexCoord.x - VertexIn[2].vary_TexCoord.x; 

	float t1 = VertexIn[0].vary_TexCoord.y - VertexIn[1].vary_TexCoord.y; 
	float t2 = VertexIn[0].vary_TexCoord.y - VertexIn[2].vary_TexCoord.y; 

	vec3 tangent = (a*t2 - b*t1) / (s1*t2 - s2*t1);
	

	for(int i = 0; i < gl_in.length(); i++)
	{
		// copy attributes
		gl_Position = gl_in[i].gl_Position;
		VertexOut.att_position = VertexIn[i].att_position;
		VertexOut.vary_position = VertexIn[i].vary_position;
		VertexOut.vary_TexCoord = VertexIn[i].vary_TexCoord;
		VertexOut.vary_newTexCoord[0] = VertexIn[i].vary_newTexCoord[0];
		VertexOut.vary_newTexCoord[1] = VertexIn[i].vary_newTexCoord[1];
		VertexOut.vary_newTexCoord[2] = VertexIn[i].vary_newTexCoord[2];
		VertexOut.vary_newTexCoord[3] = VertexIn[i].vary_newTexCoord[3];
		VertexOut.vary_LightCoord = VertexIn[i].vary_LightCoord;
		VertexOut.vary_color = VertexIn[i].vary_color;
		VertexOut.vary_normal = VertexIn[i].vary_normal; // map normal
//		VertexOut.vary_tangent = VertexIn[i].vary_tangent; // map tangent
//		VertexOut.vary_normal = normal; // geometry shader normal (not smoothed across triangles)
		VertexOut.vary_tangent = vec4(tangent, 0.0f); // geometry shader tangent

		// done with the vertex
		EmitVertex();
	}
}