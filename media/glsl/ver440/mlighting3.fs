#version 440 core

#define	MAX_LIGHTS 64

// per vertex interpolated program input
in VertexData {
    vec3 att_position;
    vec4 vary_position; // vertex position
    vec2 vary_TexCoord;
    vec2 vary_newTexCoord[4];
    vec2 vary_LightCoord;
    vec3 vary_normal;
    flat int vary_color;
    vec4 vary_tangent;
    vec4 shadowpos[18];
} Vertex;

// Final fragment color output
out vec4 Fragment;

// constant program input
uniform vec4		u_position[MAX_LIGHTS]; // light position, world coordinates
uniform vec4		u_color[MAX_LIGHTS];
uniform int		u_num_lights;
uniform float		u_ambient;
uniform float		u_lightmap;
uniform float		u_shadowmap;
uniform mat4		mvp;

uniform int u_lightmap_stage;
uniform int u_depth;
uniform float u_brightness;
uniform float u_contrast;

uniform int u_env[8];
uniform int u_water[8];
uniform float u_rgbgen_scale[8];
uniform int u_alphatest[8];
uniform int u_portal;


uniform int u_time;


uniform sampler2D tex[8];// 8 possible textures

layout(binding=8) uniform sampler2D texture_lightmap; //lightmap
layout(binding=9) uniform sampler2D texture_normalmap; //normalmap


uniform sampler2D depth[18];


// Loop could clean this up, but need to figure out how to make an array of texture arrays that bind correctly
void calc_shadow(out float shadowFlagCombined, in int light_num)
{
	float depthmap;
	vec4 shadowpos;
	vec4 shadowWdivide;
	float shadowFlag = 1.0f;
	float bias = 0.00001f;
	float darkness = 0.50f;

	vec2 poissonDisk[4] = vec2[](
	 	vec2( -0.94201624, -0.39906216 ),
		vec2( 0.94558609, -0.76890725 ),
		vec2( -0.094184101, -0.92938870 ),
		vec2( 0.34495938, 0.29387760 )
	);



	for(int i = 0; i < 18; i++)
	{
		shadowpos = Vertex.shadowpos[i];
		shadowWdivide = shadowpos / shadowpos.w;
		if (shadowWdivide.w > 0)
		{
			depthmap = texture2D(depth[i], shadowWdivide.st).r;
			shadowFlag = depthmap < shadowWdivide.z - bias ? darkness : 1.0;
			for (int i = 0; i < 4; i++)
			{
				if ( texture( depth[i], shadowWdivide.st + poissonDisk[i] / 700.0 ).r  <  shadowWdivide.z - bias )
				{
					shadowFlag -= 0.2;
				}
			}

			shadowFlagCombined = max(shadowFlag, shadowFlagCombined);
		}
	}
}


// was originally varying, but couldnt pass through geometry shader
vec4 lightDir;

void main(void)
{
	Fragment = vec4(0.0, 0.0, 0.0, 0.0);
	vec2 tc, tc0, tc1, tc2, tc3;

	tc.x = 0.0;
	tc.y = 0.0;

	vec3 normal  = normalize(vec3(mvp * vec4(Vertex.vary_normal, 1.0)));
	vec3 tangent = normalize(vec3(mvp * Vertex.vary_tangent));
	vec3 bitangent = normalize(cross(normal, tangent));
	mat3 tangent_space = mat3(tangent, bitangent, normal);

	vec3 eye = -normalize(Vertex.vary_position.xyz); // for specular reflections
	vec3 ambient;
	ambient.x = u_ambient;
	ambient.y = u_ambient;
	ambient.z = u_ambient;
	vec3 light = vec3(0.0f, 0.0f, 0.0f);

	// scale and bias parallax effect
//	float height = texture(tex[2], Vertex.vary_TexCoord).a * 0.16 + -0.08;
//	Fragment = texture(tex[0], Vertex.vary_TexCoord + height * eye.xy);
//	vec3 normal_map = normalize(texture(texture_normalmap, Vertex.vary_TexCoord + height * eye.xy).xyz);

/*
	vec3 normal_map;

	normal_map.x = (2 * texture(texture_normalmap, Vertex.vary_newTexCoord[0]).r - 1);
	normal_map.y = (2 * texture(texture_normalmap, Vertex.vary_newTexCoord[0]).g - 1);
	normal_map.z = (2 * texture(texture_normalmap, Vertex.vary_newTexCoord[0]).b - 1);

	normal = 2 * (transpose(mvp) * vec4(normal_map, 1.0)).xyz;
*/

	vec4 Fragment_stage[4];


	vec3 u = normalize(-Vertex.vary_position.xyz);
	vec3 r = reflect(u, -normal);

	tc0 = Vertex.vary_newTexCoord[0];
	tc1 = Vertex.vary_newTexCoord[1];
	tc2 = Vertex.vary_newTexCoord[2];
	tc3 = Vertex.vary_newTexCoord[3];

	if (u_portal > 0)
	{
		tc0 *= 0.5;
		tc0.s += 0.9;
		tc0.t += 1.1;


		tc1 *= 0.5;
		tc1.s += 0.9;
		tc1.t += 1.1;

		tc2 *= 0.5;
		tc2.s += 0.9;
		tc2.t += 1.1;

		tc3 *= 0.5;
		tc3.s += 0.9;
		tc3.t += 1.1;
	}

	if (u_env[0] + u_env[1] + u_env[2] + u_env[3] > 0)
	{
		// Environment "sphere" mapping
		r.z += 1.0;
		float m = 0.5 * inversesqrt(dot(r,r));
		tc = (r.xy * m) + vec2(0.5);
		tc.y *= -1;
	
		//equirectangular env mapping
	//	tc.y = r.y;
	//	r.y = 0.0;
	//	tc.x = normalize(r).x * 0.5;
		
	//	float s = sign(r.z) * 0.5;
	//	tc.s = 0.75 - s * (0.5 - tc.s);
	//	tc.t = -(0.5 + 0.5 * tc.t);
	
		tc *= 0.2; // enlarge texture so you cant see details

		tc0 = tc;
		tc1 = tc;
		tc2 = tc;
		tc3 = tc;
	}

	if (u_water[0] + u_water[1] + u_water[2] + u_water[3] > 0)
	{
		float s = Vertex.vary_newTexCoord[0].x;
		float t = Vertex.vary_newTexCoord[0].y;
		float x = Vertex.vary_position.x;
		float y = Vertex.vary_position.y;
		float z = Vertex.vary_position.z;

		tc.x = s + sin( (( x + z ) * 1.0/128.0 * 0.125 + u_time / 500.0 ) );
		tc.y = t + sin((y * 1.0 / 128 * 0.125 + u_time / 500.0));

		tc0 = tc;
		tc1 = tc;
		tc2 = tc;
		tc3 = tc;
	}


	if (u_lightmap_stage > 0)
	{
		Fragment_stage[0] = texture(tex[0], Vertex.vary_LightCoord);
	}
	else
	{
		Fragment_stage[0] = texture(tex[0], tc0);
	}
	

	if (u_lightmap_stage > 0)
	{
		Fragment_stage[1] = texture(tex[1], Vertex.vary_LightCoord);
	}
	else
	{
		Fragment_stage[1] = texture(tex[1], tc1);
	}


	if (u_lightmap_stage > 0)
	{
		Fragment_stage[2] = texture(tex[2], Vertex.vary_LightCoord);
	}
	else
	{
		Fragment_stage[2] = texture(tex[2], tc2);
	}


	if (u_lightmap_stage > 0)
	{
		Fragment_stage[3] = texture(tex[3], Vertex.vary_LightCoord);
	}
	else
	{
		Fragment_stage[3] = texture(tex[3], tc3);
	}

	for(int i = 0; i < 4; i++)
	{
		// Keep GT0
		if (u_alphatest[i] == 1)
		{
			if (Fragment_stage[i].a <= 0.0)
				discard;
		}

		// Keep LT128
		if (u_alphatest[i] == 2)
		{
			if (Fragment_stage[i].a >= 0.5)
				discard;
		}

		// Keep GE128
		if (u_alphatest[i] == 3)
		{
			if (Fragment_stage[i].a < 0.5)
				discard;
		}
	}

	ambient *= min(u_rgbgen_scale[0], 3.0);
	ambient *= min(u_rgbgen_scale[1], 3.0);
	ambient *= min(u_rgbgen_scale[2], 3.0);
	ambient *= min(u_rgbgen_scale[3], 3.0);


//	alpha 1 = opaque, 0 equals transparent
	Fragment.a = Fragment_stage[0].a * Fragment_stage[1].a  * Fragment_stage[2].a  * Fragment_stage[3].a;
//	Fragment.a = 1.0;
	Fragment.xyz += Fragment_stage[0].xyz * 0.75;
	Fragment.xyz += Fragment_stage[1].xyz;
	Fragment.xyz += Fragment_stage[2].xyz;
	Fragment.xyz += Fragment_stage[3].xyz;
//	Fragment.xyz += Fragment_stage[4].xyz;
//	Fragment.xyz += Fragment_stage[5].xyz;
//	Fragment.xyz += Fragment_stage[6].xyz;
//	Fragment.xyz += Fragment_stage[7].xyz;

//	Fragment.xyz = vec3(0.5,0.5,0.5);
//	Fragment.xyz = tangent;
	float shadow = 1.0;


	for(int i = 0; i < u_num_lights; i++)
	{
		vec3 lightPosWorld = vec3(u_position[i]);

		vec4 lightDir = mvp * vec4(Vertex.att_position - lightPosWorld, 1.0);
		lightDir.a = length(Vertex.att_position.rgb - lightPosWorld.rgb); // distance from light


		vec3 v_light = normalize(vec3(lightDir.rgb));	
		vec3 n_light = tangent_space * v_light; // light vector in tangent space

//		vec4 lightpos = mvp * vec4(lightPosWorld, 1.0);
//		vec3 v_light2 = normalize(vec3(lightpos.xyz));
//		vec3 n_light = tangent_space * v_light2; // light vector in tangent space


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
			Fragment.xyz *= lightmap * 2.0;
	}
	Fragment.rgb *= max(light, ambient);


	if (u_shadowmap > 0.0)
	{
		calc_shadow(shadow, 0);
		Fragment.rgb *= shadow;
	}

//	Fragment.rgb += u_brightness;
//	Fragment.rgb *= u_contrast;


//	Fragment.rgb = light;

}
