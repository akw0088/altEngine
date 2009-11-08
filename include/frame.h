#ifndef FRAME_H
#define FRAME_H

class Frame
{
public:
	Frame()
	{
		pos = vec3(0.0f, 20.0f, 0.0f);
		up = vec3(0.0f, 1.0f, 0.0f);
		forward = vec3(0.0f, 0.0f, 1.0f);
	}

	void update(Keyboard &keyboard);
	void update(const vec2 &mouse);
	void set(Graphics &gfx);
	vec3 pos, up, forward;
};

void frame2ent(Frame *camera, Entity &entity, Keyboard &keyboard);

#endif
