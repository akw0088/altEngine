#include "include.h"

void Engine::init(void *param1, void *param2)
{
	wave_t	wave;

	try
	{
		audio.init();
		audio.load("media/sound/mwmusic.wav", &wave);
		audio.play(&wave);
	}
	catch (const char *error)
	{
		printf("%s\n", error);
	}

	gfx.init(param1, param2);
	try
	{
		map.load("media/maps/q3tourney3.bsp");
	}
	catch (const char *error)
	{
		printf("%s\n", error);
		throw error;
	}
	map.generate_meshes(gfx);
	map.load_textures(gfx);
	
	map.get_collision_planes(&collision_plane, num_planes);
	num_planes = 1;
	collision_plane[0].normal = vec3(0.0f, 1.0f, 0.0f).normalize();
	collision_plane[0].d = 500.0f;
	Entity *box = new Entity(10.0f, vec3(0.0f, 20.0f, 0.0f));
	entity_list.add(box);
	parse_entity(map.getEnts(), entity_list);
}

void Engine::render()
{
	char msg[80];
	int i, j;

	gfx.clear();
	camera.set(gfx);
//	collision_plane[0].draw_plane();
	map.render(entity_list[0], gfx, keyboard);
	frame2ent(&camera, entity_list[0], keyboard);
	entity_list[0].render(gfx);
//	entity_list[0].in_frustum(entity_list[1]);
	for (i = 1, j = 0; i < entity_list.num; i++)
	{
		camera.set(gfx);
		entity_list[i].render(gfx);
		j++;
	}
//	glColor3f(1.0f, 1.0f, 1.0f);
	snprintf(msg, 80, "rendered %d entities", j);
	gfx.DrawText(msg, 0.01f, 0.06f);
	snprintf(msg, 80, "net_force: %3.3f %3.3f %3.3f", entity_list[0].net_force.x, entity_list[0].net_force.y, entity_list[0].net_force.z);
	gfx.DrawText(msg, 0.01f, 0.10f);
	snprintf(msg, 80, "angular_velocity: %3.3f %3.3f %3.3f", entity_list[0].angular_velocity.x, entity_list[0].angular_velocity.y, entity_list[0].angular_velocity.z);
	gfx.DrawText(msg, 0.01f, 0.14f);
	snprintf(msg, 80, "velocity: %3.3f %3.3f %3.3f", entity_list[0].velocity.x, entity_list[0].velocity.y, entity_list[0].velocity.z);
	gfx.DrawText(msg, 0.01f, 0.18f);
	snprintf(msg, 80, "position: %3.3f %3.3f %3.3f", entity_list[0].position.x, entity_list[0].position.y, entity_list[0].position.z);
	gfx.DrawText(msg, 0.01f, 0.22f);
	gfx.swap();
}

void Engine::step()
{
	camera.update(keyboard);

	// this is dog slow
	for(int i = 0; i < entity_list.num; i++)
	{
		float delta_time = 0.016f;
		float target_time = delta_time;
		float current_time = 0.0f;
		int divisions = 0;

		while (current_time < delta_time)
		{
			Entity new_entity(entity_list[i]);

			new_entity.integrate(target_time - current_time);
			if ( new_entity.collision_detect(collision_plane[0]) )
			{
				new_entity.vertex_array = NULL;
				new_entity.index_array = NULL;
				target_time = (current_time + target_time) / 2.0f;
				divisions++;

				if (divisions > 200)
				{
					new_entity.sleep = true;
					break;
//					throw "integration overflow";
				}
				continue;
			}
			entity_list[i] = new_entity;
			new_entity.vertex_array = NULL;
			new_entity.index_array = NULL;


			current_time = target_time;
			target_time = delta_time;
		}
	}

}

bool Engine::mousepos(int x, int y, int deltax, int deltay)
{
	if (keyboard.escape)
		return false;

	camera.update(vec2((float)deltax, (float)deltay));
	return true;
}

void Engine::keystroke(char *key, bool pressed)
{
	if (strcmp("enter", key) == 0)
		keyboard.enter = pressed;
	else if (strcmp("shift", key) == 0)
		keyboard.shift = pressed;
	else if (strcmp("control", key) == 0)
		keyboard.control = pressed;
	else if (strcmp("escape", key) == 0)
		keyboard.escape = pressed;
	else if (strcmp("up", key) == 0)
		keyboard.up = pressed;
	else if (strcmp("left", key) == 0)
		keyboard.left = pressed;
	else if (strcmp("down", key) == 0)
		keyboard.down = pressed;
	else if (strcmp("right", key) == 0)
		keyboard.right = pressed;
}

void Engine::resize(int width, int height)
{
	gfx.resize(width, height);
}

void Engine::destroy()
{
	map.unload();
	gfx.destroy();
	audio.destroy();
}
