#version 440 core

#define	MAX_LIGHTS 64

layout(triangles) in;
layout (triangle_strip, max_vertices=3) out;
 


in VertexData {
	vec3		att_position;
	vec4		vary_position;
	vec2		vary_TexCoord;
	vec2		vary_TexCoord0;
	vec2		vary_TexCoord1;
	vec2		vary_TexCoord2;
	vec2		vary_TexCoord3;
	vec2		vary_LightCoord;
	vec3		vary_normal;
	flat int	vary_color;
	vec4		vary_tangent;
} VertexIn[3];

 
out VertexDataOut {
    vec3 att_position;
    vec4 vary_position;
    vec2 vary_TexCoord;
    vec2 vary_TexCoord0;
    vec2 vary_TexCoord1;
    vec2 vary_TexCoord2;
    vec2 vary_TexCoord3;
    vec2 vary_LightCoord;
    vec3 vary_normal;
    flat int vary_color;
    vec4 vary_tangent;
} VertexOut;
 
void main()
{
	//calculate normal
	vec3 a = gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz;
	vec3 b = gl_in[0].gl_Position.xyz - gl_in[2].gl_Position.xyz;
	vec3 normal = cross(a,b);

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
		VertexOut.vary_TexCoord0 = VertexIn[i].vary_TexCoord0;
		VertexOut.vary_TexCoord1 = VertexIn[i].vary_TexCoord1;
		VertexOut.vary_TexCoord2 = VertexIn[i].vary_TexCoord2;
		VertexOut.vary_TexCoord3 = VertexIn[i].vary_TexCoord3;
		VertexOut.vary_LightCoord = VertexIn[i].vary_LightCoord;
		VertexOut.vary_color = VertexIn[i].vary_color;
		VertexOut.vary_normal = VertexIn[i].vary_normal; // I dont trust the artists ;)
		VertexOut.vary_tangent = VertexIn[i].vary_tangent; // I dont trust the artists ;)
//		VertexOut.vary_normal = normal;
//		VertexOut.vary_tangent = vec4(tangent, 0.0f);

		// done with the vertex
		EmitVertex();
	}
}