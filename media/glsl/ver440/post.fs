#version 440 core

// Per vertex interpolated program input
in vec4 vary_position;
flat in int vary_color;
in vec2 vary_TexCoord;

// Final fragment color output
out vec4 Fragment;

// constant program input
layout(binding=0) uniform sampler2D texture0;
layout(binding=1) uniform sampler2D texture1;
layout(binding=2) uniform sampler2D texture2;

uniform vec2 tc_offset[9];

uniform int u_type;
uniform int u_time;

uniform int u_dir;
uniform float u_scale;
uniform float u_amount;
uniform float u_strength;

float GaussianFunction(float x, float dev)
{
	return ( (1.0 / sqrt(2.0 * 3.142857 * dev) ) * exp( -(x * x) / (2.0 * dev) ) );
} 

vec4 sampleAs3DTexture(sampler2D texture, vec3 uv, float width)
{
    float sliceSize = 1.0 / width;              // space of 1 slice
    float slicePixelSize = sliceSize / width;           // space of 1 pixel
    float sliceInnerSize = slicePixelSize * (width - 1.0);  // space of width pixels
    float zSlice0 = min(floor(uv.z * width), width - 1.0);
    float zSlice1 = min(zSlice0 + 1.0, width - 1.0);
    float xOffset = slicePixelSize * 0.5 + uv.x * sliceInnerSize;
    float s0 = xOffset + (zSlice0 * sliceSize);
    float s1 = xOffset + (zSlice1 * sliceSize);
    vec4 slice0Color = texture2D(texture, vec2(s0, uv.y));
    vec4 slice1Color = texture2D(texture, vec2(s1, uv.y));
    float zOffset = mod(uv.z * width, 1.0);
    vec4 result = mix(slice0Color, slice1Color, zOffset);
    return result;
}

vec3 rgb2hsb( vec3 c )
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), 
                 vec4(c.gb, K.xy), 
                 step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), 
                 vec4(c.r, p.yzx), 
                 step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), 
                d / (q.x + e), 
                q.x);
}

vec3 hsb2rgb( vec3 c )
{
    vec3 rgb = clamp(abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),
                             6.0)-3.0)-1.0, 
                     0.0, 
                     1.0 );
    rgb = rgb*rgb*(3.0-2.0*rgb);
    return c.z * mix(vec3(1.0), rgb, c.y);
}


void main(void)
{
	if (u_type == 0)
	{
		vec4 texsample[9];
	
		for (int i = 0; i < 9; i++)
		{
			texsample[i] = texture2D(texture0, vary_TexCoord + tc_offset[i]);
		}

		// laplacian edge detect
		Fragment =  (texsample[4] * 8.0) - 
			(texsample[0] + texsample[1] + texsample[2] + 
			texsample[3] + texsample[5] + 
			texsample[6] + texsample[7] + texsample[8]);
	}
	else if (u_type == 1)
	{
		vec4 texsample[9];
	
		for (int i = 0; i < 9; i++)
		{
			texsample[i] = texture2D(texture0, vary_TexCoord + tc_offset[i]);
		}


		// gaussian blur
		Fragment = (1.0 * texsample[0] + 2.0 * texsample[1] + 1.0 * texsample[2] + 
			2.0 * texsample[3] + 4.0 * texsample[4] + 2.0 * texsample[5] + 
			1.0 * texsample[6] + 2.0 * texsample[7] + 1.0 * texsample[8]) / 16.0;
	}
	else if (u_type == 2)
	{
		vec4 texsample[9];
	
		for (int i = 0; i < 9; i++)
		{
			texsample[i] = texture2D(texture0, vary_TexCoord + tc_offset[i]);
		}

		// embosss
		Fragment = (-2.0 * texsample[0] + -1.0 * texsample[1] +
			-1.0 * texsample[3] + 1.0 * texsample[4] + 1.0 * texsample[5] + 
						1.0 * texsample[7] + 2.0 * texsample[8]) / 16.0;
	}
	else if (u_type == 3)
	{
		vec4	color = vec4(0.0,0.0,0.0,0.0);
		vec4	temp =  vec4(0.0,0.0,0.0,0.0);
		float	strength = 1.0 - (2 * u_strength);
		float	half1 = float(u_amount*2) * 0.5;
		float	texel = (1.0/256.0);
		float	dev = u_amount*2 * 0.5 * 0.5;
		int	count = int(u_amount*2);
		vec3 original = texture2D(texture0, vary_TexCoord).rgb;
		float avg = (original.r + original.g + original.b) / 3.0;

	
		dev *= dev;
		if (u_dir == 0) 
		{
			for (int i = 0; i < count; i++)
			{
				float offset = float(i) - half1;
				temp = texture2D(texture0, vary_TexCoord + vec2(offset * texel * u_scale, 0.0)) * GaussianFunction(offset * u_strength, dev);
				color += temp;
			}
		} 
		else 
		{
			for (int i = 0; i < count; i++)
			{
				float offset = float(i) - half1;
				temp = texture2D(texture0, vary_TexCoord + vec2(0.0, offset * texel * u_scale)) * GaussianFunction(offset *  u_strength, dev);

				color += temp;
			}
		}
	  
		Fragment = clamp(color, 0.0, 1.0);
		Fragment.w = 1.0;
	}
	else if (u_type == 4)
	{
		Fragment = texture2D(texture0, vary_TexCoord) * 1.0 +  texture2D(texture1, vary_TexCoord) * 0.5 + texture2D(texture2, vary_TexCoord) * 0.5;
	}
	else if (u_type == 5)
	{
		vec4 original = texture2D(texture0, vary_TexCoord);
		float avg = (original.r + original.g + original.b) / 3.0;
		float threshold = u_scale;
		if (avg > threshold)
		{
			Fragment = original;
		}
		else
		{
			Fragment = vec4(0,0,0,0);
		}
	}
	else if (u_type == 6)
	{
		float exposure = 5.0;
		float decay = 1.0f;
		float density = 0.5f;
		float weight = 0.5f;

		vec2 deltaTextCoord = vec2( vary_TexCoord.st - vec2(512,512) );
		vec2 textCoo = vary_TexCoord.st;
		deltaTextCoord *= 1.0 /  float(100) * density;
		float illuminationDecay = 1.0;


		Fragment = texture2D(texture0, vary_TexCoord);
		
		for(int i = 0; i < 100 ; i++)
		{
		         textCoo -= deltaTextCoord;
		         vec4 samp = texture2D(texture0, textCoo );
				
		         samp *= illuminationDecay * weight;
		
		         Fragment += samp;
		
		         illuminationDecay *= decay;
		 }

		Fragment *= exposure;
	}
	else if (u_type == 7)
	{
		vec3 resolution; // screen resolution
		resolution = vec3(1024, 1024, 1024);

		vec3 p = gl_FragCoord.xyz / resolution - 0.5;
		vec3 o = texture2D(texture0, 0.5 + (p.xy *= 0.992)).rbb;
		for (float i = 0.0; i < 100.0; i++) 
		{
			p.z += pow(max(0.0f, 0.5f - length(texture2D(texture0, 0.5 + (p.xy *= 0.992)).rg)), 2.0) * exp(-i * 0.08);
		}
		Fragment = vec4(o * o + p.z, 1) - vec4(1.5f, 1.5f, 1.5f, 0.0f);
	}
	else if (u_type == 8)
	{
		// Apply the color grading
		vec4 gradedPixel = sampleAs3DTexture(texture1, texture2D(texture0, vary_TexCoord).rgb, 16);
		gradedPixel.a = Fragment.a;
		Fragment = gradedPixel;
	}
	else if (u_type == 9)
	{
		vec2 texcoord = vary_TexCoord;
		texcoord.x += sin(texcoord.y * 3.14159 + u_time / 50.0f) / 100.0f;
		Fragment = texture2D(texture0, texcoord);
		Fragment.b = Fragment.b + (1.0f - Fragment.b) * 0.2f;
	}
	else if (u_type == 10)
	{
		vec4 temp = texture2D(texture0, vary_TexCoord);
		vec3 color = rgb2hsb( temp.rgb );
		color.r = u_scale;
		color.g = u_strength;
//		color.b = u_amount;

		Fragment.rgb = hsb2rgb( color );
		Fragment.a = temp.a;
	}
	else if (u_type == 11)
	{
		vec4 original = texture2D(texture0, vary_TexCoord);
		float min_dof = u_strength;
		float max_dof = u_amount;

		float z = texture(texture1, vary_TexCoord).r;      // fetch the z-value from our depth texture
		float n = 1.0;                                // the near plane
		float f = 2001.0;                               // the far plane
		float depth = (2.0 * n) / (f + n - z * (f - n));  // convert to linear values 
 

		Fragment = original * (1.0f - min(smoothstep(min_dof, min_dof + 0.1, depth), smoothstep(max_dof, max_dof - 0.1, depth)) );
	}


}



/*


// box blur
//   1 2 1
//   2 1 2   / 13
//   1 2 1

// edge detection
//    1  2  1
//    0  0  0   / 1
//   -1 -2 -1

// Sharpening
//     0  -2  0
//    -2  11 -2   / 3
//     0  -2  0

// Depth of field
//	H = (L * L) / (F * D)  - hyperfocal distance (dist to focused object)
//	NF = H * D / (H+(D-L)) - Near focus limit
//	FF = H * D / (H-(D-L)) - Far focus limit
//	L = lens focal length, 35mm 105mm... dist from center of lens to receiver
//	f = lens aperature (fstop)
//	d = diameter of least circle of confusion (sharpness)
//	D = lens focal distance
// Render blurry image, copy good pixels and blurred pixels based on Z distance

//heat shimmer, random offsets in a distortion texture, particle system to emits distortion from source


// exposure / hdr / glare effect
// exposure level = average intensity of image, normalize to 0.5 over time
// color = color * alpha


// star bursts
// directional star blur

// ghosts
// flip scale a bunch of times, mask out edges with circle blur

//directional lighting
// point lighting
// spot light, obstructed point light
// area lights (point light with area)

//spot light equations
// I = (cos alpha - cos thi) / (cos theta - cos thi)
// alpha = current lighting angle within cone
// theta = inner cone angle
// thi = outer cone angle

//bdrfs - better than phong shading


//height map to normal map

//dh/du = slope in U direction
//dh/dv = slope in V direction
//Len = sqrt(dh/du + dh/dv + 1)

//bumpmap.x = (dh/hu) / len
//bumpmap.y = (dh/dv) / len
//bumpmap.z = 1 / len

//tangent = vector along U
//binormal = vector along V

*/
