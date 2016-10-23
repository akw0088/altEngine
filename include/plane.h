#ifndef PLANE_H
#define PLANE_H

class Entity;
class Global;

class Plane
{
public:
	Plane();
	Plane(vec4 vector);
	Plane(vec3 normal, float d);
	void draw_plane(Global &global, matrix4 &transformation, matrix4 &projection);
	void get_frustum(matrix4 &projection, Plane *frustum);

	vec3	normal;
	float	d;
};

#endif



