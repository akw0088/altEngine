/*
	gl_vertex  - object space vertex
	gl_position -  eye space vertex output
*/

varying vec3		normal, tangent, eye, light, binormal;
uniform sampler2D	texture0, texture1, texture2;
uniform mat4		mvp;

void main(void)
{
	vec3 binormal;
	normal = gl_NormalMatrix * gl_Normal;
	binormal = cross(normal, tangent);

	gl_Position = mvp * gl_Vertex;
	gl_FrontColor = gl_Color;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_MultiTexCoord1;
	gl_PointSize = 5.0;

	//convert light position to normalized vector pointing at vertex
	light = normalize(light - vec3(gl_Position));
	eye = normalize(eye - vec3(gl_Position));
	light = vec3(gl_ModelViewProjectionMatrix * vec4(light.x, light.y, light.z, 1.0));
	eye = vec3(gl_ModelViewProjectionMatrix * vec4(light.x, light.y, light.z, 1.0));
}