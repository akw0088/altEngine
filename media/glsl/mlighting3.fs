#version 440 core

#define	MAX_LIGHTS 16

// per vertex interpolated program input
in VertexDataOut {
    vec4 vary_position; // vertex position
    vec2 vary_TexCoord;
    vec2 vary_LightCoord;
    vec3 vary_normal;
    flat int vary_color;
    vec3 vary_tangent;
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

// was originally varying, but couldnt pass through geometry shader
vec4 vary_light;

void main(void)
{
	vec3 normal = normalize(Vertex.vary_normal);
	vec3 tangent = normalize(Vertex.vary_tangent);
	vec3 bitangent = normalize(cross(normal, tangent));
	mat3 tangent_space = mat3(tangent, bitangent, normal);
	vec3 eye = tangent_space * -normalize(Vertex.vary_position.xyz); // eye vector in tangent space
	vec3 ambient = vec3(0.125f, 0.125f, 0.125f);
	vec3 light = vec3(0.0f, 0.0f, 0.0f);

	// scale and bias parallax effect
//	float height = texture(texture2, Vertex.vary_TexCoord).a * 0.16 + -0.08;
//	Fragment = texture(texture0, Vertex.vary_TexCoord + height * eye.xy);
//	vec3 normal_map = normalize(texture(texture2, Vertex.vary_TexCoord + height * eye.xy).xyz);

	vec3 normal_map = normalize(texture(texture2, Vertex.vary_TexCoord).xyz);
	Fragment = texture(texture0, Vertex.vary_TexCoord);


	for(int i = 0; i < u_num_lights; i++)
	{
		vec4 lightPosEye = normalize(mvp * vec4(u_position[i], 1.0));
		vary_light.rgb = vec3(Vertex.vary_position - lightPosEye); // light vector to fragment
		vary_light.a = length(vary_light.rgb); // distance from light

		vec3 v_light = tangent_space * normalize(vec3(vary_light));	// light vector in tangent space
		vec3 v_reflect = reflect(v_light, normal_map);			// normal map reflection vector
		float diffuse = max(dot(v_light, normal_map), 0.25);		// directional light factor for fragment
		float specular = max(pow(dot(v_reflect, eye), 8.0), 0.25);	// specular relection for fragment
		float atten = min( 40000.0 / pow(vary_light.a, 1.75), 0.25);	// light distance from fragment 1/(r^2) falloff
		light = light + ( vec3(u_color[i]) * u_color[i].a )  * atten * (diffuse * 0.75 + specular * 0.1); // combine everything
	}

	Fragment.rgb *= max(light, ambient);

}
