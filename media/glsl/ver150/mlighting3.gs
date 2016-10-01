#version 150 
#extension GL_EXT_geometry_shader4 : enable
#define MAX_LIGHTS 24


// constant program input
uniform vec3		u_position[MAX_LIGHTS];
uniform vec4		u_color[MAX_LIGHTS];
uniform int		u_lights;
uniform mat4		mvp;
uniform sampler2D	texture0, texture1, texture2;

//input
inout vec4		vary_position[gl_VerticesIn];
inout vec2		vary_TexCoord[gl_VerticesIn];
inout vec2		vary_LightCoord[gl_VerticesIn];
inout int		vary_color[gl_VerticesIn];
inout vec3		vary_normal[gl_VerticesIn];
//inout vec4		vary_light[MAX_LIGHTS][gl_VerticesIn];



void main(void)
{
	for(int i = 0;i < gl_VerticesIn; i++)
	{
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}

/*
in gl_PerVertex
{
	vec4 gl_Position;
	float gl_PointSize;
	float gl_ClipDistance[];
} gl_in[];

in int gl_PrimitiveIDIn;

out gl_PerVertex
{
	vec4 gl_Position;
	float gl_PointSize;
	float gl_ClipDistance[];
};
out int gl_PrimitiveID;
out int gl_Layer;
*/
