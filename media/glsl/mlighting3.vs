#version 150
#define	MAX_LIGHTS 24

/*
in int gl_VertexID;
in int gl_InstanceID;
out gl_PerVertex
{
	vec4 gl_Position;
	float gl_PointSize;
	float gl_ClipDistance[];
};
*/

// Per vertex attribute input
in vec3 attr_position;
in vec2 attr_TexCoord;
in vec2 attr_LightCoord;
in vec3 attr_normal;
in int	attr_color;
in vec3	attr_tangent;

// interpolated output
out vec4	vary_position;
out vec2	vary_TexCoord;
out vec2	vary_LightCoord;
out int		vary_color;
out vec3	vary_normal;
out vec4	vary_light[MAX_LIGHTS];
out vec3	vary_tangent;

// uniform primitive constant input
uniform vec3	u_position[MAX_LIGHTS];
uniform vec4	u_color[MAX_LIGHTS];
uniform int	u_num_lights;
uniform mat4	mvp;

void main(void)
{
	//pass through to fragment shader
	vary_normal = attr_normal;
	vary_TexCoord = attr_TexCoord;
	vary_LightCoord = attr_LightCoord;
	vary_color = attr_color;
	vary_tangent = attr_tangent;

	gl_Position = mvp * vec4(attr_position, 1.0);
	vary_position = mvp * vec4(attr_position, 1.0);

	for(int i = 0; i < u_num_lights; i++)
	{
		vec4 csPos = mvp * vec4(u_position[i], 1.0);
		vary_light[i].rgb = normalize(vec3(csPos - gl_Position));
		vary_light[i].a = length(vec3(csPos - gl_Position));
	}
}