#include "include.h"

#ifndef MENU_H
#define MENU_H

class Engine;

class Menu
{
public:
	Menu();
	void init(Graphics *gfx, Audio *audio, char **pk3_list, int num_pk3);
	void render(Global &global, bool ingame = false);
	void load(char *menu_file, char *state_file);
	void delta(char *delta, Engine &altEngine);
	bool delta(float x, float y);
	void handle(char key, Engine *altEngine);
	void draw_text(char *str, float x, float y, float scale, vec3 &color, bool start, bool stop);
	void render_console(Global &global);
	void render_chatmode(Global &global);
	void render_stringmode(Global &global);
	void handle_console(char key, Engine *altEngine);
	void handle_chatmode(char key, Engine *altEngine);
	void handle_stringmode(char key, Engine *altEngine);
	void movepos(char c, float &xpos, float &ypos, float scale);
	void print(const char *str);
	void stop();
	void play();
	void clear_console();
	void copy(char *data, unsigned int size);

	void print_chat(const char *str);
	void render_chat(Global &global);
	void clear_chat();

	void print_notif(const char *str);
	void render_notif(Global &global);
	void clear_notif();

	void sub_value(const char *str, char *out);
	void handle_slider(char *out, float value);

	~Menu();

	bool chat;
	bool notif;
	bool console;
	bool ingame;
	bool chatmode;
	bool stringmode;
	bool bindnextkey;
	char *bindstr;
	char bindcmd[128];

	menudata_t data;

	//making static so I can use it like printf
	static vector<char *> console_buffer;
	static vector<char *> chat_buffer;
	static vector<char *> string_buffer;
	static vector<char *> notif_buffer;

	char *string_target;
	char string_cmd[128];

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
	unsigned int line_offset;

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
