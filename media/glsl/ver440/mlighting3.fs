#version 440 core

#define	MAX_LIGHTS 64

// per vertex interpolated program input
in VertexDataOut {
    vec3 att_position;
    vec4 vary_position; // vertex position
    vec2 vary_TexCoord;
    vec2 vary_LightCoord;
    vec3 vary_normal;
    flat int vary_color;
    vec4 vary_tangent;
} Vertex;

// Final fragment color output
out vec4 Fragment;

// constant program input
uniform vec3		u_position[MAX_LIGHTS]; // light position, world coordinates
uniform vec4		u_color[MAX_LIGHTS];
uniform int		u_num_lights;
uniform mat4		mvp;

uniform vec2		u_tcmod_scroll0;
uniform vec2		u_tcmod_scroll1;
uniform vec2		u_tcmod_scroll2;
uniform vec2		u_tcmod_scroll3;
uniform vec2		u_tcmod_scroll4;
uniform vec2		u_tcmod_scroll5;
uniform vec2		u_tcmod_scroll6;
uniform vec2		u_tcmod_scroll7;

uniform vec2		u_tcmod_scale0;
uniform vec2		u_tcmod_scale1;
uniform vec2		u_tcmod_scale2;
uniform vec2		u_tcmod_scale3;
uniform vec2		u_tcmod_scale4;
uniform vec2		u_tcmod_scale5;
uniform vec2		u_tcmod_scale6;
uniform vec2		u_tcmod_scale7;

uniform float	u_tcmod_sin0;
uniform float	u_tcmod_sin1;
uniform float	u_tcmod_sin2;
uniform float	u_tcmod_sin3;
uniform float	u_tcmod_sin4;
uniform float	u_tcmod_sin5;
uniform float	u_tcmod_sin6;
uniform float	u_tcmod_sin7;


uniform float	u_tcmod_cos0;
uniform float	u_tcmod_cos1;
uniform float	u_tcmod_cos2;
uniform float	u_tcmod_cos3;
uniform float	u_tcmod_cos4;
uniform float	u_tcmod_cos5;
uniform float	u_tcmod_cos6;
uniform float	u_tcmod_cos7;



layout(binding=0) uniform sampler2D texture0;// 8 possible textures
layout(binding=1) uniform sampler2D texture1;
layout(binding=2) uniform sampler2D texture2;
layout(binding=3) uniform sampler2D texture3;
layout(binding=4) uniform sampler2D texture4;
layout(binding=5) uniform sampler2D texture5;
layout(binding=6) uniform sampler2D texture6;
layout(binding=7) uniform sampler2D texture7;
layout(binding=8) uniform sampler2D texture_lightmap; //lightmap
layout(binding=9) uniform sampler2D texture_normalmap; //normalmap



// was originally varying, but couldnt pass through geometry shader
vec4 lightDir;

void main(void)
{
	vec3 normal  = normalize(vec3(mvp * vec4(Vertex.vary_normal, 1.0)));
	vec3 tangent = normalize(vec3(mvp * Vertex.vary_tangent));
	vec3 bitangent = normalize(cross(normal, tangent));
	mat3 tangent_space = mat3(normal, bitangent,  tangent);

	vec3 eye = tangent_space * -normalize(Vertex.vary_position.xyz); // eye vector in tangent space
	vec3 ambient = vec3(0.125f, 0.125f, 0.125f);
	vec3 light = vec3(0.0f, 0.0f, 0.0f);

	// scale and bias parallax effect
//	float height = texture(texture2, Vertex.vary_TexCoord).a * 0.16 + -0.08;
//	Fragment = texture(texture0, Vertex.vary_TexCoord + height * eye.xy);
//	vec3 normal_map = normalize(texture(texture_normalmap, Vertex.vary_TexCoord + height * eye.xy).xyz);

	vec3 normal_map;

	normal_map.x = 2 * texture(texture_normalmap, Vertex.vary_TexCoord).r - 1; 
	normal_map.z = 2 * texture(texture_normalmap, Vertex.vary_TexCoord).g - 1; 
	normal_map.y = 2 * texture(texture_normalmap, Vertex.vary_TexCoord).b - 1; 


	mat2 mRot0 = mat2( u_tcmod_cos0, -u_tcmod_sin0, u_tcmod_sin0,  u_tcmod_cos0);
	mat2 mRot1 = mat2( u_tcmod_cos1, -u_tcmod_sin1, u_tcmod_sin1,  u_tcmod_cos1);
	mat2 mRot2 = mat2( u_tcmod_cos2, -u_tcmod_sin2, u_tcmod_sin2,  u_tcmod_cos2);
	mat2 mRot3 = mat2( u_tcmod_cos3, -u_tcmod_sin3, u_tcmod_sin3,  u_tcmod_cos3);
//	mat2 mRot4 = mat2( u_tcmod_cos4, -u_tcmod_sin4, u_tcmod_sin4,  u_tcmod_cos4);
//	mat2 mRot5 = mat2( u_tcmod_cos5, -u_tcmod_sin5, u_tcmod_sin5,  u_tcmod_cos5);
//	mat2 mRot6 = mat2( u_tcmod_cos6, -u_tcmod_sin6, u_tcmod_sin6,  u_tcmod_cos6);
//	mat2 mRot7 = mat2( u_tcmod_cos7, -u_tcmod_sin7, u_tcmod_sin7,  u_tcmod_cos7);


	vec2 bias = vec2(0.5, -0.5);
	vec4 Fragment0 = texture(texture0, ((u_tcmod_scale0 * Vertex.vary_TexCoord - u_tcmod_scale0 * bias) * mRot0 + u_tcmod_scale0 * bias) + u_tcmod_scroll0);
	vec4 Fragment1 = texture(texture1, ((u_tcmod_scale1 * Vertex.vary_TexCoord - u_tcmod_scale1 * bias) * mRot1 + u_tcmod_scale1 * bias) + u_tcmod_scroll1);
	vec4 Fragment2 = texture(texture2, ((u_tcmod_scale2 * Vertex.vary_TexCoord - u_tcmod_scale2 * bias) * mRot2 + u_tcmod_scale2 * bias) + u_tcmod_scroll2);
	vec4 Fragment3 = texture(texture3, ((u_tcmod_scale3 * Vertex.vary_TexCoord - u_tcmod_scale3 * bias) * mRot3 + u_tcmod_scale3 * bias) + u_tcmod_scroll3);
//	vec4 Fragment4 = texture(texture4, ((u_tcmod_scale4 * Vertex.vary_TexCoord - u_tcmod_scale4 * bias) * mRot4 + u_tcmod_scale4 * bias) + u_tcmod_scroll4);
//	vec4 Fragment5 = texture(texture5, ((u_tcmod_scale5 * Vertex.vary_TexCoord - u_tcmod_scale5 * bias) * mRot5 + u_tcmod_scale5 * bias) + u_tcmod_scroll5);
//	vec4 Fragment6 = texture(texture6, ((u_tcmod_scale6 * Vertex.vary_TexCoord - u_tcmod_scale6 * bias) * mRot6 + u_tcmod_scale6 * bias) + u_tcmod_scroll6);
//	vec4 Fragment7 = texture(texture7, ((u_tcmod_scale7 * Vertex.vary_TexCoord - u_tcmod_scale7 * bias) * mRot7 + u_tcmod_scale7 * bias) + u_tcmod_scroll7);

	Fragment = Fragment0;
	Fragment.xyz += Fragment1.xyz;
	Fragment.xyz += Fragment2.xyz;
	Fragment.xyz += Fragment3.xyz;
//	Fragment.xyz += Fragment4.xyz;
//	Fragment.xyz += Fragment5.xyz;
//	Fragment.xyz += Fragment6.xyz;
//	Fragment.xyz += Fragment7.xyz;

//	Fragment = Fragment + texture(texture_lightmap, Vertex.vary_LightCoord);
//	Fragment.xyz = vec3(0.5,0.5,0.5);
//	Fragment.xyz = tangent;

	for(int i = 0; i < u_num_lights; i++)
	{
		vec3 lightPosWorld = u_position[i];

		vec4 lightDir = mvp * vec4(Vertex.att_position - lightPosWorld, 1.0);
		lightDir.a = length(Vertex.att_position.rgb - lightPosWorld.rgb); // distance from light


		vec3 v_light = normalize(vec3(lightDir.rgb));	
		vec3 n_light = tangent_space * v_light; // light vector in tangent space

		float diffuse = max(dot(v_light, normal), 0.25);		// directional light factor for fragment
		float atten = min( 80000.0 / pow(lightDir.a, 2.25), 0.25);	// light distance from fragment 1/(r^2) falloff
		vec3 v_reflect = reflect(v_light, normal);			// normal map reflection vector
		float specular = max(pow(dot(v_reflect, eye), 8.0), 0.25);	// specular relection for fragment
		light = light + ( vec3(u_color[i]) * u_color[i].a )  * atten * (diffuse * 0.75 + specular * 0.0); // combine everything
	}

	Fragment.rgb *= max(light, ambient);


}
