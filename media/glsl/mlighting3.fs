#version 150
#define	MAX_LIGHTS 24

// per vertex interpolated program input
in vec4		vary_position;
flat in int		vary_color;
in vec2		vary_TexCoord;
in vec2		vary_LightCoord;
in vec3		vary_normal;
in vec4		vary_light[MAX_LIGHTS];
in vec3		vary_tangent;

// Final fragment color output
out vec4 Fragment;

// constant program input
uniform vec3		u_position[MAX_LIGHTS];
uniform vec4		u_color[MAX_LIGHTS];
uniform int		u_num_lights;
uniform mat4		mvp;
uniform sampler2D	texture0, texture1, texture2;

void main(void)
{
	vec3 normal = normalize(vary_normal);
	vec3 tangent = normalize(vary_tangent);
	vec3 bitangent = normalize(cross(normal, tangent));
	mat3 tangent_space = mat3(tangent, bitangent, normal);
	vec3 eye = tangent_space * -normalize(vary_position.xyz);
	vec3 intensity = vec3(1.0, 1.0, 1.0);

	// scale and bias parallax effect
//	float height = texture2D(texture2, vary_TexCoord).a * 0.16 + -0.08;
//	Fragment = texture2D(texture0, vary_TexCoord + height * eye.xy);
//	vec3 normal_map = normalize(texture2D(texture2, vary_TexCoord + height * eye.xy).xyz);

	vec3 normal_map = normalize(texture2D(texture2, vary_TexCoord).xyz);
	Fragment = texture2D(texture0, vary_TexCoord);

	for(int i = 0; i < u_num_lights; i++)
	{
		vec3 v_light = tangent_space * normalize(vec3(vary_light[i]));
		vec3 v_reflect = reflect(v_light, normal_map);
		float diffuse = max(dot(v_light, normal_map), 0.0) * 0.5;
		float specular = pow(max(dot(v_reflect, eye), 0.0), 25.0) * 0.5;
		float atten = min(40000.0 / pow(vary_light[i].a, 1.75), 0.75);
		intensity = intensity + atten * (diffuse + specular + vec3(u_color[i]) * 0.25);
	}
	Fragment.rgb *= intensity;
}
