#version 330

layout(points) in;
layout(points) out;
layout(max_vertices = 40) out;

in	vec3		ivary_position[];
in	vec2		ivary_TexCoord[];
in	vec2		ivary_LightCoord[];
in	vec3		ivary_velocity[];
in	int		ivary_color[];
in	vec4		ivary_tangent[];
//	float		ivary_life[];
//	float		ivary_size[];
//	float		ivary_type[];
//	float		ivary_pad0[];

 
out	vec3 vary_position;
out	vec2 vary_TexCoord;
out	vec2 vary_LightCoord;
out	vec3 vary_velocity;
out	flat vary_color;
out	vec4 vary_tangent;
//	float vary_life;
//	float vary_size;
//	float vary_type;
//	float vary_pad0;


uniform vec3	u_gen_position;
uniform vec3	u_gen_vel_min;
uniform vec3	u_gen_vel_range;
uniform int	u_gen_color;
uniform float	u_gen_size; 
uniform float	u_gen_life_min;
uniform float	u_gen_life_range;
uniform int	u_gen_num; 

uniform vec3	u_gravity;
uniform float	u_delta_time;
uniform vec3	u_seed;


vec3 local_seed;


// This function returns random number from zero to one
float rand_zero_one()
{
	uint n = floatBitsToUint(local_seed.y * 214013.0 + local_seed.x * 2531011.0 + local_seed.z * 141251.0);
	n = n * (n * n * 15731u + 789221u);
	n = (n >> 9u) | 0x3F800000u;

	float fRes =  2.0 - uintBitsToFloat(n);
	local_seed = vec3(local_seed.x + 147158.0 * fRes, local_seed.y*fRes  + 415161.0 * fRes, local_seed.z + 324154.0 * fRes);
	return fRes;
}

void main()
{
	local_seed = u_seed;
	
	// gl_Position doesn't matter, rendering is discarded
	
	vary_position = ivary_position[0];
	vary_velocity = ivary_velocity[0];

	if(ivary_tangent[0].z < 0) //type < 0
	{
		vary_position += vary_velocity * u_delta_time;
		vary_velocity += u_gravity * u_delta_time;
	}
	
	vary_color	= ivary_color[0];

	vary_tangent.x	= ivary_tangent[0].x - u_delta_time; //subtract from particle life
	vary_tangent.y	= ivary_tangent[0].y; // pass size through
	vary_tangent.z	= ivary_tangent[0].z; // pass type through
	vary_tangent.w	= ivary_tangent[0].w; // isnt used

	vary_TexCoord = vec2(0.0, 0.0);
	vary_LightCoord = vec2(1.0, 1.0);

//	vary_life	= ivary_life[0] - u_delta_time;
//	vary_size	= ivary_size[0];
//	vary_type	= ivary_type[0];
	  
	if(vary_tangent.z > 0) // type > 0 = generate particles
	{
		// Keep generator particle
		EmitVertex();
		EndPrimitive();
		
		// Generate partiles based on uniform count
		for(int i = 0; i < u_gen_num; i++)
		{
			vary_position = u_gen_position;
			vary_velocity = u_gen_vel_min + vec3(	u_gen_vel_range.x * rand_zero_one(),
							u_gen_vel_range.y * rand_zero_one(),
							u_gen_vel_range.z * rand_zero_one());
			vary_color	= u_gen_color;

			vary_tangent.x = u_gen_life_min + u_gen_life_range * rand_zero_one();
			vary_tangent.y = u_gen_size;
			vary_tangent.z = -1.0;
			vary_tangent.w = 1.0f;

			vary_TexCoord = vec2(0.0, 0.0);
			vary_LightCoord = vec2(1.0, 1.0);

//			vary_life	= u_gen_life_min + u_gen_life_range * rand_zero_one();
//			vary_size	= u_gen_size;
//			vary_type	= 1;
			EmitVertex();
			EndPrimitive();
		}
	}
	else if(vary_tangent.x > 0.0) // life > 0
	{
		EmitVertex();
		EndPrimitive(); 
	}
}