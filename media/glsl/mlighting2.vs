/*
	gl_vertex  - object space vertex
	gl_position -  eye space vertex output
*/


#define	MAX_LIGHTS 4

//vertex shader output
varying vec3		normal, eye;
varying vec4		light[MAX_LIGHTS];

//program input
uniform vec3		u_position[MAX_LIGHTS];
uniform vec4		u_color[MAX_LIGHTS];
uniform int		u_lights;
uniform mat4		mvp;

void main(void)
{
	normal = normalize(gl_NormalMatrix * gl_Normal);

	gl_Position = mvp * gl_Vertex;
	gl_FrontColor = gl_Color;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_MultiTexCoord1;
	gl_PointSize = 5.0;

	eye = normalize(vec3(gl_Position));
	for(int i = 0; i < u_lights; i++)
	{
		vec4 csPos = gl_ModelViewProjectionMatrix * vec4(u_position[i], 1.0);
		light[i].rgb = normalize(vec3(csPos - gl_Position));
		light[i].a = length(vec3(csPos - gl_Position));
	}
}