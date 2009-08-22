class Frame
{
public:
	Frame()
	{
		pos = vec3(0.0f, 0.0f, 0.0f);
		up = vec3(0.0f, 1.0f, 0.0f);
		forward = vec3(0.0f, 0.0f, 1.0f);
	}

	void update();
	void update(const vec2 &mouse);
	void set();
	vec3 pos, up, forward;
};

