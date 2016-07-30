/*
	gl_vertex  - object space vertex
	gl_position -  eye space vertex output
*/

#define	MAX_LIGHTS 4

//vertex shader input
varying vec3		normal, eye;
varying vec4		light[MAX_LIGHTS];

//program input
uniform vec3		u_position[MAX_LIGHTS];
uniform vec4		u_color[MAX_LIGHTS];
uniform int		u_lights;
uniform mat4		mvp;

uniform sampler2D	texture0, texture1, texture2;

void main(void)
{
	vec4 texColor[3];
	vec3 normalMap;
	vec3 v_normal;
	float heightMap;

	texColor[0] = texture2D(texture0, gl_TexCoord[0].st);
	texColor[1] = texture2D(texture1, gl_TexCoord[1].st);
	texColor[2] = texture2D(texture2, gl_TexCoord[0].st);

	normalMap = normalize(texColor[2].rgb);
	heightMap = texColor[2].a;

	gl_FragColor = (texColor[0] * 0.7) + (texColor[1] * 0.20) + (gl_Color * 0.01);
	v_normal = normalize(normal);


	//ambient
	vec3 intensity = vec3(0.5, 0.5, 0.5);
	for(int i = 0; i < 8; i++)
	{
		vec3 v_light = normalize(vec3(light[i]));
		vec3 v_reflect = reflect(v_light, v_normal);
		float diffuse = max(dot(v_light, v_normal), 0.0) * 0.25;
		float specular = pow(max(dot(v_reflect, -eye), 0.0), 10.0) * 0.25;
		float atten = min(40000.0 / pow(light[i].a, 1.75), 0.75);

		intensity = intensity + atten * (diffuse + specular + vec3(u_color[i]) * 0.25);
	}
	gl_FragColor.rgb *= intensity;
	gl_FragColor.rgb *= gl_FragColor.a;
}
