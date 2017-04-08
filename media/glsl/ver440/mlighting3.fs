#version 440 core

#define	MAX_LIGHTS 64

// per vertex interpolated program input
in VertexDataOut {
    vec3 att_position;
    vec4 vary_position; // vertex position
    vec2 vary_TexCoord;
    vec2 vary_TexCoord0;
    vec2 vary_TexCoord1;
    vec2 vary_TexCoord2;
    vec2 vary_TexCoord3;
    vec2 vary_LightCoord;
    vec3 vary_normal;
    flat int vary_color;
    vec4 vary_tangent;
} Vertex;

// Final fragment color output
out vec4 Fragment;

// constant program input
uniform vec4		u_position[MAX_LIGHTS]; // light position, world coordinates
uniform vec4		u_color[MAX_LIGHTS];
uniform int		u_num_lights;
uniform float		u_ambient;
uniform float		u_lightmap;
uniform mat4		mvp;

uniform int u_lightmap_stage;


uniform int u_env0;
uniform int u_env1;
uniform int u_env2;
uniform int u_env3;
uniform int u_env4;
uniform int u_env5;
uniform int u_env6;
uniform int u_env7;

uniform int u_water0;
uniform int u_water1;
uniform int u_water2;
uniform int u_water3;
uniform int u_water4;
uniform int u_water5;
uniform int u_water6;
uniform int u_water7;

uniform float u_rgbgen_scale0;
uniform float u_rgbgen_scale1;
uniform float u_rgbgen_scale2;
uniform float u_rgbgen_scale3;
uniform float u_rgbgen_scale4;
uniform float u_rgbgen_scale5;
uniform float u_rgbgen_scale6;
uniform float u_rgbgen_scale7;


uniform int u_time;


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
	mat3 tangent_space = mat3(tangent, bitangent, normal);

	vec3 eye = tangent_space * -normalize(Vertex.vary_position.xyz); // eye vector in tangent space
	vec3 ambient;
	ambient.x = u_ambient;
	ambient.y = u_ambient;
	ambient.z = u_ambient;
	vec3 light = vec3(0.0f, 0.0f, 0.0f);

	// scale and bias parallax effect
//	float height = texture(texture2, Vertex.vary_TexCoord).a * 0.16 + -0.08;
//	Fragment = texture(texture0, Vertex.vary_TexCoord + height * eye.xy);
//	vec3 normal_map = normalize(texture(texture_normalmap, Vertex.vary_TexCoord + height * eye.xy).xyz);

	vec3 normal_map;

	normal_map.x = 2 * texture(texture_normalmap, Vertex.vary_TexCoord0).r - 1; 
	normal_map.z = 2 * texture(texture_normalmap, Vertex.vary_TexCoord0).g - 1; 
	normal_map.y = 2 * texture(texture_normalmap, Vertex.vary_TexCoord0).b - 1; 


	vec4 Fragment0;
	vec4 Fragment1;
	vec4 Fragment2;
	vec4 Fragment3;


	vec3 u = normalize(-Vertex.vary_position.zyx);
	vec3 r = reflect(u, -normal);


	vec2 tc;

	if (u_env0 + u_env1 + u_env2 + u_env3 > 0)
	{
		// Environment "sphere" mapping
		r.z += 1.0;
		float m = 0.5 * inversesqrt(dot(r,r));
		tc = (r.xy * m) + vec2(0.5);
	
		//equirectangular env mapping
	//	tc.y = r.y;
	//	r.y = 0.0;
	//	tc.x = normalize(r).x * 0.5;
		
	//	float s = sign(r.z) * 0.5;
	//	tc.s = 0.75 - s * (0.5 - tc.s);
	//	tc.t = 0.5 + 0.5 * tc.t;
	
		tc *= 0.2; // enlarge texture so you cant see details
	}

	if (u_water0 + u_water1 + u_water2 + u_water3 > 0)
	{
		float s = Vertex.vary_TexCoord0.x;
		float t = Vertex.vary_TexCoord0.y;
		float x = Vertex.vary_position.x;
		float y = Vertex.vary_position.y;
		float z = Vertex.vary_position.z;

		tc.x = s + sin( (( x + z ) * 1.0/128.0 * 0.125 + u_time / 500.0 ) );
		tc.y = t + sin((y * 1.0 / 128 * 0.125 + u_time / 500.0));
	}


	if (u_lightmap_stage > 0)
	{
		Fragment0 = 0.125 * texture(texture0, Vertex.vary_LightCoord);
	}
	else
	{
		if (u_env0 > 0 || u_water0 > 0)
			Fragment0 = texture(texture0, tc);
		else
			Fragment0 = texture(texture0, Vertex.vary_TexCoord0);
	}
	
	if (u_env1 > 0 || u_water1 > 0)
		Fragment1 = texture(texture1, tc);
	else
		Fragment1 = texture(texture1, Vertex.vary_TexCoord1);


	if (u_env2 > 0 || u_water2 > 0)
		Fragment2 = texture(texture2, tc);
	else
		Fragment2 = texture(texture2, Vertex.vary_TexCoord2);


	if (u_env3 > 0 || u_water3 > 0)
		Fragment3 = texture(texture3, tc);
	else
		Fragment3 = texture(texture3, Vertex.vary_TexCoord3);



	ambient *= min(u_rgbgen_scale0, 3.0);
	ambient *= min(u_rgbgen_scale1, 3.0);
	ambient *= min(u_rgbgen_scale2, 3.0);
	ambient *= min(u_rgbgen_scale3, 3.0);


	float frag0gs = (Fragment0.r + Fragment0.g + Fragment0.b) / 3.0;
	float frag1gs = (Fragment1.r + Fragment1.g + Fragment1.b) / 3.0;
	float frag2gs = (Fragment2.r + Fragment2.g + Fragment2.b) / 3.0;
	float frag3gs = (Fragment3.r + Fragment3.g + Fragment3.b) / 3.0;

//	alpha 1 = opaque, 0 equals transparent
	Fragment.a = Fragment0.a * Fragment1.a  * Fragment2.a  * Fragment3.a;
//	Fragment.a = 1.0;
	Fragment.xyz += Fragment0.xyz * 0.75;
	Fragment.xyz += Fragment1.xyz;
	Fragment.xyz += Fragment2.xyz;
	Fragment.xyz += Fragment3.xyz;
//	Fragment.xyz += Fragment4.xyz;
//	Fragment.xyz += Fragment5.xyz;
//	Fragment.xyz += Fragment6.xyz;
//	Fragment.xyz += Fragment7.xyz;

//	Fragment.xyz = vec3(0.5,0.5,0.5);
//	Fragment.xyz = tangent;

	for(int i = 0; i < u_num_lights; i++)
	{
		vec3 lightPosWorld = vec3(u_position[i]);

		vec4 lightDir = mvp * vec4(Vertex.att_position - lightPosWorld, 1.0);
		lightDir.a = length(Vertex.att_position.rgb - lightPosWorld.rgb); // distance from light


		vec3 v_light = normalize(vec3(lightDir.rgb));	
		vec3 n_light = tangent_space * v_light; // light vector in tangent space

		float diffuse = max(dot(v_light, normal), 0.25);		// directional light factor for fragment
		float atten = min( u_position[i].a * 160000.0 / pow(lightDir.a, 2.25), 0.25);	// light distance from fragment 1/(r^2) falloff
		vec3 v_reflect = reflect(v_light, normal);			// normal map reflection vector
		float specular = max(pow(dot(v_reflect, eye), 8.0), 0.25);	// specular relection for fragment
		light = light + ( vec3(u_color[i]) * u_color[i].a )  * atten * (diffuse * 0.75 + specular * 0.0); // combine everything
	}

	light *= vec3(1.0 - u_lightmap, 1.0 - u_lightmap, 1.0 - u_lightmap);
	vec3 lightmap = texture(texture_lightmap, Vertex.vary_LightCoord).xyz;


	if (u_lightmap > 0.0)
	{
		if (lightmap.r + lightmap.g + lightmap.b > 0.001)
			Fragment.xyz *= lightmap * 1.5;
	}
	Fragment.rgb *= max(light, ambient);
}
