#version 330

uniform mat4 u_mvp;
uniform vec3 u_quad1, u_quad2;

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

in	vec3[]		iatt_position;
in	vec2[]		ivary_TexCoord;
in	vec2[]		ivary_LightCoord;
in	vec3[]		ivary_normal;
in	int[]		ivary_color;
in	vec4[]		ivary_tangent;


 
out    vec3 att_position;
out    vec2 vary_TexCoord;
out    vec2 vary_LightCoord;
out    vec3 vary_normal;
out    int vary_color;
out    vec4 vary_tangent; // color value


/*
// packed in normal
//in float life[];
//in float size[];
//in int type[];

flat out vec4 vColorPart; // passsed in tangent
*/

void main()
{
	if(ivary_normal[0].z != 0) // type
	{
		vec3 vPosOld = gl_in[0].gl_Position.xyz;
		float fSize = ivary_normal[0].y;
		
		vary_tangent = vec4(	ivary_tangent[0].r,
						ivary_tangent[0].g,
						ivary_tangent[0].b,
						ivary_normal[0].x); // life + color
		   
		vec3 vPos = vPosOld + (-u_quad1 - u_quad2) * fSize;
		vary_TexCoord = vec2(0.0, 0.0);
		gl_Position = u_mvp * vec4(vPos, 1.0);
		EmitVertex();
		
		vPos = vPosOld + (-u_quad1 + u_quad2) * fSize;
		vary_TexCoord = vec2(0.0, 1.0);
		gl_Position = u_mvp * vec4(vPos, 1.0);
		EmitVertex();
		
		vPos = vPosOld + (u_quad1 - u_quad2) * fSize;
		vary_TexCoord = vec2(1.0, 0.0);
		gl_Position = u_mvp * vec4(vPos, 1.0);
		EmitVertex();
		
		vPos = vPosOld + (u_quad1 + u_quad2) * fSize;
		vary_TexCoord = vec2(1.0, 1.0);
		gl_Position = u_mvp * vec4(vPos, 1.0);
		EmitVertex();
		  
		EndPrimitive();
	}
}