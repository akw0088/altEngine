#include "triangulate.h"


#ifdef WIN32

extern int debug_voronoi;
int Triangulate::draw_mode;

char Triangulate::draw_names[10][80] = {
	"[Triangle and points]",
	"[circles and current triangle]",
	"[bad triangles]",
	"[polygon edges and shared]",
	"[polygon and remaining triangles]",
	"[after delete]",
	"[final]",
	"[voronoi]",
	"eight",
	"nine",
};


#define COLOR_TABLE_SIZE (12)
COLORREF color_table[COLOR_TABLE_SIZE] = {
	RGB(0,0,255),
	RGB(0,255,0),
	RGB(0,255,255),
	RGB(255,0,0),
	RGB(255,0,255),
	RGB(255,255,0),
	RGB(0,0,128),
	RGB(0,128,0),
	RGB(0,128,128),
	RGB(128,0,0),
	RGB(128,0,128),
	RGB(128,128,0)
};

HBRUSH brush_table[COLOR_TABLE_SIZE] = { 0 };
HPEN pen_table[COLOR_TABLE_SIZE] = { 0 };

#endif
int Triangulate::debug_point;

int Triangulate::add_point_in_polygon(const vec3 &point, vec3 *poly, unsigned int &num_poly, vec3 *tri, unsigned int &num_triangle)
{
	unsigned int i = 0;

	// for each edge in polygon, make a triangle using it and our point
	for (i = 0; i + 1 < num_poly; i += 2)
	{
		vec3 a = poly[i];
		vec3 b = poly[i + 1];

		if (num_triangle + 3 < MAX_TRIANGLE)
		{
			tri[num_triangle + 0] = a;
			tri[num_triangle + 1] = b;
			tri[num_triangle + 2] = point;
			num_triangle += 3;
		}
	}
	return 0;
}

void Triangulate::get_circum_circle(const vec3 &a, const vec3 &b, const vec3 &c, float &radius, vec3 &center)
{
	vec3 ac = c - a;
	vec3 ab = b - a;
	vec3 v = vec3::crossproduct(ab, ac);

	// get circle containing all three triangles points
	// center of this circle will be a voronoi vertex
	vec3 a_to_center = (
		vec3::crossproduct(v, ab) * ac.magnitudeSq() +
		vec3::crossproduct(ac, v) * ab.magnitudeSq()
		) /
		(2.0f * v.magnitudeSq());

	radius = a_to_center.magnitude();

	center = a + a_to_center;

}

bool Triangulate::point_in_sphere(const vec3 &point, vec3 &origin, float radius)
{
	vec3 dist = point - origin;

	float mag = dist.magnitude();

	if (mag < radius)
	{
		return true;
	}

	return false;
}

bool Triangulate::point_in_triangle(const vec3 &p, const vec3 &tri_a, const vec3 &tri_b, const vec3 &tri_c)
{
	vec3 a = tri_a - p;
	vec3 b = tri_b - p;
	vec3 c = tri_c - p;

	vec3 normal_bc = vec3::crossproduct(b, c);   
	vec3 normal_ca = vec3::crossproduct(c, a);   
	vec3 normal_ab = vec3::crossproduct(a, b);

	// check sign of the dot product of normals
	// should all be positive if inside
	if (normal_bc * normal_ca < 0.0f)
	{
		return false;
	}
	else if (normal_bc * normal_ab < 0.0f)
	{
		return false;
	}

	return true;
}



bool Triangulate::point_is_same(const vec3 &a, const vec3 &b)
{
	if ((a - b).magnitude() < EPSILON)
	{
		return true;
	}

	return false;
}


bool Triangulate::add_poly(const vec3 &na, const vec3 &nb, vec3 *polygon, unsigned int &num_poly)
{
	for (unsigned int i = 0; i < num_poly; i += 2)
	{
		vec3 a = polygon[i + 0];
		vec3 b = polygon[i + 1];

		if (
				(	point_is_same(a, na) && point_is_same(b, nb) ) ||
				(	point_is_same(b, na) && point_is_same(a, nb) )
			)
		{
			// ignore duplicate edges as polygon is bounded by perimeter
			// not the internal diagnols
#ifdef DEBUG
			printf("edge %d %d already exists\r\n", i, i + 1);
#endif
			return false;
		}
	}

	polygon[num_poly++] = na;
	polygon[num_poly++] = nb;
	return true;
}


// We have bad triangles that form a polygon hole, but they contain interior edges
// We want to delete edges in bad triangles
// if we have one triangle, we really dont delete anything, entire triangle is the polygon
// but if we have two adjacent triangles that share an edge
// we delete the shared edge as it is not part of the polygon
// So essentially we are looking for edges that are in bad triangles twice to find shared edges
void Triangulate::compare_edges(
	const vec3 &T1_a, const vec3 &T1_b, const vec3 &T1_c,
	const vec3 &T2_a, const vec3 &T2_b, const vec3 &T2_c,
	vec3 *polygon, unsigned int &num_poly,
	vec3 *shared, unsigned int &num_shared)
{
	edge_list_t edge = { 0 };

	int num_edge = 0;


	if (num_shared + 2 > MAX_POLY)
	{
#ifdef DEBUG
		printf("Reached max shared\r\n");
#endif
		return;
	}

	if (num_poly + 2 >= MAX_TRIANGLE)
	{
#ifdef DEBUG
		printf("Reached max poly\r\n");
#endif
		return;
	}


#ifdef DEBUG
	printf("\r\n\r\ncompare edges()\r\n");
#endif
	// rotate 1 abc to abc
	if (
			(	point_is_same(T1_a, T2_a) && point_is_same(T1_b, T2_b) ) ||
			(	point_is_same(T1_a, T2_b) && point_is_same(T1_b, T2_a) )
		)
	{
#ifdef DEBUG
		printf("T1 ab equivalent to T2 ab\r\n");
#endif
		edge.ab_ab = 1;
		add_poly(T1_a, T1_b, shared, num_shared);
	}

	if (
			(	point_is_same(T1_c, T2_c) && point_is_same(T1_a, T2_a) ) ||
			(	point_is_same(T1_c, T2_a) && point_is_same(T1_a, T2_c) )
		)
	{
#ifdef DEBUG
		printf("T1 ac equivalent to T2 ac\r\n");
#endif
		edge.ac_ac = 1;
		add_poly(T1_a, T1_c, shared, num_shared);
	}

	if (
			(	point_is_same(T1_b, T2_b) && point_is_same(T1_c, T2_c) ) ||
			(	point_is_same(T1_b, T2_c) && point_is_same(T1_c, T2_b) )
		)
	{
#ifdef DEBUG
		printf("T1 bc equivalent to T2 bc\r\n");
#endif
		edge.bc_bc = 1;
		add_poly(T1_b, T1_c, shared, num_shared);

	}

	// rotate left -- ba ca bc -> ca bc ba  
	if (
			(	point_is_same(T1_a, T2_c) && point_is_same(T1_b, T2_a) ) ||
			(	point_is_same(T1_a, T2_a) && point_is_same(T1_b, T2_c) )
		)
	{
#ifdef DEBUG
		printf("T1 ab equivalent to T2 ac\r\n");
#endif
		edge.ab_ac = 1;
		add_poly(T1_a, T1_b, shared, num_shared);

	}

	if (
			(	point_is_same(T1_c, T2_b) && point_is_same(T1_a, T2_c) ) ||
			(	point_is_same(T1_c, T2_c) && point_is_same(T1_a, T2_b) )
		)
	{
#ifdef DEBUG
		printf("T1 ac equivalent to T2 bc\r\n");
#endif
		edge.ac_bc = 1;
		add_poly(T1_a, T1_c, shared, num_shared);

	}

	if (
			(	point_is_same(T1_b, T2_b) && point_is_same(T1_c, T2_a) ) ||
			(	point_is_same(T1_b, T2_a) && point_is_same(T1_c, T2_b) )
		)
	{
#ifdef DEBUG
		printf("T1 bc equivalent to T2 ba\r\n");
#endif
		edge.bc_ab = 1;
		add_poly(T1_b, T1_c, shared, num_shared);

	}

	// rotate left -- ba ca bc -> bc ba ca  
	if (
			(	point_is_same(T1_a, T2_b) && point_is_same(T1_b, T2_c) ) ||
			(	point_is_same(T1_a, T2_c) && point_is_same(T1_b, T2_b) )
		)
	{
#ifdef DEBUG
		printf("T1 ab equivalent to T2 bc\r\n");
#endif
		edge.ab_bc = 1;
		add_poly(T1_a, T1_b, shared, num_shared);

	}

	if (
			(	point_is_same(T1_c, T2_b) && point_is_same(T1_a, T2_a) ) ||
			(	point_is_same(T1_c, T2_a) && point_is_same(T1_a, T2_b) )
		)
	{
#ifdef DEBUG
		printf("T1 ac equivalent to T2 ba\r\n");
#endif
		edge.ac_ab = 1;

		add_poly(T1_a, T1_c, shared, num_shared);
	}

	if (
			(	point_is_same(T1_b, T2_c) && point_is_same(T1_c, T2_a) ) ||
			(	point_is_same(T1_b, T2_a) && point_is_same(T1_c, T2_c) )
		)
	{
#ifdef DEBUG
		printf("T1 bc equivalent to T2 ca\r\n");
#endif
		edge.bc_ac = 1;

		add_poly(T1_b, T1_c, shared, num_shared);

	}

	// Now we know the shared edges, so add everything that is not shared


	// Triangle one
	if (edge.ab_ab == 0 && edge.ab_ac == 0 && edge.ab_bc == 0)
	{
		vec3 na = T1_a;
		vec3 nb = T1_b;

		// ab is good to add, check for duplicates
		if (add_poly(na, nb, polygon, num_poly))
		{
			num_edge++;
		}
	}

	if (edge.ac_ac == 0 && edge.ac_bc == 0 && edge.ac_ab == 0)
	{
		vec3 na = T1_a;
		vec3 nb = T1_c;

		// ab is good to add, check for duplicates
		if (add_poly(na, nb, polygon, num_poly))
		{
			num_edge++;
		}
	}

	if (edge.bc_bc == 0 && edge.bc_ab == 0 && edge.bc_ac == 0)
	{
		// bc is good to add
		vec3 na = T1_b;
		vec3 nb = T1_c;

		if (add_poly(na, nb, polygon, num_poly))
		{
			num_edge++;
		}

	}

	// Triangle two
	if (edge.ab_ab == 0 && edge.ac_ab == 0 && edge.bc_ab == 0)
	{
		// ab is good to add
		vec3 na = T2_a;
		vec3 nb = T2_b;


		if (add_poly(na, nb, polygon, num_poly))
		{
			num_edge++;
		}

	}

	if (edge.ac_ac == 0 && edge.ab_ac == 0 && edge.bc_ac == 0)
	{
		// ac is good to add
		vec3 na = T2_a;
		vec3 nb = T2_c;

		if (add_poly(na, nb, polygon, num_poly))
		{
			num_edge++;
		}
	}

	if (edge.bc_bc == 0 && edge.ab_bc == 0 && edge.ac_bc == 0)
	{
		// bc is good to add
		vec3 na = T2_b;
		vec3 nb = T2_c;

		if (add_poly(na, nb, polygon, num_poly))
		{
			num_edge++;
		}
	}


#ifdef DEBUG
	printf("\tFound %d unique edges\r\n", num_edge);
#endif
}


void Triangulate::delete_triangle(const vec3 &a, const vec3 &b, const vec3 &c, vec3 *triangle, unsigned int &num_triangle)
{
	for (unsigned int k = 0; k < num_triangle; k += 3)
	{
		/*
			Delete triangle even if it's ordered differently

			Ways to organized three points ABC and still have the same triangle
			{a,b,c} {a,c,b} {b,a,c} {b,c,a} {c,a,b} {c,b,a}
		*/

		// abc abc
		vec3 v1 = a - triangle[k + 0];
		vec3 v2 = b - triangle[k + 1];
		vec3 v3 = c - triangle[k + 2];

		//bca abc
		vec3 v4 = b - triangle[k + 0];
		vec3 v5 = c - triangle[k + 1];
		vec3 v6 = a - triangle[k + 2];

		//cab abc
		vec3 v7 = c - triangle[k + 0];
		vec3 v8 = a - triangle[k + 1];
		vec3 v9 = b - triangle[k + 2];

		//cba abc
		vec3 v10 = c - triangle[k + 0];
		vec3 v11 = b - triangle[k + 1];
		vec3 v12 = a - triangle[k + 2];

		//acb abc
		vec3 v13 = a - triangle[k + 0];
		vec3 v14 = c - triangle[k + 1];
		vec3 v15 = b - triangle[k + 2];

		//bac abc
		vec3 v16 = b - triangle[k + 0];
		vec3 v17 = a - triangle[k + 1];
		vec3 v18 = c - triangle[k + 2];


		if (
			(	v1.magnitude() < EPSILON &&
				v2.magnitude() < EPSILON &&
				v3.magnitude() < EPSILON)
			||
			(	v4.magnitude() < EPSILON &&
				v5.magnitude() < EPSILON &&
				v6.magnitude() < EPSILON)
			||
			(	v7.magnitude() < EPSILON &&
				v8.magnitude() < EPSILON &&
				v9.magnitude() < EPSILON)
			||
			(	v10.magnitude() < EPSILON &&
				v11.magnitude() < EPSILON &&
				v12.magnitude() < EPSILON)
			||
			(	v13.magnitude() < EPSILON &&
				v14.magnitude() < EPSILON &&
				v15.magnitude() < EPSILON)
			||
			(	v16.magnitude() < EPSILON &&
				v17.magnitude() < EPSILON &&
				v18.magnitude() < EPSILON))

		{
			//remove triangle from triangulation
			triangle[k + 0] = triangle[num_triangle - 3];
			triangle[k + 1] = triangle[num_triangle - 2];
			triangle[k + 2] = triangle[num_triangle - 1];
			num_triangle -= 3;
			k -= 3;
		}
	}
}


void Triangulate::delete_triangle_with_edge(const vec3 &a, const vec3 &b, vec3 *triangle, unsigned int &num_triangle)
{
	for (unsigned int k = 0; k < num_triangle; k += 3)
	{
		/*
		{a,b,c} {a,c,b} {b,a,c} {b,c,a} {c,a,b} {c,b,a}
		*/

		// abc abc
		vec3 v1 = a - triangle[k + 0];
		vec3 v2 = b - triangle[k + 1];

		//bca abc
		vec3 v4 = b - triangle[k + 0];
		vec3 v6 = a - triangle[k + 2];

		//cab abc
		vec3 v8 = a - triangle[k + 1];
		vec3 v9 = b - triangle[k + 2];

		//cba abc
		vec3 v11 = b - triangle[k + 1];
		vec3 v12 = a - triangle[k + 2];

		//acb abc
		vec3 v13 = a - triangle[k + 0];
		vec3 v15 = b - triangle[k + 2];

		//bac abc
		vec3 v16 = b - triangle[k + 0];
		vec3 v17 = a - triangle[k + 1];


		if (
			(	v1.magnitude() < EPSILON &&
				v2.magnitude() < EPSILON)
			||
			(	v4.magnitude() < EPSILON &&
				v6.magnitude() < EPSILON)
			||
			(	v8.magnitude() < EPSILON &&
				v9.magnitude() < EPSILON)
			||
			(	v11.magnitude() < EPSILON &&
				v12.magnitude() < EPSILON)
			||
			(	v13.magnitude() < EPSILON &&
				v15.magnitude() < EPSILON)
			||
			(	v16.magnitude() < EPSILON &&
				v17.magnitude() < EPSILON)
			)

		{
			//remove triangle from triangulation
			triangle[k + 0] = triangle[num_triangle - 3];
			triangle[k + 1] = triangle[num_triangle - 2];
			triangle[k + 2] = triangle[num_triangle - 1];
			num_triangle -= 3;
			k -= 3;
		}
	}
}


void Triangulate::delete_triangle_with_vertex(const vec3 &a, vec3 *triangle, unsigned int &num_triangle)
{
	for (unsigned int k = 0; k < num_triangle; k += 3)
	{
		/*
		{a,b,c} {a,c,b} {b,a,c} {b,c,a} {c,a,b} {c,b,a}
		*/

		// abc abc
		vec3 v1 = a - triangle[k + 0];
		vec3 v2 = a - triangle[k + 1];
		vec3 v3 = a - triangle[k + 2];

		if (
			    v1.magnitude() < EPSILON ||
				v2.magnitude() < EPSILON ||
				v3.magnitude() < EPSILON
			)
		{
			//remove triangle from triangulation
			triangle[k + 0] = triangle[num_triangle - 3];
			triangle[k + 1] = triangle[num_triangle - 2];
			triangle[k + 2] = triangle[num_triangle - 1];
			num_triangle -= 3;
			k -= 3;
		}
	}
}

//
//	Good background video:
//		https://www.youtube.com/watch?v=-XCVn73p3xs
//
void Triangulate::BowyerWatson(const vec3 *point, unsigned int num_point, vec3 *triangle, unsigned int &num_triangle)
{
	vec3 super_tri[3];

	super_tri[0] = point[0];
	super_tri[1] = point[1];
	super_tri[2] = point[2];
	unsigned int num_super_tri = 3;

	vec3 mid = (super_tri[0] + super_tri[1] + super_tri[2]) / 3.0;

	unsigned int size = 1;
	int done = 0;

	unsigned int attempts = 0;

	if (num_point >= 3)
	{
		// make a triangle just big enough
		while (!done)
		{
			int all_in = 1;

			// extend triangle to infinite plane by moving away from midpoint
			triangle[0] = super_tri[0] + ((super_tri[0] - mid) * (float)size);
			triangle[1] = super_tri[1] + ((super_tri[1] - mid) * (float)size);
			triangle[2] = super_tri[2] + ((super_tri[2] - mid) * (float)size);

			for (unsigned int i = 0; i < num_point; i++)
			{
				if (point_in_triangle(point[i], triangle[0], triangle[1], triangle[2]) == false)
				{
					size += 100;
					all_in = 0;
					attempts++;
					break;
				}
			}

			if (all_in == 1 || attempts > 2000)
			{
				done = 1;
			}
		}
		super_tri[0] = triangle[0];
		super_tri[1] = triangle[1];
		super_tri[2] = triangle[2];
	}
	else
	{
		size = 20000;
		triangle[0] = super_tri[0] + ((super_tri[0] - mid) * (float)size);
		triangle[1] = super_tri[1] + ((super_tri[1] - mid) * (float)size);
		triangle[2] = super_tri[2] + ((super_tri[2] - mid) * (float)size);
	}


	num_triangle = 3;

	// triangle is now a valid, add a point and re-triangulate

	// for each point in pointList
	// add all the points one at a time to the triangulation
	for (unsigned int i = 0; i < num_point; i++)
	{

		static vec3 polygon[MAX_POLY];
		unsigned int num_poly = 0;

		vec3 badTriangles[MAX_BAD];
		unsigned int num_bad = 0;

		vec3 shared[MAX_POLY];
		unsigned int num_shared = 0;


//		printf("\tinput %d: %f %f %f\r\n", i, point[i].x, point[i].y, point[i].z);


		// first find all the triangles that are no longer valid due to the insertion
		for (unsigned int j = 0; j < num_triangle; j += 3)
		{
			vec3 a = triangle[j + 0];
			vec3 b = triangle[j + 1];
			vec3 c = triangle[j + 2];

			float radius;
			vec3 center;

			get_circum_circle(a, b, c, radius, center);


			// If a point is in the sphere, we broke the Delaunay triangle property
			if (point_in_sphere(point[i], center, radius))
			{
				// add triangle to badTriangles as it has a point in it's circle
				badTriangles[num_bad++] = a;
				badTriangles[num_bad++] = b;
				badTriangles[num_bad++] = c;
			}
		}

		// Now we want to find the boundary of the hole created by the bad Triangles
		for (unsigned int j = 0; j < num_bad; j += 3)
		{
			vec3 badA_a = badTriangles[j + 0];
			vec3 badA_b = badTriangles[j + 1];
			vec3 badA_c = badTriangles[j + 2];

			for (unsigned int k = 0; k < num_bad; k += 3)
			{
				if (k == j)
					continue;

				vec3 badB_a = badTriangles[k + 0];
				vec3 badB_b = badTriangles[k + 1];
				vec3 badB_c = badTriangles[k + 2];

#ifdef DEBUG
				printf("Comparing %d %d %d triangles to %d %d %d [num_bad = %d]\r\n", j, j + 1, j + 2, k, k + 1, k + 2, num_bad);
#endif
				// we compare each edge of the bad triangles and keep any edges shared by two bad triangles
				compare_edges(badA_a, badA_b, badA_c, badB_a, badB_b, badB_c, &polygon[0], num_poly, shared, num_shared);
			}
		}

		// If we have no polygon, as in the first step, we keep the super triangle
		if (num_poly == 0)
		{
			for (unsigned int j = 0; j < num_bad; j += 3)
			{
#ifdef DEBUG
				if (num_shared > 0)
				{
					printf("Had no polys, but some shared polys?\r\n");
				}
#endif

				// if we have no polys, then add the bad triangles
				polygon[num_poly++] = badTriangles[0];
				polygon[num_poly++] = badTriangles[1];
				polygon[num_poly++] = badTriangles[1];
				polygon[num_poly++] = badTriangles[2];
				polygon[num_poly++] = badTriangles[2];
				polygon[num_poly++] = badTriangles[0];
			}
		}



		// Delete the super triangle
		vec3 a = super_tri[0];
		vec3 b = super_tri[1];
		vec3 c = super_tri[2];

		delete_triangle(a, b, c, triangle, num_triangle);






		// polygon defines edges of the hole
		// so we add new point to each edge of polygon

		// delete any triangles with shared edges
		for (unsigned int j = 0; j < num_shared; j += 2)
		{
			vec3 a = shared[j + 0];
			vec3 b = shared[j + 1];

			delete_triangle_with_edge(a, b, triangle, num_triangle);
		}



		// re-triangulate the polygonal hole
		add_point_in_polygon(point[i], polygon, num_poly, triangle, num_triangle);


		// once we added the point, the polygon hole boundary isnt needed, so set it to zero
		num_poly = 0;


		// Delete the bad triangles, as we should have fixed them with the added point
		for (unsigned int j = 0; j < num_bad; j += 3)
		{
			vec3 a = badTriangles[j + 0];
			vec3 b = badTriangles[j + 1];
			vec3 c = badTriangles[j + 2];

			delete_triangle(a, b, c, triangle, num_triangle);
		}

		// Delete the shared edges, they should already be gone, but just in case
		for (unsigned int j = 0; j < num_shared; j += 2)
		{
			vec3 a = shared[j + 0];
			vec3 b = shared[j + 1];

			delete_triangle_with_edge(a, b, triangle, num_triangle);
		}

	}

	// for each vertex in super triangle
	for (unsigned int j = 0; j < num_super_tri; j += 3)
	{
		vec3 a = super_tri[j + 0];
		vec3 b = super_tri[j + 1];
		vec3 c = super_tri[j + 2];

		delete_triangle_with_vertex(a, triangle, num_triangle);
		delete_triangle_with_vertex(b, triangle, num_triangle);
		delete_triangle_with_vertex(c, triangle, num_triangle);
	}


	// final output
	for (unsigned int j = 0; j < num_triangle; j += 3)
	{
		vec3 a = triangle[j + 0];
		vec3 b = triangle[j + 1];
		vec3 c = triangle[j + 2];

//		printf("\toutput %d: %f %f %f\r\n", j + 0, a.x, a.y, a.z);
//		printf("\toutput %d: %f %f %f\r\n", j + 1, b.x, b.y, b.z);
//		printf("\toutput %d: %f %f %f\r\n", j + 2, c.x, c.y, c.z);
	}
}



#ifdef WIN32
void Triangulate::draw_point(HDC hdc, const vec3 &point, float size, float scale, POINT offset)
{
	Rectangle(hdc,
		(int)((point.x * scale - size * scale) + offset.x),
		(int)((point.y * scale - size * scale) + offset.y),
		(int)((point.x * scale + size * scale) + offset.x),
		(int)((point.y * scale + size * scale) + offset.y));
}

void Triangulate::draw_fill(HDC hdc, const vec3 &point, COLORREF color, float scale, POINT offset)
{
	int ret0 = ExtFloodFill(hdc,
		(int)((point.x * scale) + offset.x),
		(int)((point.y * scale) + offset.y),
		color,
		FLOODFILLBORDER);
}


void Triangulate::draw_triangle(HDC hdc, const vec3 &a, const vec3 &b, const vec3 &c, float scale, POINT offset)
{
	MoveToEx(hdc,
		(int)(a.x * scale + offset.x),
		(int)(a.y * scale + offset.y),
		NULL);

	LineTo(hdc,
		(int)(b.x * scale + offset.x),
		(int)(b.y * scale + offset.y));
	LineTo(hdc,
		(int)(c.x * scale + offset.x),
		(int)(c.y * scale + offset.y));
	LineTo(hdc,
		(int)(a.x * scale + offset.x),
		(int)(a.y * scale + offset.y));
}


void Triangulate::draw_line(HDC hdc, const vec3 &a, const vec3 &b, float scale, POINT offset)
{
	MoveToEx(hdc,
		(int)(a.x * scale + offset.x),
		(int)(a.y * scale + offset.y),
		NULL);
	LineTo(hdc,
		(int)(b.x * scale + offset.x),
		(int)(b.y * scale + offset.y));
}


void Triangulate::draw_circle(HDC hdc, const vec3 &c, float radius, float scale, POINT offset)
{
	int left, right, top, bottom;

	left   = (int)((c.x - radius) * scale + offset.x);
	right  = (int)((c.x + radius) * scale + offset.x);
	top    = (int)((c.y - radius) * scale + offset.y);
	bottom = (int)((c.y + radius)  * scale + offset.y);
	Ellipse(hdc, left, top, right, bottom);
}

void Triangulate::debug_BowyerWatson(HDC hdc, const vec3 *point, unsigned int num_point, vec3 *triangle, unsigned int &num_triangle, float scale, POINT offset)
{
	vec3 super_tri[3];
	vec3 voronoi_edge[256][32];
	unsigned int num_edge[256] = { 0 };
	vec3 center_array[4096];
	unsigned int num_center = 0;

	super_tri[0] = point[0];
	super_tri[1] = point[1];
	super_tri[2] = point[2];
	unsigned int num_super_tri = 3;

	vec3 mid = (super_tri[0] + super_tri[1] + super_tri[2]) / 3.0;

	int size = 1;
	int done = 0;


	if (num_point >= 3)
	{
		// make a triangle just big enough
		while (!done)
		{
			int all_in = 1;

			// extend triangle to infinite plane by moving away from midpoint
			triangle[0] = super_tri[0] + ((super_tri[0] - mid) * (float)size);
			triangle[1] = super_tri[1] + ((super_tri[1] - mid) * (float)size);
			triangle[2] = super_tri[2] + ((super_tri[2] - mid) * (float)size);

			for (unsigned int i = 0; i < num_point; i++)
			{
				if (point_in_triangle(point[i], triangle[0], triangle[1], triangle[2]) == false)
				{
					size += 5;
					all_in = 0;
					break;
				}
			}

			if (all_in == 1)
			{
				done = 1;
			}
		}
		super_tri[0] = triangle[0];
		super_tri[1] = triangle[1];
		super_tri[2] = triangle[2];
	}
	else
	{
		size = 2000;
		triangle[0] = super_tri[0] + ((super_tri[0] - mid) * (float)size);
		triangle[1] = super_tri[1] + ((super_tri[1] - mid) * (float)size);
		triangle[2] = super_tri[2] + ((super_tri[2] - mid) * (float)size);
	}

	if (draw_mode == TRIANGULATE_VORONOI)
	{
		size = 200;
		triangle[0] = super_tri[0] + ((super_tri[0] - mid) * (float)size);
		triangle[1] = super_tri[1] + ((super_tri[1] - mid) * (float)size);
		triangle[2] = super_tri[2] + ((super_tri[2] - mid) * (float)size);
	}


	num_triangle = 3;

	if (draw_mode == TRIANGULATE_TRI_POINT || draw_mode == TRIANGULATE_TRI_BAD)
	{
		draw_triangle(hdc, super_tri[0], super_tri[1], super_tri[2], scale, offset);
	}

	// triangle is now a valid, add a point and re-triangulate

	// for each point in pointList
	// add all the points one at a time to the triangulation
	for (unsigned int i = 0; i < num_point; i++)
	{
		if ((int)i > debug_point)
			break;

		static vec3 polygon[MAX_POLY];
		unsigned int num_poly = 0;

		vec3 badTriangles[MAX_BAD];
		unsigned int num_bad = 0;

		vec3 shared[MAX_POLY];
		unsigned int num_shared = 0;



		static HPEN hPen;

		if (hPen == 0)
			hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));


		// draw selected point really big
		if (draw_mode < TRIANGULATE_VORONOI2)
		{
			SelectObject(hdc, hPen);
			draw_point(hdc, point[i], 50, scale, offset);
			SelectObject(hdc, GetStockObject(BLACK_PEN));
		}


		// first find all the triangles that are no longer valid due to the insertion
		for (unsigned int j = 0; j < num_triangle; j += 3)
		{
			vec3 a = triangle[j + 0];
			vec3 b = triangle[j + 1];
			vec3 c = triangle[j + 2];

			float radius;
			vec3 center;

			if (draw_mode == TRIANGULATE_CIR_POINT && debug_point == i)
			{
				SelectObject(hdc, GetStockObject(BLACK_PEN));
				draw_triangle(hdc, a, b, c, scale, offset);
			}

			get_circum_circle(a, b, c, radius, center);

			// If a point is in the sphere, we broke the Delaunay triangle property
			if (point_in_sphere(point[i], center, radius))
			{
				static HPEN hPen;

				if (hPen == 0)
					hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 255));

				static HPEN hPen2;

				if (hPen2 == 0)
					hPen2 = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));


				if (draw_mode == TRIANGULATE_CIR_POINT && debug_point == i)
				{
					SelectObject(hdc, hPen);
					SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
					draw_circle(hdc, center, radius, scale, offset);

					SelectObject(hdc, hPen2);
					draw_triangle(hdc, a, b, c, scale, offset);

					SelectObject(hdc, GetStockObject(BLACK_PEN));
				}



				// add triangle to badTriangles as it has a point in it's circle
				badTriangles[num_bad++] = a;
				badTriangles[num_bad++] = b;
				badTriangles[num_bad++] = c;
			}
			else
			{
				if (draw_mode == TRIANGULATE_CIR_POINT && debug_point == i)
				{
					SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
					draw_circle(hdc, center, radius, scale, offset);
				}
				if (draw_mode == TRIANGULATE_VORONOI2)
				{
					// draw circumcircle vertex
					SelectObject(hdc, GetStockObject(BLACK_PEN));
					SelectObject(hdc, GetStockObject(BLACK_BRUSH));
					draw_point(hdc, center, 15, scale, offset);
					SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));

				}


			}
		}


		// for each bad triangle draw it
		for (unsigned int j = 0; j < num_bad; j += 3)
		{
			vec3 a = badTriangles[j + 0];
			vec3 b = badTriangles[j + 1];
			vec3 c = badTriangles[j + 2];


			if (draw_mode == TRIANGULATE_TRI_BAD && debug_point == i)
			{
				static HPEN hPen;

				if (hPen == 0)
					hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));

				SelectObject(hdc, hPen);
				draw_triangle(hdc, a, b, c, scale, offset);
				SelectObject(hdc, GetStockObject(BLACK_PEN));
			}
		}


		// Now we want to find the boundary of the hole created by the bad Triangles
		for (unsigned int j = 0; j < num_bad; j += 3)
		{
			vec3 badA_a = badTriangles[j + 0];
			vec3 badA_b = badTriangles[j + 1];
			vec3 badA_c = badTriangles[j + 2];

			for (unsigned int k = 0; k < num_bad; k += 3)
			{
				if (k == j)
					continue;

				vec3 badB_a = badTriangles[k + 0];
				vec3 badB_b = badTriangles[k + 1];
				vec3 badB_c = badTriangles[k + 2];

				printf("Comparing %d %d %d triangles to %d %d %d [num_bad = %d]\r\n", j, j + 1, j + 2, k, k + 1, k + 2, num_bad);
				// we compare each edge of the bad triangles and keep any edges shared by two bad triangles
				compare_edges(badA_a, badA_b, badA_c, badB_a, badB_b, badB_c, &polygon[0], num_poly, shared, num_shared);
			}
		}

		// If we have no polygon, as in the first step, we keep the super triangle
		if (num_poly == 0)
		{
			for (unsigned int j = 0; j < num_bad; j += 3)
			{
				if (num_shared > 0)
				{
					printf("Had no polys, but some shared polys?\r\n");
				}

				// if we have no polys, then add the bad triangles
				polygon[num_poly++] = badTriangles[0];
				polygon[num_poly++] = badTriangles[1];
				polygon[num_poly++] = badTriangles[1];
				polygon[num_poly++] = badTriangles[2];
				polygon[num_poly++] = badTriangles[2];
				polygon[num_poly++] = badTriangles[0];
			}
		}



		// Delete the super triangle
		vec3 a = super_tri[0];
		vec3 b = super_tri[1];
		vec3 c = super_tri[2];

		delete_triangle(a, b, c, triangle, num_triangle);



		// Draw the triangles after the delete
		for (unsigned int j = 0; j < num_triangle; j += 3)
		{
			vec3 a = triangle[j + 0];
			vec3 b = triangle[j + 1];
			vec3 c = triangle[j + 2];


			if (draw_mode == TRIANGULATE_POLY_ALL && debug_point == i)
			{
				static HPEN hPen;

				if (hPen == 0)
					hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));

				SelectObject(hdc, hPen);
				draw_triangle(hdc, a, b, c, scale, offset);
				SelectObject(hdc, GetStockObject(BLACK_PEN));

			}
		}



		// draw the polgonal hole
		for (unsigned int j = 0; j < num_poly; j += 2)
		{
			vec3 a = polygon[j + 0];
			vec3 b = polygon[j + 1];


			if ((draw_mode == TRIANGULATE_POLY_SHARED || draw_mode == TRIANGULATE_POLY_ALL) && debug_point == i)
			{
				static HPEN hPen;

				if (hPen == 0)
					hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));

				SelectObject(hdc, hPen);
				draw_line(hdc, a, b, scale, offset);
			}
		}


		// draw the shared edges between two bad triangles
		for (unsigned int j = 0; j < num_shared; j += 2)
		{
			vec3 a = shared[j + 0];
			vec3 b = shared[j + 1];


			if ((draw_mode == TRIANGULATE_POLY_SHARED || draw_mode == TRIANGULATE_POLY_ALL) && debug_point == i)
			{
				static HPEN hPen;

				if (hPen == 0)
					hPen = CreatePen(PS_DASH, 1, RGB(0, 255, 0));

				SelectObject(hdc, hPen);
				draw_line(hdc, a, b, scale, offset);
			}
		}

		// polygon defines edges of the hole
		// so we add new point to each edge of polygon

		// delete any triangles with shared edges
		for (unsigned int j = 0; j < num_shared; j += 2)
		{
			vec3 a = shared[j + 0];
			vec3 b = shared[j + 1];

			delete_triangle_with_edge(a, b, triangle, num_triangle);
		}



		// re-triangulate the polygonal hole
		add_point_in_polygon(point[i], polygon, num_poly, triangle, num_triangle);


		// once we added the point, the polygon hole boundary isnt needed, so set it to zero
		num_poly = 0;


		// Delete the bad triangles, as we should have fixed them with the added point
		for (unsigned int j = 0; j < num_bad; j += 3)
		{
			vec3 a = badTriangles[j + 0];
			vec3 b = badTriangles[j + 1];
			vec3 c = badTriangles[j + 2];

			delete_triangle(a, b, c, triangle, num_triangle);
		}

		// Delete the shared edges, they should already be gone, but just in case
		for (unsigned int j = 0; j < num_shared; j += 2)
		{
			vec3 a = shared[j + 0];
			vec3 b = shared[j + 1];

			delete_triangle_with_edge(a, b, triangle, num_triangle);
		}


		// draw after delete
		for (unsigned int j = 0; j < num_triangle; j += 3)
		{
			vec3 a = triangle[j + 0];
			vec3 b = triangle[j + 1];
			vec3 c = triangle[j + 2];

			if (draw_mode == 5 && debug_point == i)
			{
				draw_triangle(hdc, a, b, c, scale, offset);
			}
		}

	}



	if (draw_mode == TRIANGULATE_VORONOI)
	{
		// draw voronoi cells by connecting circum circle centers of neighboring triangles
		for (unsigned int j = 0; j < num_triangle; j += 3)
		{
			vec3 a1 = triangle[j + 0];
			vec3 b1 = triangle[j + 1];
			vec3 c1 = triangle[j + 2];

			for (unsigned int k = 0; k < num_triangle; k += 3)
			{
				vec3 a2 = triangle[k + 0];
				vec3 b2 = triangle[k + 1];
				vec3 c2 = triangle[k + 2];



				if (j == k)
				{
					continue;
				}


				unsigned int num_shared = 0;
				vec3 shared[2];

				if (is_triangle_neighbor(a1, b1, c1, a2, b2, c2, shared, num_shared))
				{
					vec3 center1;
					float radius1;
					vec3 center2;
					float radius2;

					get_circum_circle(a1, b1, c1, radius1, center1);
					get_circum_circle(a2, b2, c2, radius2, center2);


					static HPEN hPen;

					if (hPen == 0)
						hPen = CreatePen(PS_SOLID, 5, RGB(128, 128, 128));

					SelectObject(hdc, hPen);
					draw_line(hdc, center1, center2, scale, offset);

					SelectObject(hdc, GetStockObject(BLACK_PEN));
				}
			}
		}

		// fill the voronoi cells
		for (unsigned int j = 0; j < num_triangle; j += 3)
		{
			vec3 a1 = triangle[j + 0];
			vec3 b1 = triangle[j + 1];
			vec3 c1 = triangle[j + 2];

			SelectObject(hdc, brush_table[(j + 0) % COLOR_TABLE_SIZE]);
			draw_fill(hdc, a1, RGB(128, 128, 128), scale, offset);
			SelectObject(hdc, brush_table[(j + 1) % COLOR_TABLE_SIZE]);
			draw_fill(hdc, b1, RGB(128, 128, 128), scale, offset);
			SelectObject(hdc, brush_table[(j + 2) % COLOR_TABLE_SIZE]);
			draw_fill(hdc, c1, RGB(128, 128, 128), scale, offset);
			SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));

		}

		// draw the voronoi sites
		for (unsigned int j = 0; j < num_triangle; j += 3)
		{
			vec3 a1 = triangle[j + 0];
			vec3 b1 = triangle[j + 1];
			vec3 c1 = triangle[j + 2];

			static HPEN hPen;

			if (hPen == 0)
				hPen = CreatePen(PS_SOLID, 5, RGB(0, 0, 0));

			SelectObject(hdc, hPen);

			draw_point(hdc, a1, 5, scale, offset);
			draw_point(hdc, b1, 5, scale, offset);
			draw_point(hdc, c1, 5, scale, offset);
			SelectObject(hdc, GetStockObject(BLACK_PEN));

		}

	}







	// for each vertex in super triangle
	for (unsigned int j = 0; j < num_super_tri; j += 3)
	{
		vec3 a = super_tri[j + 0];
		vec3 b = super_tri[j + 1];
		vec3 c = super_tri[j + 2];

		delete_triangle_with_vertex(a, triangle, num_triangle);
		delete_triangle_with_vertex(b, triangle, num_triangle);
		delete_triangle_with_vertex(c, triangle, num_triangle);
	}


	// Draw final output
	for (unsigned int j = 0; j < num_triangle; j += 3)
	{
		vec3 a = triangle[j + 0];
		vec3 b = triangle[j + 1];
		vec3 c = triangle[j + 2];

		if (draw_mode == TRIANGULATE_TRI_POINT || draw_mode == TRIANGULATE_CIR_POINT || draw_mode == TRIANGULATE_OUTPUT || draw_mode == TRIANGULATE_VORONOI2)
		{
			draw_triangle(hdc, a, b, c, scale, offset);
		}
	}


	for (unsigned int j = 0; j < num_triangle; j += 3)
	{
		vec3 a = triangle[j + 0];
		vec3 b = triangle[j + 1];
		vec3 c = triangle[j + 2];

		int ia = j + 0;
		int ib = j + 1;
		int ic = j + 2;

		vec3 mid_ab = (a + b) / 2.0f;
		vec3 mid_bc = (b + c) / 2.0f;
		vec3 mid_ca = (c + a) / 2.0f;

		// edges of the voronoi bisect with each point
		// so take the midpoints of each edge in the output triangle, and that will give voronoi edge
		// from each point

		float radius;
		vec3 center;
		get_circum_circle(a, b, c, radius, center);

		center_array[num_center++] = center;
		//draw_circle(hdc, center, radius, scale, offset);


		vec3 inf_ab_pos = (mid_ab - center) * 100;
		vec3 inf_bc_pos = (mid_bc - center) * 100;
		vec3 inf_ca_pos = (mid_ca - center) * 100;

		vec3 inf_ab_neg = (mid_ab - center) * -100;
		vec3 inf_bc_neg = (mid_bc - center) * -100;
		vec3 inf_ca_neg = (mid_ca - center) * -100;

		// technically we want to color each side of these lines differently, maybe a flood fill or something
		if (draw_mode == TRIANGULATE_VORONOI2)
		{
			static HPEN hPen;

			if (hPen == 0)
				hPen = CreatePen(PS_SOLID, 5, RGB(128, 128, 128));

			SelectObject(hdc, hPen);
			draw_line(hdc, inf_ab_pos, center, scale, offset);
			draw_line(hdc, inf_bc_pos, center, scale, offset);
			draw_line(hdc, inf_ca_pos, center, scale, offset);

			// each vertex is a voronoi vertex, so the midpoints belong to one of the three
			SelectObject(hdc, brush_table[(j + 0) % COLOR_TABLE_SIZE]);
			draw_fill(hdc, a, RGB(128, 128, 128), scale, offset);
			SelectObject(hdc, brush_table[(j + 1) % COLOR_TABLE_SIZE]);
			draw_fill(hdc, b, RGB(128, 128, 128), scale, offset);
			SelectObject(hdc, brush_table[(j + 2) % COLOR_TABLE_SIZE]);
			draw_fill(hdc, c, RGB(128, 128, 128), scale, offset);
			SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
			SelectObject(hdc, GetStockObject(BLACK_PEN));
			//draw_point(hdc, center, 50, scale, offset);

			draw_point(hdc, a, 20, scale, offset);
			draw_point(hdc, b, 20, scale, offset);
			draw_point(hdc, c, 20, scale, offset);

		}

		if (draw_mode == 9999)
		{
			vec3 mid_ab = (a + b) / 2.0f;
			vec3 mid_bc = (b + c) / 2.0f;
			vec3 mid_ca = (c + a) / 2.0f;

			// edges of the voronoi bisect with each point
			// so take the midpoints of each edge in the output triangle, and that will give voronoi edge
			// from each point

			float radius;
			vec3 center;
			get_circum_circle(a, b, c, radius, center);
			//draw_circle(hdc, center, radius, scale, offset);


			vec3 inf_ab_pos = (mid_ab - center) * 100;
			vec3 inf_bc_pos = (mid_bc - center) * 100;
			vec3 inf_ca_pos = (mid_ca - center) * 100;

			vec3 inf_ab_neg = (mid_ab - center) * -100;
			vec3 inf_bc_neg = (mid_bc - center) * -100;
			vec3 inf_ca_neg = (mid_ca - center) * -100;

			// so we are going to save the dividing lines and later draw the cells

			// save edges for a and b
			voronoi_edge[ia][num_edge[ia]++] = inf_ab_pos;
			voronoi_edge[ia][num_edge[ia]++] = center;
			voronoi_edge[ia][num_edge[ia]++] = inf_ab_neg;
			voronoi_edge[ib][num_edge[ib]++] = inf_ab_pos;
			voronoi_edge[ib][num_edge[ib]++] = center;
			voronoi_edge[ib][num_edge[ib]++] = inf_ab_neg;

			// save edges for b and c
			voronoi_edge[ib][num_edge[ib]++] = inf_bc_pos;
			voronoi_edge[ib][num_edge[ib]++] = center;
			voronoi_edge[ib][num_edge[ib]++] = inf_bc_neg;
			voronoi_edge[ic][num_edge[ic]++] = inf_bc_pos;
			voronoi_edge[ic][num_edge[ic]++] = center;
			voronoi_edge[ic][num_edge[ic]++] = inf_bc_neg;

			// save edges for c and a
			voronoi_edge[ic][num_edge[ic]++] = inf_ca_neg;
			voronoi_edge[ic][num_edge[ic]++] = center;
			voronoi_edge[ic][num_edge[ic]++] = inf_ca_pos;
			voronoi_edge[ia][num_edge[ia]++] = inf_ca_pos;
			voronoi_edge[ia][num_edge[ia]++] = center;
			voronoi_edge[ia][num_edge[ia]++] = inf_ca_neg;

		}
	}

	if (draw_mode == TRIANGULATE_VORONOI2)
	{
		for (unsigned int j = 0; j < num_triangle; j++)
		{
			for (unsigned int k = 0; k < num_edge[j]; k += 3)
			{
				vec3 a = voronoi_edge[j][k + 0];
				vec3 center = voronoi_edge[j][k + 1];
				vec3 b = voronoi_edge[j][k + 2];



				SelectObject(hdc, pen_table[j % COLOR_TABLE_SIZE]);
				SelectObject(hdc, brush_table[j % COLOR_TABLE_SIZE]);
				draw_line(hdc, a, center, scale, offset);
				draw_line(hdc, b, center, scale, offset);

				// really need to intersect each line here with neighboring triangle lines
				// but kind of a PITA
				draw_point(hdc, triangle[j], 20, scale, offset);


			}
		}
		SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
		SelectObject(hdc, GetStockObject(BLACK_PEN));
	}

}

bool Triangulate::is_triangle_neighbor(vec3 &a1, vec3 &b1, vec3 &c1, vec3 &a2, vec3 &b2, vec3 &c2, vec3 *shared_out, unsigned int &num_shared)
{
	vec3 polygon[128];
	unsigned int num_poly = 0;
	vec3 shared[9];

	compare_edges(a1, b1, c1, a2, b2, c2, polygon, num_poly, shared, num_shared);

	if (num_shared == 2)
	{
		shared_out[0] = shared[0];
		shared_out[1] = shared[1];
		return true;
	}
	else
	{
		num_shared = 0;
		return false;
	}


}


#endif
