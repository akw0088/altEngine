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
layout (location = 0) out vec4 Fragment;
layout (location = 1) out vec4 normal_depth;


// constant program input
uniform vec4		u_position[MAX_LIGHTS]; // light position, world coordinates
uniform vec4		u_color[MAX_LIGHTS];
uniform int		u_num_lights;
uniform float		u_ambient;
uniform float		u_lightmap;
uniform float		u_shadowmap;
uniform int		u_num_shadowmap;
uniform mat4		mvp;


uniform int u_lightmap_stage;
uniform int u_depth;
uniform float u_dissolve;
uniform float u_brightness;
uniform float u_contrast;
uniform float u_exposure;

uniform int u_env[4];
uniform float u_rgbgen_scale[4];
uniform int u_alphatest[4];
uniform int u_portal;
uniform float u_normalmap;
uniform int u_tone;

uniform float u_fog;
uniform float u_fog_start;
uniform float u_fog_end;
uniform vec3 u_fog_color;
uniform vec3 u_normalmap_scale;
uniform float u_specular_exponent;
uniform float u_specular_factor;
uniform float u_diffuse_factor;
uniform float u_atten_exponent;
uniform float u_atten_scale;
uniform float u_atten_min;
uniform float u_diffuse_min;
uniform float u_specular_min;



uniform sampler2D tex[4];// 4 possible textures

layout(binding=7) uniform sampler2D texture_dissolve;
layout(binding=8) uniform sampler2D texture_lightmap; //lightmap
layout(binding=9) uniform sampler2D texture_normalmap; //normalmap


uniform sampler2D depth[18];
uniform float u_alpha;

vec3 ACESFilm( vec3 x )
{
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return clamp( (x * ( a * x + b ) ) / ( x * ( c * x + d ) + e), 0.0, 1.0);
}


vec3 Reinhart(vec3 x)
{
	vec3 mapped = x / (x + vec3(1.0));
	// gamma correction 
	//mapped = pow(mapped, vec3(1.0 / gamma));
  
	return mapped;
}

vec3 Uncharted2Tonemap(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;


	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

void calc_shadow(out float shadowFlagCombined, in int light_num)
{
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

	for(int i = 0; i < u_num_shadowmap; i++)
	{
		shadowpos = Vertex.shadowpos[i];
		shadowWdivide = shadowpos / shadowpos.w;
		if (shadowWdivide.w > 0)
		{
			vec4 depthmap = texture2D(depth[i], shadowWdivide.st);

			shadowFlag = depthmap.r < shadowWdivide.z - bias ? darkness : 1.0;
			for (int i = 0; i < 4; i++)
			{
				if ( texture( depth[i], shadowWdivide.st + poissonDisk[i] * 0.0014285 ).r  <  shadowWdivide.z - bias )
				{
					shadowFlag -= 0.2;
				}
			}

			shadowFlagCombined = max(shadowFlag, shadowFlagCombined);
		}
	}
}

vec3 lighting( int lightIndex, vec4 pos )
{
	vec3 lightPosWorld = pos.xyz;
	vec4 lightDir = vec4(lightPosWorld - Vertex.att_position, 1.0); // vector from pixel to light (terminal - origin)
	vec3 eye = -normalize(Vertex.vary_position.xyz); // vector from pixel to eye (eye at origin)
	lightDir.a = length(Vertex.att_position.rgb - lightPosWorld.rgb); // distance from light


	vec3 v_light = normalize(vec3(lightDir.rgb));
	vec3 n_light;
	vec3 v_light2;
	vec3 norm = normalize(Vertex.vary_normal);

	if (u_normalmap > 0.5)
	{
		vec3 normal_map;

		normal_map.x = u_normalmap_scale.x * (2 * texture(texture_normalmap, Vertex.vary_newTexCoord[0]).r - 1);
		normal_map.y = u_normalmap_scale.y * (2 * texture(texture_normalmap, Vertex.vary_newTexCoord[0]).g - 1);
		normal_map.z = u_normalmap_scale.z * (2 * texture(texture_normalmap, Vertex.vary_newTexCoord[0]).b - 1);

		// normal in tangent space
		norm = normal_map.xyz;

		// make tangent space matrix
		norm  = Vertex.vary_normal;
		vec3 tangent = normalize(vec3(Vertex.vary_tangent));
		vec3 bitangent = normalize(cross(norm, tangent));
		mat3 tangent_space = mat3(tangent, bitangent, norm);

		n_light = tangent_space * v_light; // move light vector from eye space to tangent space
	}


	vec3 v_reflect;
	float diffuse;

	float atten = min( u_atten_scale / pow(lightDir.a, u_atten_exponent), u_atten_min);		// light distance from fragment 1/(r^2) falloff
	if (u_normalmap > 0.5)
	{
		diffuse = max(dot(n_light, norm), u_diffuse_min);				// directional light factor for fragment
		v_reflect = reflect(-n_light, norm);					// normal map reflection vector
	}
	else
	{
		diffuse = max(dot(v_light, norm), u_diffuse_min);					// directional light factor for fragment
		v_reflect = reflect(-v_light, norm); // negate to get light vector towards surface
	}
	float specular = max(pow(dot(v_reflect, eye), u_specular_exponent), u_specular_min);			// specular relection for fragment


	return ( vec3(u_color[lightIndex]) * u_color[lightIndex].a )  * atten * (diffuse * u_diffuse_factor + specular * u_specular_factor); // combine everything
}

// was originally varying, but couldnt pass through geometry shader
vec4 lightDir;

void main(void)
{
	vec2 tc, tc0, tc1, tc2, tc3;
	vec3 unormal  = normalize(vec3(mvp * vec4(Vertex.vary_normal, 1.0)));
	tc.x = 0.0;
	tc.y = 0.0;



	vec3 ambient;
	ambient.x = u_ambient;
	ambient.y = u_ambient;
	ambient.z = u_ambient;
	vec3 light = vec3(0.0f, 0.0f, 0.0f);

	// scale and bias parallax effect
//	float height = texture(tex[2], Vertex.vary_TexCoord).a * 0.16 + -0.08;
//	Fragment = texture(tex[0], Vertex.vary_TexCoord + height * eye.xy);
//	vec3 normal_map = normalize(texture(texture_normalmap, Vertex.vary_TexCoord + height * eye.xy).xyz);



	vec4 Fragment_stage[4];



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

	if (u_env[0]  > 0 || u_env[1]  > 0 || u_env[2] > 0 || u_env[3] > 0)
	{
		vec3 u = normalize(-Vertex.vary_position.xyz);
		vec3 r = reflect(u, -unormal);

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


	if (u_lightmap_stage > 0)
	{
		// Scale and bias affects lightmaps on surfaces with shader, keeps e3window on q3tourney2 right brightness
		Fragment_stage[0] = texture(tex[0], Vertex.vary_LightCoord) * 2.0 + 1.0;
	}
	else
	{
		Fragment_stage[0] = texture(tex[0], tc0);
	}
	

	if (u_lightmap_stage > 0)
	{
		Fragment_stage[1] = texture(tex[1], Vertex.vary_LightCoord) * 2.0 + 1.0;
	}
	else
	{
		Fragment_stage[1] = texture(tex[1], tc1);
	}


	if (u_lightmap_stage > 0)
	{
		Fragment_stage[2] = texture(tex[2], Vertex.vary_LightCoord) * 2.0 + 1.0;
	}
	else
	{
		Fragment_stage[2] = texture(tex[2], tc2);
	}


	if (u_lightmap_stage > 0)
	{
		Fragment_stage[3] = texture(tex[3], Vertex.vary_LightCoord) * 2.0 + 1.0;
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
		Fragment_stage[i] *= min(u_rgbgen_scale[i], 3.0);
	}

	float pdrop = texture(texture_dissolve, Vertex.vary_newTexCoord[0]).r;

	if ( pdrop < u_dissolve)
	{
		discard;
	}

	// Dont write to fragment until after discard's
	Fragment = vec4(0.0, 0.0, 0.0, 0.0);
	for(int i = 0; i < 4; i++)
	{
		Fragment.xyz += Fragment_stage[i].xyz;
	}


//	alpha 1 = opaque, 0 equals transparent
	Fragment.a = Fragment_stage[0].a * Fragment_stage[1].a  * Fragment_stage[2].a  * Fragment_stage[3].a;
//	Fragment.a = 1.0;

//	Fragment.xyz = vec3(0.5,0.5,0.5);
//	Fragment.xyz = tangent;
	float shadow = 1.0;

	if (u_portal > 0)
	{
		// darken a bit
		Fragment.xyz *= 0.8;
		return;
	}


	for(int i = 0; i < u_num_lights; i++)
	{
		light += lighting(i, u_position[i]);
	}

	if (u_lightmap > 0.0 && u_normalmap <= 0.0)
	{
		vec3 lightmap = texture(texture_lightmap, Vertex.vary_LightCoord).xyz;

		light *= vec3(1.0 - u_lightmap, 1.0 - u_lightmap, 1.0 - u_lightmap);

		if (lightmap.r + lightmap.g + lightmap.b > 0.001)
			Fragment.xyz *= lightmap;
	}


	if (u_fog > 0.0)
	{
		float fog_distance = abs(Vertex.vary_position.z);
		float fog_factor;

		if (u_fog_end - fog_distance > 0.0f)
			fog_factor = (u_fog_end - fog_distance) / (u_fog_end - u_fog_start);
		else
			fog_factor = 0.0f;

		clamp(fog_factor, 0.0f, 1.0f);
		Fragment.rgb = mix(u_fog_color, Fragment.rgb, fog_factor);
	}


	Fragment.rgb *= max(light, ambient);



	if (u_shadowmap > 0.0)
	{
		calc_shadow(shadow, 0);
		Fragment.rgb *= shadow;
	}

	if (u_contrast < 0)
	{
		float value = (Fragment.r + Fragment.g + Fragment.b) * 0.3333333f;
		Fragment.r = value;
		Fragment.g = value;
		Fragment.b = value;
		return;
	}

	if (u_tone == 0)
	{
		Fragment.rgb = ACESFilm(Fragment.rgb * u_exposure);
	}
	else if (u_tone == 1)
	{
		Fragment.rgb = Uncharted2Tonemap(Fragment.rgb * u_exposure);
	}
	else if (u_tone == 2)
	{
		Fragment.rgb = Reinhart(Fragment.rgb * u_exposure);
	}


	if (u_lightmap_stage <= 0)
	{
		Fragment.rgb += u_brightness;
		Fragment.rgb *= u_contrast;
	}



	if (u_alpha >= 0.0)
	{
		Fragment.a = u_alpha;
	}


	normal_depth.xyz = unormal.xyz;
	normal_depth.a = Vertex.vary_position.z;

//	Fragment.rgb = light;

}
