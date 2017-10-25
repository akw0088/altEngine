#version 440 core

// Per vertex interpolated program input
in vec4 vary_position;
flat in int vary_color;
in vec2 vary_TexCoord;

// Final output
layout (location = 0) out vec4 color;

// Samplers for pre-rendered color, normal and depth
layout (binding = 0) uniform sampler2D sColor;
layout (binding = 1) uniform sampler2D sNormalDepth;

// constant program input
uniform mat4		mvp;
uniform int u_depth;
layout(binding=0) uniform sampler2D texture0;

// Various uniforms controling SSAO effect
uniform float ssao_level = 1.0;
uniform float object_level = 1.0;
uniform float ssao_radius = 5.0;
uniform bool weight_by_angle = true;
uniform uint point_count = 8;
uniform bool randomize_points = true;
uniform int u_width;
uniform int u_height;

// Uniform block containing up to 256 random directions (x,y,z,0)
// and 256 more completely random vectors
layout (binding = 0, std140) uniform SAMPLE_POINTS
{
    vec4 pos[256];
    vec4 random_vectors[256];
} points;


// A re-weighted Gaussian kernel
const vec3 chromaticAberrationKernel[9] = vec3[9](
vec3(0.0000000000000000000, 0.04416589065853191, 0.0922903086524308425), vec3(0.10497808951021347), vec3(0.0922903086524308425, 0.04416589065853191, 0.0000000000000000000),
vec3(0.0112445223775533675, 0.10497808951021347, 0.1987116566428735725), vec3(0.40342407932501833), vec3(0.1987116566428735725, 0.10497808951021347, 0.0112445223775533675),
vec3(0.0000000000000000000, 0.04416589065853191, 0.0922903086524308425), vec3(0.10497808951021347), vec3(0.0922903086524308425, 0.04416589065853191, 0.0000000000000000000)
);

// Accumulate an approximation of chromatic aberration using a specially weighted kernel.
// resolution - Screen or texture resolution to sample each pixel.
// coordinate - Screen space coordinate in pixels.
// texture - The texture to sample.
vec4 chromaticAberration(vec2 resolution, vec2 coordinate, sampler2D texture)
{
	vec3 chromaticAberrationSample = vec3(0.0);
	
	for(int y = -1; y < 2; y++)
	{
		for(int x = -1; x < 2; x++)
		{
			chromaticAberrationSample += texture2D(texture, ((coordinate + vec2(x,y)) / resolution)).rgb * chromaticAberrationKernel[((y + 1) * 3) + (x + 1)];
		}
	}
	return vec4(chromaticAberrationSample, texture2D(texture, coordinate / resolution).a);
}

void main(void)
{
    // Get texture position from gl_FragCoord
    vec2 P = gl_FragCoord.xy / textureSize(sNormalDepth, 0);
    // ND = normal and depth
    vec4 ND = textureLod(sNormalDepth, P, 0);
    // Extract normal and depth
    vec3 N = ND.xyz;
    float my_depth = ND.w;

    // Local temporary variables
    int i;
    int j;
    int n;

    float occ = 0.0;
    float total = 0.0;

    // n is a pseudo-random number generated from fragment coordinate and depth
    n = (int(gl_FragCoord.x * 7123.2315 + 125.232) *
         int(gl_FragCoord.y * 3137.1519 + 234.8)) ^
         int(my_depth);
    // Pull one of the random vectors
    vec4 v = points.random_vectors[n & 255];

    // r is our 'radius randomizer'
    float r = (v.r + 3.0) * 0.1;
    if (!randomize_points)
        r = 0.5;

    // For each random point (or direction)...
    for (i = 0; i < point_count; i++)
    {
        // Get direction
        vec3 dir = points.pos[i].xyz;

        // Put it into the correct hemisphere
        if (dot(N, dir) < 0.0)
            dir = -dir;

        // f is the distance we've stepped in this direction
        // z is the interpolated depth
        float f = 0.0;
        float z = my_depth;

        // We're going to take 4 steps - we could make this
        // configurable
        total += 4.0;

        for (j = 0; j < 4; j++)
        {
            // Step in the right direction
            f += r;
            // Step _towards_ viewer reduces z
            z -= dir.z * f;

            // Read depth from current fragment
            float their_depth =
                textureLod(sNormalDepth,
                           (P + dir.xy * f * ssao_radius), 0).w;

            // Calculate a weighting (d) for this fragment's
            // contribution to occlusion
            float d = abs(their_depth - my_depth);
            d *= d;

            // If we're obscured, accumulate occlusion
            if ((z - their_depth) > 0.0)
            {
                occ += 4.0 / (1.0 + d);
            }
        }
    }

    // Calculate occlusion amount
    float ao_amount = (1.0 - occ / total);

    // Get object color from color texture
//    color = chromaticAberration(vec2(u_width,u_height), gl_FragCoord.xy, texture0);
    vec4 object_color =  chromaticAberration(vec2(u_width,u_height), gl_FragCoord.xy, texture0);;
//textureLod(sColor, P, 0);


    // Mix in ambient color scaled by SSAO level

    color = object_level * object_color +
            mix(vec4(0.2), vec4(ao_amount), ssao_level);
	color.a = 1.0f;



//color = vec4(1.0,1.0,1.0,1.0);
}
