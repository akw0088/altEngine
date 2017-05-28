#ifndef FRAME_H
#define FRAME_H

class Frame
{
public:
	Frame()
	{
		reset();
	}

	void reset();
	void update(input_t &keyboard);
	void update(const vec2 &mouse, float scale);
	void set(matrix4 &trans);
	void set(matrix3 &trans);
	vec3 pos, up, forward;
};

#endif
