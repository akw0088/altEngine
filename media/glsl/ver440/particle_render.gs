#version 330

uniform mat4 u_mvp;
uniform vec3 u_quad1, u_quad2;

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

in VertexData {
	vec3		att_position;
	vec2		vary_TexCoord;
	vec2		vary_LightCoord;
	vec3		vary_normal;
	flat int	vary_color;
	vec4		vary_tangent;
} VertexIn[];

 
out VertexDataOut {
    vec3 att_position;
    vec2 vary_TexCoord;
    vec2 vary_LightCoord;
    vec3 vary_normal;
    flat int vary_color;
    vec4 vary_tangent; // color value
} VertexOut;


/*
// packed in normal
//in float life[];
//in float size[];
//in int type[];

flat out vec4 vColorPart; // passsed in tangent
*/

void main()
{
	if(VertexIn[0].vary_normal.z != 0) // type
	{
		vec3 vPosOld = gl_in[0].gl_Position.xyz;
		float fSize = VertexIn[0].vary_normal.y;
		
		VertexOut.vary_tangent = vec4(	VertexIn[0].vary_tangent.r,
						VertexIn[0].vary_tangent.g,
						VertexIn[0].vary_tangent.b,
						VertexIn[0].vary_normal.x); // life + color
		   
		vec3 vPos = vPosOld + (-u_quad1 - u_quad2) * fSize;
		VertexOut.vary_TexCoord = vec2(0.0, 0.0);
		gl_Position = u_mvp * vec4(vPos, 1.0);
		EmitVertex();
		
		vPos = vPosOld + (-u_quad1 + u_quad2) * fSize;
		VertexOut.vary_TexCoord = vec2(0.0, 1.0);
		gl_Position = u_mvp * vec4(vPos, 1.0);
		EmitVertex();
		
		vPos = vPosOld + (u_quad1 - u_quad2) * fSize;
		VertexOut.vary_TexCoord = vec2(1.0, 0.0);
		gl_Position = u_mvp * vec4(vPos, 1.0);
		EmitVertex();
		
		vPos = vPosOld + (u_quad1 + u_quad2) * fSize;
		VertexOut.vary_TexCoord = vec2(1.0, 1.0);
		gl_Position = u_mvp * vec4(vPos, 1.0);
		EmitVertex();
		  
		EndPrimitive();
	}
}