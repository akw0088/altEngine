#version 440 core

#define	MAX_LIGHTS 32
#define MAX_SHADOW 4

// per vertex interpolated program input
in VertexDataOut {
    vec4 vary_position; // vertex position
    vec2 vary_TexCoord;
    vec2 vary_LightCoord;
    vec3 vary_normal;
    flat int vary_color;
    vec3 vary_tangent;
    vec4 shadowpos[24]; // 4 shadow casting lights 6 faces
} Vertex;

// Final fragment color output
out vec4 Fragment;

// constant program input
uniform vec3		u_position[MAX_LIGHTS]; // light position, world coordinates
uniform vec4		u_color[MAX_LIGHTS];
uniform int		u_num_lights;
uniform mat4		mvp;




layout(binding=0) uniform sampler2D texture0;
layout(binding=1) uniform sampler2D texture1;
layout(binding=2) uniform sampler2D texture2;

layout(binding=3) uniform sampler2DArray shadowtex0;
layout(binding=4) uniform sampler2DArray shadowtex1;
layout(binding=5) uniform sampler2DArray shadowtex2;
layout(binding=6) uniform sampler2DArray shadowtex3;
layout(binding=7) uniform sampler2DArray shadowtex4;
layout(binding=8) uniform sampler2DArray shadowtex5;








// was originally varying, but couldnt pass through geometry shader
vec4 vary_light;



// Loop could clean this up, but need to figure out how to make an array of texture arrays that bind correctly
void calc_shadow(out float shadowFlagCombined, in int light_num)
{
	float shadowDistFromLight;
	vec4 shadowWdivide;
	float shadowFlag = 0.0;


	//Shadowmap0
	shadowWdivide = Vertex.shadowpos[0 + light_num * 6] / Vertex.shadowpos[0 + light_num * 6].w;
	shadowDistFromLight = texture(shadowtex0, vec3(shadowWdivide.s, shadowWdivide.t, light_num) ).r;
	shadowWdivide.z += 0.0005;

	if (Vertex.shadowpos[0 + light_num * 6].w > 0.0)
		shadowFlag = shadowDistFromLight < shadowWdivide.z ? 0.0 : 1.0;
	shadowFlagCombined = max(shadowFlagCombined, shadowFlag);

	//Shadowmap1
	shadowWdivide = Vertex.shadowpos[1 + light_num * 6] / Vertex.shadowpos[1 + light_num* 6].w;
	shadowDistFromLight = texture(shadowtex1, vec3(shadowWdivide.s, shadowWdivide.t, light_num)).r;
	shadowWdivide.z += 0.0005;

	if (Vertex.shadowpos[1 + light_num * 6].w > 0.0)
		shadowFlag = shadowDistFromLight < shadowWdivide.z ? 0.0 : 1.0;
	shadowFlagCombined = max(shadowFlagCombined, shadowFlag);

	//Shadowmap2
	shadowWdivide = Vertex.shadowpos[2 + light_num * 6] / Vertex.shadowpos[2 + light_num * 6].w;
	shadowDistFromLight = texture(shadowtex2, vec3(shadowWdivide.s, shadowWdivide.t, light_num)).r;
	shadowWdivide.z += 0.0005;


	if (Vertex.shadowpos[2 + light_num * 6].w > 0.0)
		shadowFlag = shadowDistFromLight < shadowWdivide.z ? 0.0 : 1.0;
	shadowFlagCombined = max(shadowFlagCombined, shadowFlag);


	//Shadowmap3
	shadowWdivide = Vertex.shadowpos[3 + light_num * 6] / Vertex.shadowpos[3 + light_num * 6].w;
	shadowDistFromLight = texture(shadowtex3, vec3(shadowWdivide.s, shadowWdivide.t, light_num)).r;
	shadowWdivide.z += 0.0005;


	if (Vertex.shadowpos[3 + light_num * 6].w > 0.0)
		shadowFlag = shadowDistFromLight < shadowWdivide.z ? 0.0 : 1.0;
	shadowFlagCombined = max(shadowFlagCombined, shadowFlag);

	//Shadowmap4
	shadowWdivide = Vertex.shadowpos[4 + light_num * 6] / Vertex.shadowpos[4 + light_num * 6].w;
	shadowDistFromLight = texture(shadowtex4, vec3(shadowWdivide.s, shadowWdivide.t, light_num)).r;
	shadowWdivide.z += 0.0005;


	if (Vertex.shadowpos[4 + light_num * 6].w > 0.0)
		shadowFlag = shadowDistFromLight < shadowWdivide.z ? 0.0 : 1.0;
	shadowFlagCombined = max(shadowFlagCombined, shadowFlag);

	//Shadowmap5
	shadowWdivide = Vertex.shadowpos[5 + light_num * 6] / Vertex.shadowpos[5 + light_num * 6].w;
	shadowDistFromLight = texture(shadowtex5, vec3(shadowWdivide.s, shadowWdivide.t, light_num)).r;
	shadowWdivide.z += 0.0005;


	if (Vertex.shadowpos[5 + light_num * 6].w > 0.0)
		shadowFlag = shadowDistFromLight < shadowWdivide.z ? 0.0 : 1.0;
	shadowFlagCombined = max(shadowFlagCombined, shadowFlag);
}




void calc_light(in vec3 eye, in vec3 normal_map, out vec3 light, in int light_num)
{
	vec4 lightPosEye = normalize(mvp * vec4(u_position[light_num], 1.0));
	vary_light.rgb = vec3(Vertex.vary_position - lightPosEye); // light vector to fragment
	vary_light.a = length(vary_light.rgb); // distance from light
	
	vec3 v_light = normalize(vec3(vary_light));			// light vector in tangent space
	vec3 v_reflect = reflect(v_light, normal_map);			// normal map reflection vector
	float diffuse = max(dot(v_light, normal_map), 0.25);		// directional light factor for fragment
	float specular = max(pow(dot(v_reflect, eye), 8.0), 0.25);	// specular relection for fragment
	float atten = min( 80000.0 / pow(vary_light.a, 1.75), 0.25);	// light distance from fragment 1/(r^2) falloff

	light = ( vec3(u_color[light_num]) * u_color[light_num].a )  * (diffuse * 0.75 + specular * 0.1); // combine everything
}


void main(void)
{
	vec3 normal = normalize(Vertex.vary_normal);
	vec3 tangent = normalize(Vertex.vary_tangent);
	vec3 bitangent = normalize(cross(normal, tangent));
	mat3 tangent_space = mat3(tangent, bitangent, normal);

	vec3 eye = tangent_space * -normalize(Vertex.vary_position.xyz); // eye vector in tangent space
	vec3 ambient = vec3(0.125f, 0.125f, 0.125f);
	vec3 light = vec3(0.0f, 0.0f, 0.0f);

	vec3 normal_map = texture(texture2, Vertex.vary_TexCoord).xyz;

	Fragment = texture(texture0, Vertex.vary_TexCoord);

	




	for(int i = 0; i < u_num_lights; i++)
	{
		float shadowFlagCombined = 0.5f;

		if (i < MAX_SHADOW)
		{		
			calc_shadow(shadowFlagCombined, i);

			vec3 temp;
			calc_light(eye, normal_map, temp, i);

			light = light + shadowFlagCombined * temp;
		}
		else
		{
			vec3 temp;

			calc_light(eye, normal_map, temp, i);
			light = light + temp;
		}

	}

	light = light / u_num_lights;

	Fragment.rgb *= max(light, ambient);

}
