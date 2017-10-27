#include "menu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

vector<char *> Menu::console_buffer;
vector<char *> Menu::chat_buffer;
vector<char *> Menu::string_buffer;
vector<char *> Menu::notif_buffer;


#define DMESG_SIZE 2048
char dmesg[DMESG_SIZE][1024];
int dmesg_index = 0;

#define CHATMSG_SIZE 256
char chatmsg[CHATMSG_SIZE][1024];
int chatmsg_index = 0;

#define NOTIF_SIZE 7
char notifmsg[NOTIF_SIZE][1024];
int notif_index = 0;


Menu::Menu()
{
	bindnextkey = false;
	sprintf(data.resolution, "Unknown");
	sprintf(data.window, "Unknown");
	sprintf(data.attack, "Mouse1");
	sprintf(data.jump, "Enter + Space");
	sprintf(data.duck, "Shift");
	sprintf(data.zoom, "Mouse2");
	sprintf(data.use, "Mouse3");
	sprintf(data.weapnext, "mwheelup");
	sprintf(data.weapprev, "mwheeldown");
	sprintf(data.moveforward, "W + Up");
	sprintf(data.moveback, "S + Down");
	sprintf(data.moveleft, "A + Left");
	sprintf(data.moveright, "S + Right");
	sprintf(data.walk, "KP_INS");
	sprintf(data.spectate, "Control");
	sprintf(data.console, "Tilde");
	sprintf(data.menu, "Escape");
	sprintf(data.scores, "Tab");
	sprintf(data.name, "UnnamedPlayer");
	sprintf(data.model, "Visor");
	sprintf(data.skin, "Default");
	sprintf(data.team, "None");
	data.apply[0] = '\0';

	data.brightness = 0.5f;
	data.contrast = 0.5f;
	data.volume = 0.5f;
	data.musicvol = 0.5f;
	data.sfxvol = 0.5f;
	data.rscale = 0.5f;
	data.sensitivity = 1.0f;
	data.shadowvol = true;
	data.fov = 0.1f;
	data.bloom = true;
	data.invert = false;

}

void Menu::clear_console()
{
	console_buffer.clear();
}

void Menu::clear_chat()
{
	chat_buffer.clear();
}

void Menu::clear_notif()
{
	notif_buffer.clear();
}

void Menu::init(Graphics *gfx, Audio *audio, char **pk3_list, int num_pk3)
{
	float ident[16] = {	1.0f, 0.0f, 0.0f, 0.0f,
						0.0f, 1.0f, 0.0f, 0.0f,
						0.0f, 0.0f, 1.0f, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f};

	matrix = ident;

	Menu::gfx = gfx;
	Menu::audio = audio;

	console = false;
	ingame = false;
	chatmode = false;
	chat = true;
	notif = true;
	line_offset = 0;

	data.vsync = false;
	data.fullscreen = false;

	menu_object = load_texture(*gfx, "media/menu.tga", false, false);
	console_object = load_texture_pk3(*gfx, "media/gfx/misc/console01.tga", pk3_list, num_pk3, false, false);
	font_object = load_texture_pk3(*gfx, "media/gfx/2d/bigchars.tga", pk3_list, num_pk3, false, false);
	font.init(gfx);
//	strcpy(menu_wave.file, "media/sound/mwmusic.wav");
//	audio->load(menu_wave);
	menu_source = audio->create_source(false, true);
#ifndef DEDICATED
	alSourcef(menu_source, AL_GAIN, 0.125f);
#endif
	

	wave_t wave;
	waveFormat_t format;

	memset(&format, 0, sizeof(waveFormat_t));
	wave.format = &format;
	int buffer = audio->load_ogg("media/sound/menumusic.ogg", wave);
//	engine->snd_wave.push_back(wave);
	

	audio->select_buffer(menu_source, buffer);
	audio->play(menu_source);
	strcpy(delta_wave.file, "sound/misc/menu1.wav");
	audio->load(delta_wave);
	delta_source = audio->create_source(false, true);
	audio->select_buffer(delta_source, delta_wave.buffer);
	menu_state = 0;
	history_index = 0;
}

void Menu::handle_slider(char *out, float value)
{
	if (value >= 0.0 && value < 0.01)
	{
		strcat(out, "[         ]");
	}
	else if (value > 0.01 && value <= 0.101)
	{
		strcat(out, "[=         ]");
	}
	else if (value > 0.1 && value <= 0.201)
	{
		strcat(out, "[==        ]");
	}
	else if (value > 0.2 && value <= 0.301)
	{
		strcat(out, "[===       ]");
	}
	else if (value > 0.3 && value <= 0.401)
	{
		strcat(out, "[====      ]");
	}
	else if (value > 0.4 && value <= 0.501)
	{
		strcat(out, "[=====     ]");
	}
	else if (value > 0.5 && value <= 0.601)
	{
		strcat(out, "[======    ]");
	}
	else if (value > 0.6 && value <= 0.701)
	{
		strcat(out, "[=======   ]");
	}
	else if (value > 0.7 && value <= 0.801)
	{
		strcat(out, "[========  ]");
	}
	else if (value > 0.8 && value <= 0.901)
	{
		strcat(out, "[========= ]");
	}
	else if (value > 0.9 && value <= 1.01)
	{
		strcat(out, "[==========]");
	}
}

void Menu::sub_value(const char *str, char *out)
{
	int length = strlen(str);
	int j = 0;

	for (int i = 0; i < length;)
	{
		if (str[i] == '$')
		{
			out[j] = '\0';
			char key[80];
			sscanf(&str[i], "$%s", &key);

			if (strcmp(key, "d_fullscreen") == 0)
			{
				if (data.fullscreen)
				{
					strcat(out, "On");
					j += 2;
				}
				else
				{
					strcat(out, "Off");
					j += 3;
				}


				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_vsync") == 0)
			{
				if (data.vsync)
				{
					strcat(out, "On");
					j += 2;
				}
				else
				{
					strcat(out, "Off");
					j += 3;
				}


				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_ssao") == 0)
			{
				if (data.ssao)
				{
					strcat(out, "On");
					j += 2;
				}
				else
				{
					strcat(out, "Off");
					j += 3;
				}


				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_bloom") == 0)
			{
				if (data.bloom)
				{
					strcat(out, "On");
					j += 2;
				}
				else
				{
					strcat(out, "Off");
					j += 3;
				}


				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_shadowvol") == 0)
			{
				if (data.shadowvol)
				{
					strcat(out, "On");
					j += 2;
				}
				else
				{
					strcat(out, "Off");
					j += 3;
				}


				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_shadowmaps") == 0)
			{
				if (data.shadowmaps)
				{
					strcat(out, "On");
					j += 2;
				}
				else
				{
					strcat(out, "Off");
					j += 3;
				}


				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_portal") == 0)
			{
				if (data.portal)
				{
					strcat(out, "On");
					j += 2;
				}
				else
				{
					strcat(out, "Off");
					j += 3;
				}


				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_skyray") == 0)
			{
				if (data.skyray)
				{
					strcat(out, "On");
					j += 2;
				}
				else
				{
					strcat(out, "Off");
					j += 3;
				}


				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_brightness") == 0)
			{
				char value[32];
				handle_slider(out, data.brightness);
				j += 12;
				i += 1 + strlen(key);

				sprintf(value, " %3.1f", 2.0f * data.brightness);
				strcat(out, value);
				j += strlen(value);
				continue;
			}
			else if (strcmp(key, "d_contrast") == 0)
			{
				char value[32];

				handle_slider(out, data.contrast);
				j += 12;
				i += 1 + strlen(key);
				sprintf(value, " %3.1f", 2.0f * data.contrast);
				strcat(out, value);
				j += strlen(value);
				continue;
			}
			else if (strcmp(key, "d_volume") == 0)
			{
				char value[32];

				handle_slider(out, data.volume);
				j += 12;
				i += 1 + strlen(key);
				sprintf(value, " %3.1f", data.volume);
				strcat(out, value);
				j += strlen(value);
				continue;
			}
			else if (strcmp(key, "d_musicvol") == 0)
			{
				char value[32];

				handle_slider(out, data.musicvol);
				j += 12;
				i += 1 + strlen(key);
				sprintf(value, " %3.1f", data.musicvol);
				strcat(out, value);
				j += strlen(value);
				continue;
			}
			else if (strcmp(key, "d_sfxvol") == 0)
			{
				char value[32];

				handle_slider(out, data.sfxvol);
				j += 12;
				i += 1 + strlen(key);
				sprintf(value, " %3.1f", data.sfxvol);
				strcat(out, value);
				j += strlen(value);
				continue;
			}
			else if (strcmp(key, "d_fov") == 0)
			{
				char value[32];

				handle_slider(out, data.fov);
				j += 12;
				i += 1 + strlen(key);
				sprintf(value, " %3.1f", 100 * data.fov + 90);
				strcat(out, value);
				j += strlen(value);
				continue;
			}
			else if (strcmp(key, "d_sensitivity") == 0)
			{
				char value[32];

				handle_slider(out, data.fov);
				j += 12;
				i += 1 + strlen(key);
				sprintf(value, " %3.1f", 2.5 * data.sensitivity);
				strcat(out, value);
				j += strlen(value);
				continue;
			}
			else if (strcmp(key, "d_invert") == 0)
			{
				if (data.invert)
				{
					strcat(out, "On");
					j += 2;
				}
				else
				{
					strcat(out, "Off");
					j += 3;
				}


				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_resolution") == 0)
			{
				strcat(out, data.resolution);
				j += strlen(data.resolution);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_window") == 0)
			{
				strcat(out, data.window);
				j += strlen(data.window);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_rscale") == 0)
			{
				char value[32];

				handle_slider(out, data.rscale);
				j += 12;
				i += 1 + strlen(key);
				sprintf(value, " %3.1f", 2.0 * data.rscale);
				strcat(out, value);
				j += strlen(value);
				continue;
			}
			else if (strcmp(key, "d_name") == 0)
			{
				strcat(out, data.name);
				j += strlen(data.name);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_apply") == 0)
			{
				strcat(out, data.apply);
				j += strlen(data.apply);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_model") == 0)
			{
				strcat(out, data.model);
				j += strlen(data.model);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_skin") == 0)
			{
				strcat(out, data.skin);
				j += strlen(data.skin);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_skin") == 0)
			{
				strcat(out, data.skin);
				j += strlen(data.skin);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_team") == 0)
			{
				strcat(out, data.team);
				j += strlen(data.team);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_attack") == 0)
			{
				strcat(out, data.attack);
				j += strlen(data.attack);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_jump") == 0)
			{
				strcat(out, data.jump);
				j += strlen(data.jump);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_duck") == 0)
			{
				strcat(out, data.duck);
				j += strlen(data.duck);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_zoom") == 0)
			{
				strcat(out, data.zoom);
				j += strlen(data.zoom);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_use") == 0)
			{
				strcat(out, data.use);
				j += strlen(data.use);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_weapnext") == 0)
			{
				strcat(out, data.weapnext);
				j += strlen(data.weapnext);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_weapprev") == 0)
			{
				strcat(out, data.weapprev);
				j += strlen(data.weapprev);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_forward") == 0)
			{
				strcat(out, data.moveforward);
				j += strlen(data.moveforward);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_backward") == 0)
			{
				strcat(out, data.moveback);
				j += strlen(data.moveback);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_left") == 0)
			{
				strcat(out, data.moveleft);
				j += strlen(data.moveleft);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_right") == 0)
			{
				strcat(out, data.moveright);
				j += strlen(data.moveright);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_walk") == 0)
			{
				strcat(out, data.walk);
				j += strlen(data.walk);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_spectate") == 0)
			{
				strcat(out, data.spectate);
				j += strlen(data.spectate);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_console") == 0)
			{
				strcat(out, data.console);
				j += strlen(data.console);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_menu") == 0)
			{
				strcat(out, data.menu);
				j += strlen(data.menu);
				i += 1 + strlen(key);
				continue;
			}
			else if (strcmp(key, "d_scores") == 0)
			{
				strcat(out, data.scores);
				j += strlen(data.scores);
				i += 1 + strlen(key);
				continue;
			}





		}

		out[j++] = str[i++];
	}
}

void Menu::render(Global &global, bool ingame)
{
	gfx->SelectTexture(0, menu_object);
	gfx->SelectVertexBuffer(Model::quad_vertex);
	gfx->SelectIndexBuffer(Model::quad_index);
	for(unsigned int i = 0; i < menu_list.size(); i++)
	{
		menu_t *item = menu_list[i];
		if (item->state == menu_state)
		{
			if (item->flag & 1 && ingame == false)
			{
				gfx->clear();
				global.Select();
				global.Params(matrix, 0);
				gfx->DrawArrayTri(0, 0, 6, 4);
				gfx->cleardepth();
			}
			break;
		}
	}

	for(unsigned int i = 0; i < menu_list.size(); i++)
	{
		menu_t *item = menu_list[i];
		if (item->state == menu_state)
		{
			if (item->flag & 2)
			{
				// draw model
			}
			else if (item->flag & 4)
			{
				// draw texture
			}
			else
			{
				char msg[512];
				vec3 color(item->color[0], item->color[1], item->color[2]);

				memset(msg, 0, 512);
				sub_value(item->msg, msg);
				draw_text(msg, item->position[0],
					item->position[1], item->scale, color, true, true);
			}
		}
	}
}

void Menu::load(char *menu_file, char *state_file)
{
	FILE *menu = fopen(menu_file, "r");
	char line[1024];
	int ret;

	if (menu == NULL)
	{
		debugf("Unable to open %s\n", menu_file);
		return;
	}

	while ( fgets(line, sizeof(line), menu) )
	{
		menu_t *item = new menu_t;

		ret = sscanf(line, "menu %d %f %f %f %f %f %f %f %d %d \"%[^\"]s", &item->state,
			&item->position[0], &item->position[1], &item->position[2], &item->scale,
			&item->color[0], &item->color[1], &item->color[2], &item->flag, &item->mouse_state, item->msg);

		if (ret != 11)
		{
			delete item;
			continue;
		}
		else
		{
			menu_list.push_back(item);
		}
	}
	fclose(menu);

	FILE *state = fopen(state_file, "r");

	if (state == NULL)
	{
		debugf("Unable to open %s\n", state_file);
		return;
	}

	while ( fgets(line, sizeof(line), state) )
	{
		state_t *item = new state_t;

		ret = sscanf(line, "state %d %s %d \"%[^\"]s", &item->start,
			item->delta, &item->end, item->cmd);

		if (ret != 4)
		{
			delete item;
			continue;
		}
		else
		{
			state_list.push_back(item);
		}
	}
	fclose(state);
}

void Menu::play()
{
	audio->play(menu_source);
}

void Menu::stop()
{
	audio->stop(menu_source);
}

void Menu::delta(char *delta, Engine &altEngine)
{
	for(unsigned int i = 0; i < state_list.size(); i++)
	{
		state_t *item = state_list[i];
		if (item->start == menu_state)
		{
			if (strcmp(delta, item->delta) == 0)
			{
				menu_state = item->end;
				audio->play(delta_source);
				if (strcmp(item->cmd, "null") != 0)
					altEngine.console(item->cmd);
				break;
			}
		}
	}
	if (menu_state == 0 && ingame)
	{
		ingame = false;
		menu_state = 10;
	}
}

bool Menu::delta(float x, float y)
{
	float	min_dist = 2.0f;
	int		nearest = -1;

	for(unsigned int i = 0; i < menu_list.size(); i++)
	{
		menu_t *item = menu_list[i];
		if (item->state == menu_state)
		{
			float dist  = sqrt((item->position[1] - y) * (item->position[1] - y) + (item->position[0] - x) * (item->position[0] - x));
			if (min_dist > dist)
			{
				min_dist = dist;
				nearest = item->mouse_state;
			}
		}
	}

	if (nearest == -1 || nearest == menu_state)
		return false;
	else
	{
		menu_state = nearest;
		audio->play(delta_source);
		return true;
	}
}

void Menu::handle(char key, Engine *altEngine)
{
	static unsigned int last_key = 0;

	if (last_key + TICK_RATE / 8 > altEngine->tick_num)
	{
		return;
	}
	last_key = altEngine->tick_num;

	switch (key)
	{
	case 3:
		delta("up", *altEngine);
		break;
	case 4:
		delta("left", *altEngine);
		break;
	case 5:
		delta("down", *altEngine);
		break;
	case 6:
		delta("right", *altEngine);
		break;
	case '\r':
	case 14:
		delta("enter", *altEngine);
		break;
	case 15:
		delta("select", *altEngine);
		break;
	case '~':
	case '`':
		console = !console;
		break;
	case 27:
		if (altEngine->q3map.loaded == false)
			delta("unload", *altEngine);

		if (ingame && altEngine->ingame_menu_timer == 0)
		{
			ingame = false;
			altEngine->ingame_menu_timer = TICK_RATE >> 2;
		}
		break;
	}
}


// This could use some speeding up, rendering one character at a time is slow
void Menu::draw_text(char *str, float x, float y, float scale, vec3 &color, bool start, bool stop)
{
	float xpos = 0.0f;
	float ypos = 0.0f;
	unsigned int length = strlen(str);

	if (start)
	{
		gfx->Blend(true);
		gfx->Depth(false);
		gfx->BlendFuncSrcAlphaOneMinusSrcAlpha();
		gfx->SelectTexture(0, font_object);
		gfx->SelectIndexBuffer(Model::quad_index);
		gfx->SelectVertexBuffer(Model::quad_vertex);
		font.Select();
	}

	for(unsigned int i = 0; i < length; i++)
	{
		if (str[i] == '\n')
			continue;

		if (str[i] == '\t')
			continue;

		font.Params(str[i], x + xpos, y + ypos, scale, color);
		gfx->DrawArrayTri(0, 0, 6, 4);
		movepos(str[i], xpos, ypos, scale);
	}

	if (stop)
	{
		gfx->Depth(true);
		gfx->Blend(false);
	}
}

void Menu::render_console(Global &global)
{
	vec3 color(1.0f, 1.0f, 1.0f);

	gfx->SelectTexture(0, console_object);
	gfx->SelectIndexBuffer(Model::quad_index);
	gfx->SelectVertexBuffer(Model::quad_vertex);
	global.Select();
	matrix.m[13] = 1.0f;
	global.Params(matrix, 0);
	matrix.m[13] = 0.0f;
	gfx->DrawArrayTri(0, 0, 6, 4);

	gfx->cleardepth();
	for (unsigned int i = 0; i < console_buffer.size(); i++)
	{
		bool start = false;

		if (i == 0)
			start = true;

		draw_text(console_buffer[i], 0.0125f, 0.4f - 0.025f * (console_buffer.size() - 1 - i - line_offset), 0.025f, color, start, false);
	}
	strcat(key_buffer, "\4");
	draw_text(key_buffer, 0.0125f, 0.5f - 0.05f, 0.025f, color, false, false);
	key_buffer[strlen(key_buffer) - 1] = '\0';
	draw_text("Console", 0.85f - 0.0125f, 0.5f - 0.025f, 0.025f, color, false, true);
}

void Menu::render_chatmode(Global &global)
{
	vec3 color(1.0f, 1.0f, 1.0f);

	gfx->cleardepth();
	strcat(key_buffer, "\4");
	draw_text(key_buffer, 0.175f, 0.5f - 0.05f, 0.025f, color, true, false);
	key_buffer[strlen(key_buffer) - 1] = '\0';
	draw_text("Say:", 0.1f, 0.5f - 0.05f, 0.025f, color, false, true);
}

void Menu::render_stringmode(Global &global)
{
	vec3 color(1.0f, 1.0f, 1.0f);

	gfx->cleardepth();
	strcat(key_buffer, "\4");
	draw_text(key_buffer, 0.3f, 0.8f - 0.05f, 0.025f, color, true, false);
	key_buffer[strlen(key_buffer) - 1] = '\0';
	draw_text("Enter value:", 0.1f, 0.8f - 0.05f, 0.025f, color, false, true);
}

void Menu::render_chat(Global &global)
{
	vec3 color(1.0f, 1.0f, 1.0f);

	global.Select();
	matrix.m[13] = 1.0f;
	global.Params(matrix, 0);
	matrix.m[13] = 0.0f;

//	gfx->cleardepth();
	for (unsigned int i = 0; i < chat_buffer.size(); i++)
	{
		bool start = false;
		bool stop = false;

		if (i == 0)
			start = true;
		else if (i == chat_buffer.size() - 1)
			stop = true;

		draw_text(chat_buffer[i], 0.1f, 0.7f - 0.025f * (chat_buffer.size() - 1 - i), 0.025f, color, start, stop);
	}
}

void Menu::render_notif(Global &global)
{
	vec3 color(1.0f, 1.0f, 1.0f);

	global.Select();
	matrix.m[13] = 1.0f;
	global.Params(matrix, 0);
	matrix.m[13] = 0.0f;

//	gfx->cleardepth();
	for (unsigned int i = 0; i < notif_buffer.size(); i++)
	{
		bool start = false;
		bool stop = false;

		if (i == 0)
			start = true;
		else if (i == notif_buffer.size() - 1)
			stop = true;
		draw_text(notif_buffer[i], 0.1f, 0.2f - 0.025f * (notif_buffer.size() - 1 - i), 0.025f, color, start, stop);

		if (i == NOTIF_SIZE)
			break;
	}
}

void Menu::print_chat(const char *str)
{
	int size = strlen(str) + 1;
	char *line = chatmsg[chatmsg_index++];

	if (chatmsg_index == CHATMSG_SIZE)
		chatmsg_index = 0;

	memcpy(line, str, size);
	chat_buffer.push_back(line);
}

void Menu::print_notif(const char *str)
{
	int size = strlen(str) + 1;
	char *line = notifmsg[notif_index++];

	if (notif_index == NOTIF_SIZE)
		notif_index = 0;

	memcpy(line, str, size);
	notif_buffer.push_back(line);

	if (notif_buffer.size() > NOTIF_SIZE)
		notif_buffer.erase(notif_buffer.begin());
}

void Menu::print(const char *str)
{
	int size = strlen(str) + 1;
	char *line = dmesg[dmesg_index++];

	if (dmesg_index == DMESG_SIZE)
		dmesg_index = 0;

	memcpy(line, str, size);
	console_buffer.push_back(line);
}

void Menu::movepos(char c, float &xpos, float &ypos, float scale)
{
	// fake proportional font spacing
	switch(c)
	{
	case ' ':
	case ':':
		xpos += scale / 2.25f;
		break;
	case 'a':
	case 'g':
	case 'p':
	case 's':
	case 'u':
	case 'e':
	case 'o':
	case 'r':
	case 'h':
	case 'y':
	case 'x':
	case 'd':
		xpos += scale / 1.625f;
		break;
	case 'i':
	case 'l':
	case 't':
	case '.':
		xpos += scale / 1.85f;
		break;
	case '\n':
		ypos += scale;
		xpos = 0.0f;
		break;
	case '\t':
		xpos += 3 * (scale / 1.5f);
		break;
	default:
		xpos += scale / 1.5f;
		break;
	}
}

void Menu::handle_console(char key, Engine *altEngine)
{
	int length = strlen(key_buffer);

	switch(key)
	{
	case 3:
		if (history_index < cmd_buffer.size())
		{
			strcpy(key_buffer, cmd_buffer[cmd_buffer.size() - 1 - history_index]);
			history_index++;
		}
		break;
	case 5:
		if (history_index)
		{
			history_index--;
			strcpy(key_buffer, cmd_buffer[cmd_buffer.size() - 1 - history_index]);
		}
		else
		{
			key_buffer[0] = '\0';
		}
		break;
	case 1:
		if (line_offset > 0)
			line_offset--;
		break;
	case 2:
		line_offset++;
		break;
	case 10:
		if (line_offset >= 10)
			line_offset -= 10;
		break;
	case 11:
		line_offset += 10;
		break;
	case '`':
	case '~':
		console = false;
		key_buffer[0] = '\0';
		break;
	case '\r':
		{
		char *line = dmesg[dmesg_index++];

		if (dmesg_index == DMESG_SIZE)
			dmesg_index = 0;
		memcpy(line, key_buffer, strlen(key_buffer) + 1);
		cmd_buffer.push_back(line);
		altEngine->console(key_buffer);
		key_buffer[0] = '\0';
		history_index = 0;
		break;
		}
	case 4:
	case '\t':
		for (unsigned int i = 0; i < altEngine->num_cmd; i++)
		{
			if (strncmp(key_buffer, altEngine->cmd_list[i], length) == 0)
			{
				print(altEngine->cmd_list[i]);
			}
		}
		break;
	case '\b':
		if (length - 1 == -1)
			return;

		key_buffer[length - 1] = '\0';
		break;
	default:
		if (length + 1 == sizeof(key_buffer))
			return;
		key_buffer[length] = key;
		key_buffer[length + 1] = '\0';
	}
}

void Menu::handle_chatmode(char key, Engine *altEngine)
{
	int length = strlen(key_buffer);

	switch (key)
	{
	case 27:
		chatmode = false;
		break;
	case 3:
		if (history_index < chat_buffer.size())
		{
			strcpy(key_buffer, chat_buffer[chat_buffer.size() - 1 - history_index]);
			history_index++;
		}
		break;
	case 5:
		if (history_index)
		{
			history_index--;
			strcpy(key_buffer, chat_buffer[chat_buffer.size() - 1 - history_index]);
		}
		break;
	case '\r':
	{
		altEngine->chat(NULL, key_buffer);
		key_buffer[0] = '\0';
		chatmode = false;
		break;
	}
	case 4:
	case '\b':
		if (length - 1 == -1)
			return;

		key_buffer[length - 1] = '\0';
		break;
	default:
		if (length + 1 == sizeof(key_buffer))
			return;
		key_buffer[length] = key;
		key_buffer[length + 1] = '\0';
	}
}

void Menu::handle_stringmode(char key, Engine *altEngine)
{
	int length = strlen(key_buffer);

	switch (key)
	{
	case 27:
		*string_target = '\0';
		stringmode = false;
		break;
	case 3:
		if (history_index < string_buffer.size())
		{
			strcpy(key_buffer, string_buffer[string_buffer.size() - 1 - history_index]);
			history_index++;
		}
		break;
	case 5:
		if (history_index)
		{
			history_index--;
			strcpy(key_buffer, string_buffer[string_buffer.size() - 1 - history_index]);
		}
		break;
	case '\r':
	{
		strncpy(string_target, key_buffer, 127);
		key_buffer[0] = '\0';
		stringmode = false;
		break;
	}
	case 4:
	case '\b':
		if (length - 1 == -1)
			return;

		key_buffer[length - 1] = '\0';
		break;
	default:
		if (length + 1 == sizeof(key_buffer))
			return;
		key_buffer[length] = key;
		key_buffer[length + 1] = '\0';
	}
}

Menu::~Menu()
{
	for(unsigned int i = 0; i < state_list.size(); i++)
	{
		delete state_list[i];
	}

	for(unsigned int i = 0; i < menu_list.size(); i++)
	{
		delete menu_list[i];
	}
	

	delete [] menu_wave.data;
	delete [] delta_wave.data;

	state_list.clear();
}

void Menu::copy(char *data, unsigned int size)
{
	int length = MIN(size - 1, strlen(key_buffer));
	strncpy(data, key_buffer, length + 1);
}
