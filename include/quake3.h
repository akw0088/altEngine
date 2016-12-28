#include "include.h"

#ifndef QUAKE3_H
#define QUAKE3_H

class Quake3
{
	//Remove this when things seem settled
	friend class Engine;

public:
	Quake3();

	void init(Engine *engine);
	void step(int frame_step);
	void destroy();

	void render_hud(double last_frametime);
	void draw_name(vec4 &pos, Entity *entity, Menu &menu, vec3 &color);

	void handle_weapons(Player &player, Frame &camera_frame, button_t &input);

	void handle_plasma(Entity *entity, Player &player);
	void handle_rocketlauncher(Entity *entity, Player &player);
	void handle_grenade(Entity *entity, Player &player);
	void handle_lightning(Entity *entity, Player &player);
	void handle_railgun(Entity *entity, Player &player);
	void handle_machinegun(Player &player, int self);
	void handle_shotgun(Player &player);
	void handle_gibs(Player &player);

protected:
	bool blink;
	// for cloning
	Entity *box;
	Entity *ball;
	Entity *thug22;
	Entity *rocket;
	Entity *pineapple;

private:
	Engine *engine;


};

#endif

