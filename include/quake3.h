#include "include.h"

#ifndef QUAKE3_H
#define QUAKE3_H

class Quake3
{
public:
	Quake3();

	void init(Engine *engine);
	void step(int frame_step);
	void destroy();

	void handle_weapons(Player &player, Frame &camera_frame, button_t &input);

	void handle_plasma(Entity *entity, Player &player, Frame &camera_frame);
	void handle_rocketlauncher(Entity *entity, Player &player, Frame &camera_frame);
	void handle_grenade(Entity *entity, Player &player, Frame &camera_frame);
	void handle_lightning(Entity *entity, Player &player, Frame &camera_frame);
	void handle_railgun(Entity *entity, Player &player, Frame &camera_frame);
	void handle_machinegun(Player &player, Frame &camera_frame);
	void handle_shotgun(Player &player, Frame &camera_frame);
	void handle_gibs(Player &player, Frame &camera_frame);

	bool blink;

private:
	Engine *engine;
};

#endif

