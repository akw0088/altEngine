#include "include.h"

#ifndef MENU_H
#define MENU_H

class Engine;

class Menu
{
public:
	void init(Graphics *gfx, Audio *audio);
	void render(Global &global);
	void load(char *menu_file, char *state_file);
	void delta(char *delta, Engine &altEngine);
	bool delta(float x, float y);
	void handle(char key, Engine *altEngine);
	void draw_text(char *str, float x, float y, float scale, vec3 &color);
	void draw_text(char *str, float x, float y, float z, float scale, vec3 &color);
	void render_console(Global &global);
	void handle_console(char key, Engine *altEngine);
	void movepos(char c, float &xpos, float &ypos, float scale);
	void print(const char *str);
	void stop();
	void play();
	void clear_console();
	~Menu();

	bool console;
	bool ingame;

	//making static so I can use it like printf
	static vector<char *> console_buffer;

private:
	matrix4 matrix;
	Graphics *gfx;
	Audio *audio;

	vector<char *> cmd_buffer;
	vector<menu_t *> menu_list;
	vector<state_t *> state_list;
	int menu_state;

	char key_buffer[1024];
	unsigned int history_index;

	mFont	font;
	int		menu_object;
	int		console_object;
	int		font_object;
	wave_t	menu_wave;
	int		menu_source;
	wave_t	delta_wave;
	int		delta_source;
};

#endif
