#include "vector.h"
#include "matrix.h"

class quaternion
{
public:
	matrix3 to_matrix();

	quaternion &operator=(const quaternion &q);
	quaternion operator+(const quaternion &q);
	quaternion operator-(const quaternion &q);
	quaternion operator*(const float scalar);
	quaternion operator*(const vec3 &vec);
	quaternion operator*(const quaternion &q);
	quaternion &operator*=(const quaternion &q);
	quaternion &operator+=(const quaternion &q);
	quaternion &operator-=(const quaternion &q);

	float s;
	float x;
	float y;
	float z;
};

