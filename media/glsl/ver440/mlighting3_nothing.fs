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
uniform mat4		mvp;


uniform int u_lightmap_stage;
uniform int u_depth;
uniform float u_brightness;
uniform float u_contrast;
uniform float u_exposure;

uniform int u_env[4];
uniform float u_rgbgen_scale[4];
uniform int u_alphatest[4];
uniform int u_portal;
uniform int u_normalmap;

uniform float u_fog;
uniform float u_fog_start;
uniform float u_fog_end;
uniform vec3 u_fog_color;



uniform sampler2D tex[4];// 4 possible textures

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
	vec3 eye = -normalize(Vertex.vary_position.xyz); // for specular reflections

	vec3 norm  = Vertex.vary_normal;
	vec3 tangent = normalize(vec3(mvp * Vertex.vary_tangent));
	vec3 bitangent = normalize(cross(norm, tangent));
	mat3 tangent_space = mat3(tangent, bitangent, norm);


	vec4 lightDir = mvp * vec4(Vertex.att_position - lightPosWorld, 1.0);
	lightDir.a = length(Vertex.att_position.rgb - lightPosWorld.rgb); // distance from light


	vec3 v_light = normalize(vec3(lightDir.rgb));	
//	vec3 n_light = tangent_space * v_light; // light vector in tangent space

	if (u_normalmap > 0)
	{
		vec3 normal_map;

		normal_map.x = (2 * texture(texture_normalmap, Vertex.vary_newTexCoord[0]).r - 1);
		normal_map.y = (2 * texture(texture_normalmap, Vertex.vary_newTexCoord[0]).g - 1);
		normal_map.z = (2 * texture(texture_normalmap, Vertex.vary_newTexCoord[0]).b - 1);

		norm = 2 * (transpose(mvp) * vec4(normal_map, 1.0)).xyz;


		vec4 lightpos = mvp * vec4(lightPosWorld, 1.0);
		vec3 v_light2 = normalize(vec3(lightpos.xyz));
		vec3 n_light = tangent_space * v_light2; // light vector in tangent space
	}


	float diffuse = max(dot(v_light, norm), 0.25);					// directional light factor for fragment
	float atten = min( pos.a * 160000.0 / pow(lightDir.a, 2.25), 0.25);		// light distance from fragment 1/(r^2) falloff
	vec3 v_reflect = reflect(v_light, norm);					// normal map reflection vector
	float specular = max(pow(dot(v_reflect, eye), 8.0), 0.25);			// specular relection for fragment


	return ( vec3(u_color[lightIndex]) * u_color[lightIndex].a )  * atten * (diffuse * 0.75 + specular * 0.0); // combine everything
}

// was originally varying, but couldnt pass through geometry shader
vec4 lightDir;

void main(void)
{
	Fragment.rgb = vec3(texture(tex[0], Vertex.vary_TexCoord));// + texture(tex[1], Vertex.vary_TexCoord) + texture(tex[2], Vertex.vary_TexCoord) + texture(tex[3], Vertex.vary_TexCoord));
	Fragment.a = 1.0f;
}
