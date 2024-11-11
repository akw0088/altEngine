//=============================================================================
// This file is part of the altEngine distribution
// (https://github.com/akw0088/altEngine/)
// Copyright (c) 2018 Alexander Wright All Rights Reserved.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//=============================================================================

// this code is here, but needs lots of work

#include "raytrace.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif




#define red x
#define green y
#define blue z


using namespace raytrace;

float srgbEncode(float c)
{
	if (c <= 0.0031308f)
	{
		return 12.92f * c;
	}
	else
	{
		return 1.055f * powf(c, 0.4166667f) - 0.055f; // Inverse gamma 2.4
	}
}



bool hitSphere(const raytrace::ray_t &r, const raytrace::sphere_t& s, float &t)
{
	vec3 dist = s.pos - r.start;
	float B = (r.dir.x * dist.x + r.dir.y * dist.y + r.dir.z * dist.z);
	float D = B * B - dist * dist + s.size * s.size;

	if (D < 0.0f)
		return false;

	float t0 = B - sqrtf(D);
	float t1 = B + sqrtf(D);

	bool retvalue = false;

	if ((t0 > 0.1f) && (t0 < t))
	{
		t = t0;
		retvalue = true;
	}
	if ((t1 > 0.1f) && (t1 < t))
	{
		t = t1;
		retvalue = true;
	}
	return retvalue;
}


bool hitPlane(const raytrace::ray_t &r, rplane_t &p, float &t)
{
	float dot = p.n * r.dir;

	if (dot == 0)
		return 0;

	float d = ((p.p - r.start) * p.n) / dot;

	t = d;

	return 1;
}


bool hitTriangle2(const raytrace::ray_t &r, const raytrace::triangle_t& p, float &t, int width, int height)
{
	vec3 origin = r.start;
	vec3 dir = r.dir;

	vec3 span1 = p.b - p.a;
	vec3 span2 = p.c - p.a;
	vec3 normal = vec3::crossproduct(span1, span2);
	float d = -(normal.x * p.a.x + normal.y * p.a.y + normal.z * p.a.z);

	float denom = (normal.x * dir.x + normal.y * dir.y + normal.z * dir.z);
	if (denom == 0.0f)
		return false;

	t = -(origin * normal + d) / denom;
	if (t < 0.0)
		return false;

	// hit plane, check barycentric coordinates
		t = 1.0f;

	float lam1, lam2, lam3;

	if (get_barycentric(r.start.x / width, r.start.y / height, p.a, p.b, p.c, lam1, lam2, lam3) == false)
		return false;

	if (lam1 > 0.0f && lam1 < 1.0f && lam2 > 0.0f && lam2 < 1.0f && lam3 > 0.0f && lam3 < 1.0f)
		return true;
	else
		return false;
}

bool hitTriangle(const raytrace::ray_t &ro, raytrace::triangle_t &tri, float &t, int width, int height)
{
	vec3 v0v1 = tri.b - tri.a;
	vec3 v0v2 = tri.c - tri.a;

	raytrace::ray_t r = ro;

	r.start.x /= width;
	r.start.y /= height;

	vec3 n = vec3::crossproduct(v0v1, v0v2);
	float area2 = n.magnitude();

	float NdotRayDirection = n * r.dir;

	// ray and plane are parallel
	if (fabs(NdotRayDirection) < 0.0001f)
		return false;

	float d = n * tri.a;

	t = ((n * r.start) + d) / NdotRayDirection;

	t = 1.0f;

	// check if the triangle is in behind the ray
	if (t < 0)
		return false;

	// compute the intersection point
	vec3 P = r.start + r.dir * t;

	// vector perpendicular to triangle's plane
	vec3 vperp;

	// edge 0
	vec3 edge0 = tri.b - tri.a;
	vec3 vp0 = P - tri.a;

	vperp = vec3::crossproduct(edge0, vp0);

	// check if P outside edge0
	if (n * vperp < 0)
		return false;

	// edge 1
	vec3 edge1 = tri.b - tri.a;
	vec3 vp1 = P - tri.a;
	vperp = vec3::crossproduct(edge1, vp1);

	// check if P outside edge1
	if (n * vperp < 0)
		return false;

	// edge 2
	vec3 edge2 = tri.a - tri.b;
	vec3 vp2 = P - tri.b;
	vperp = vec3::crossproduct(edge2, vp2);

	// check if P outside edge2
	if (n * vperp < 0)
		return false;

	return true;
}


color_t addRay(raytrace::ray_t viewRay, vertex_t *vertex_array, int *index_array, int num_vert, int num_index, light_t *light, int num_light, int width, int height, matrix4 &mvp)
{
	color_t output = { 0.0f, 0.0f, 0.0f };
	float coef = 1.0f;
	int level = 0;

	do
	{
		vec3 ptHitPoint;
		int currentTriangle = -1;
		float t = 2000.0f;

		material_t currentMat;
		vec3 vNormal;

		// note this is done *PER PIXEL*, should definitely do a smaller windowing
		for (int i = 0; i < num_index; i += 3)
		{
			raytrace::triangle_t triangle;
			vec4 a, b, c;

			// project to eye space
			a = mvp * vec4(vertex_array[index_array[i]].position, 1.0f);
			b = mvp * vec4(vertex_array[index_array[i+1]].position, 1.0f);
			c = mvp * vec4(vertex_array[index_array[i+2]].position, 1.0f);


			// perspective divide
			float inv = 1 / a.w;
			a.x = a.x * inv;
			a.y = a.y * inv;
			a.z = a.z * inv;
			a.w = 1.0f;

			inv = 1 / b.w;
			b.x = b.x * inv;
			b.y = b.y * inv;
			b.z = b.z * inv;
			b.w = 1.0f;


			inv = 1 / c.w;
			c.x = c.x * inv;
			c.y = c.y * inv;
			c.z = c.z * inv;
			c.w = 1.0f;


			// cull
			if (a.x > 1.0f || a.x < -1.0f)
				continue;
			if (a.y > 1.0f || a.y < -1.0f)
				continue;
			if (a.z > 1.0f || a.z < -1.0f)
				continue;

			if (b.x > 1.0f || b.x < -1.0f)
				continue;
			if (b.y > 1.0f || b.y < -1.0f)
				continue;
			if (b.z > 1.0f || b.z < -1.0f)
				continue;

			if (c.x > 1.0f || c.x < -1.0f)
				continue;
			if (c.y > 1.0f || c.y < -1.0f)
				continue;
			if (c.z > 1.0f || c.z < -1.0f)
				continue;


			// viewport transform
			a.x *= 0.5f;
			a.y *= 0.5f;
			a.z *= 0.5f;
			a.x += 0.5f;
			a.y += 0.5f;
			a.z += 0.5f;

			b.x *= 0.5f;
			b.y *= 0.5f;
			b.z *= 0.5f;
			b.x += 0.5f;
			b.y += 0.5f;
			b.z += 0.5f;

			c.x *= 0.5f;
			c.y *= 0.5f;
			c.z *= 0.5f;
			c.x += 0.5f;
			c.y += 0.5f;
			c.z += 0.5f;


			triangle.a = vec3(a);
			triangle.b = vec3(b);
			triangle.c = vec3(c);

			if (hitTriangle2(viewRay, triangle, t, width, height))
			{
				currentTriangle = i;
			}
		}

		if (currentTriangle == -1)
			break;

		ptHitPoint = viewRay.start + viewRay.dir * t;

		if (currentTriangle != -1)
		{
			int i = currentTriangle;
			vec3 normal = vertex_array[index_array[i]].normal;
			vNormal = ptHitPoint - normal;

			vec3 norm = vec3(0.0f, 0.0f, -1.0f);

			vNormal = norm * (norm * viewRay.dir);
			vNormal *= -2.0;
			vNormal += viewRay.dir;

			//currentMat = myScene.materialContainer[myScene.planeContainer[0].materialId];
			currentMat.diffuse = vec3(0.5f, 0.0f, 0.0f);
			currentMat.power = 0;
			currentMat.reflection = 0.4f;
			currentMat.specular = vec3(0.5f, 0.5f, 0.5f);
		}

		float temp = vNormal * vNormal;
		if (temp == 0.0f)
			break;

		temp = 1.0f / sqrtf(temp);
		vNormal = vNormal * temp;

		raytrace::ray_t lightRay;
		lightRay.start = ptHitPoint;

		for (int j = 0; j < num_light; j++)
		{
			light_t currentLight = light[j];

			lightRay.dir = currentLight.pos - ptHitPoint;
			float fLightProjection = lightRay.dir * vNormal;

//			if (fLightProjection <= 0.0f)
//				continue;

			float lightDist = lightRay.dir * lightRay.dir;

			{
				float temp = lightDist;
				if (temp == 0.0f)
					continue;
				temp = invsqrtf(temp);
				lightRay.dir = lightRay.dir * temp;
				fLightProjection = temp * fLightProjection;
			}

			bool inShadow = false;

#if 0
			// Shadow hit test
			{
				float t = lightDist;
				for (unsigned int i = 0; i < num_index; i += 3)
				{
					triangle_t triangle;

					triangle.a = vertex_array[index_array[i]].position;
					triangle.b = vertex_array[index_array[i+1]].position;
					triangle.c = vertex_array[index_array[i+2]].position;

					if (hitTriangle(lightRay, triangle, t))
					{
						inShadow = true;
						break;
					}
				}
			}
#endif
			lightRay.dir = vec3(1.0f, 2.0f, -3.0f);
			if (!inShadow)
			{
				float lambert = (lightRay.dir * vNormal) * coef;
				output.red += lambert * currentLight.intensity.red * currentMat.diffuse.red;
				output.green += lambert * currentLight.intensity.green * currentMat.diffuse.green;
				output.blue += lambert * currentLight.intensity.blue * currentMat.diffuse.blue;

				// Blinn 
				// The direction of Blinn is exactly at mid point of the light ray 
				// and the view ray. 
				// We compute the Blinn vector and then we normalize it
				// then we compute the coeficient of blinn
				// which is the specular contribution of the current light.

				float fViewProjection = viewRay.dir * vNormal;
				vec3 blinnDir = lightRay.dir - viewRay.dir;
				float temp = blinnDir * blinnDir;
				if (temp != 0.0f)
				{
					float blinn = invsqrtf(temp) * MAX(fLightProjection - fViewProjection, 0.0f);
					blinn = coef * powf(blinn, currentMat.power);
					output.red += blinn;// * currentMat.specular  * currentLight.intensity;
					output.green += blinn;
					output.blue += blinn;
				}
			}
		}
		float reflect = 2.0f * (viewRay.dir * vNormal);


		coef *= currentMat.reflection;
		viewRay.start = ptHitPoint;
		viewRay.dir = viewRay.dir - vNormal * reflect;
		level++;
	} while ((coef > 0.0f) && (level < 10));
	return output;
}

bool render_raytrace(vertex_t *vertex_array, int *index_array, int num_vert, int num_index, int width, int height, unsigned int *pixel, light_t *light, int num_light, matrix4 &mvp)
{
#pragma omp parallel num_threads(64)
	{
#pragma omp for
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				color_t output = { 0.0f, 0.0f, 0.0f };

				float fragmenty = (float)y;
				float fragmentx = (float)x;

				// Antialiasing loops
//				for (float fragmenty = y ; fragmenty < y + 1.0f; fragmenty += 0.5f )
//				for (float fragmentx = x ; fragmentx < x + 1.0f; fragmentx += 0.5f )
				{
					float sampleRatio = 1.0f;

					raytrace::ray_t viewRay = { { fragmentx, fragmenty, 1000.0f },
					{ 0.0f, 0.0f, -1.0f } };
					color_t temp = addRay(viewRay, vertex_array, index_array, num_vert, num_index, light, num_light, width, height, mvp);

					// pseudo photo exposure
					float exposure = -1.00f; // random exposure value. TODO : determine a good value automatically
					temp.blue = (1.0f - expf(temp.blue * exposure));
					temp.red = (1.0f - expf(temp.red * exposure));
					temp.green = (1.0f - expf(temp.green * exposure));

					output += temp * sampleRatio;
				}

				// gamma correction
				output.blue = srgbEncode(output.blue);
				output.red = srgbEncode(output.red);
				output.green = srgbEncode(output.green);

				pixel[y * width + x] = RGB(MIN(output.blue*255.0f, 255.0f), MIN(output.green*255.0f, 255.0f), MIN(output.red*255.0f, 255.0f));
			}
		}
	}
	return true;
}


