#version 440 core

#define	MAX_LIGHTS 32
#define MAX_SHADOW 4

// per vertex interpolated program input
in VertexDataOut {
    vec3 att_position;
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



// Loop could clean this up, but need to figure out how to make an array of texture arrays that bind correctly
void calc_shadow(out float shadowFlagCombined, in int light_num)
{
	float shadowDistFromLight;
	vec4 shadowWdivide;
	float shadowFlag = 0.0;


	//Shadowmap0
	Vertex.shadowpos[0 + light_num * 6] * 0.5 + 0.5;
	shadowWdivide = Vertex.shadowpos[0 + light_num * 6] / Vertex.shadowpos[0 + light_num * 6].w;
	shadowDistFromLight = texture(shadowtex0, vec3(shadowWdivide.s, shadowWdivide.t, light_num) ).r;
	shadowWdivide.z += 0.0005;

	if (Vertex.shadowpos[0 + light_num * 6].w > 0.0)
		shadowFlag = shadowDistFromLight < shadowWdivide.z ? 0.0 : 1.0;
	shadowFlagCombined = max(shadowFlagCombined, shadowFlag);

	//Shadowmap1
	Vertex.shadowpos[1 + light_num * 6] * 0.5 + 0.5;
	shadowWdivide = Vertex.shadowpos[1 + light_num * 6] / Vertex.shadowpos[1 + light_num* 6].w;
	shadowDistFromLight = texture(shadowtex1, vec3(shadowWdivide.s, shadowWdivide.t, light_num)).r;
	shadowWdivide.z += 0.0005;

	if (Vertex.shadowpos[1 + light_num * 6].w > 0.0)
		shadowFlag = shadowDistFromLight < shadowWdivide.z ? 0.0 : 1.0;
	shadowFlagCombined = max(shadowFlagCombined, shadowFlag);

	//Shadowmap2
	Vertex.shadowpos[2 + light_num * 6] * 0.5 + 0.5;
	shadowWdivide = Vertex.shadowpos[2 + light_num * 6] / Vertex.shadowpos[2 + light_num * 6].w;
	shadowDistFromLight = texture(shadowtex2, vec3(shadowWdivide.s, shadowWdivide.t, light_num)).r;
	shadowWdivide.z += 0.0005;


	if (Vertex.shadowpos[2 + light_num * 6].w > 0.0)
		shadowFlag = shadowDistFromLight < shadowWdivide.z ? 0.0 : 1.0;
	shadowFlagCombined = max(shadowFlagCombined, shadowFlag);


	//Shadowmap3
	Vertex.shadowpos[3 + light_num * 6] * 0.5 + 0.5;
	shadowWdivide = Vertex.shadowpos[3 + light_num * 6] / Vertex.shadowpos[3 + light_num * 6].w;
	shadowDistFromLight = texture(shadowtex3, vec3(shadowWdivide.s, shadowWdivide.t, light_num)).r;
	shadowWdivide.z += 0.0005;


	if (Vertex.shadowpos[3 + light_num * 6].w > 0.0)
		shadowFlag = shadowDistFromLight < shadowWdivide.z ? 0.0 : 1.0;
	shadowFlagCombined = max(shadowFlagCombined, shadowFlag);

	//Shadowmap4
	Vertex.shadowpos[4 + light_num * 6] * 0.5 + 0.5;
	shadowWdivide = Vertex.shadowpos[4 + light_num * 6] / Vertex.shadowpos[4 + light_num * 6].w;
	shadowDistFromLight = texture(shadowtex4, vec3(shadowWdivide.s, shadowWdivide.t, light_num)).r;
	shadowWdivide.z += 0.0005;


	if (Vertex.shadowpos[4 + light_num * 6].w > 0.0)
		shadowFlag = shadowDistFromLight < shadowWdivide.z ? 0.0 : 1.0;
	shadowFlagCombined = max(shadowFlagCombined, shadowFlag);

	//Shadowmap5
	Vertex.shadowpos[5 + light_num * 6] * 0.5 + 0.5;
	shadowWdivide = Vertex.shadowpos[5 + light_num * 6] / Vertex.shadowpos[5 + light_num * 6].w;
	shadowDistFromLight = texture(shadowtex5, vec3(shadowWdivide.s, shadowWdivide.t, light_num)).r;
	shadowWdivide.z += 0.0005;


	if (Vertex.shadowpos[5 + light_num * 6].w > 0.0)
		shadowFlag = shadowDistFromLight < shadowWdivide.z ? 0.0 : 1.0;
	shadowFlagCombined = max(shadowFlagCombined, shadowFlag);
}




void calc_light(in vec3 eye, in vec3 normal, out vec3 lighti, in int light_num)
{
	vec4 lightPosEye = mvp * vec4(u_position[light_num], 1.0);

	vec4 lightDir = mvp * vec4(Vertex.att_position - lightPosWorld, 1.0);


	lightDir.rgb = vec3(Vertex.att_position - lightPosEye); // light vector to fragment
	lightDir.a = length(vary_light.rgb); // distance from light
	
	vec3 v_light = normalize(vec3(lightDir));			// light vector
	vec3 v_reflect = reflect(v_light, normal);			// normal map reflection vector


	float diffuse = max(dot(v_light, normal), 0.25);		// directional light factor for fragment
	float specular = max(pow(dot(v_reflect, eye), 8.0), 0.25);	// specular reflection for fragment
	float atten = min( 80000.0 / pow(lightDir.a, 1.75), 0.25);	// light distance from fragment 1/(r^2) falloff
	vec3 color = vec3(u_color[light_num]) ;			// light color from bsp
	float intensity = u_color[light_num].a;			// light intensity from bsp

	lighti = color * intensity  * (diffuse * 0.5 + specular * 0.0); // combine everything
}


void main(void)
{
	vec3 normal = normalize(Vertex.vary_normal);
	vec3 tangent = normalize(Vertex.vary_tangent);
	vec3 bitangent = normalize(cross(normal, tangent));
	mat3 tangent_space = mat3(	tangent.x, bitangent.x, normal.x,
					tangent.y, bitangent.y, normal.y,
					tangent.z, bitangent.z, normal.z	);

	vec3 eye = tangent_space * -normalize(Vertex.vary_position.xyz); // eye vector in tangent space
	vec3 ambient = vec3(0.125f, 0.125f, 0.125f);
	vec3 light = vec3(0.0f, 0.0f, 0.0f);


	vec3 normal_map;

	normal_map.x = 2 * texture(texture2, Vertex.vary_TexCoord).r - 1; 
	normal_map.z = 2 * texture(texture2, Vertex.vary_TexCoord).g - 1; 
	normal_map.y = 2 * texture(texture2, Vertex.vary_TexCoord).b - 1; 

	Fragment = texture(texture0, Vertex.vary_TexCoord);


	for(int i = 0; i < u_num_lights; i++)
	{
		float shadowFlagCombined = 0.5f;

		if (i < MAX_SHADOW)
		{		
			calc_shadow(shadowFlagCombined, i);

			vec3 lighti;
			calc_light(eye, normal_map, lighti, i);

			light = light + (shadowFlagCombined * 1.0 / u_num_lights);
		}
		else
		{
			vec3 lighti;

			calc_light(eye, normal_map, lighti, i);
			light = light + (1.0 / u_num_lights);
		}

	}

	Fragment.rgb *= max(light, ambient);

}
