#ifndef PLANE_H
#define PLANE_H

class Entity;

class Plane
{
public:
	Plane();
	Plane(vec4 vector);
	Plane(vec3 normal, float d);
	void draw_plane();

	vec3	normal;
	float	d;
};

#endif

/*
unsigned getTimeStamp(void)
{
	unsigned timestamp = 0;

	_asm
	{
		// rdtsc returns 64bit "timestamp" in edx:eax, timestamp is really a count of clock cycles
		rdtsc
		// we are only interested in small time intervals, so highword is worthless,
		mov	DWORD PTR timestamp, eax
	}
	return timestamp;
}
*/

