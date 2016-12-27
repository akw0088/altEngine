#include "menu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

vector<char *> Menu::console_buffer;

#define DMESG_SIZE 256
char dmesg[DMESG_SIZE][1024];
int dmesg_index = 0;

void Menu::clear_console()
{
	console_buffer.clear();
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

	menu_object = load_texture(*gfx, "media/menu.tga", false);
	console_object = load_texture_pk3(*gfx, "media/gfx/misc/console01.tga", pk3_list, num_pk3, false);
	font_object = load_texture_pk3(*gfx, "media/gfx/2d/bigchars.tga", pk3_list, num_pk3, false);
	font.init(gfx);
	strcpy(menu_wave.file, "media/sound/mwmusic.wav");
	audio->load(menu_wave);
	menu_source = audio->create_source(false, true);
	audio->select_buffer(menu_source, menu_wave.buffer);
	audio->play(menu_source);
	strcpy(delta_wave.file, "sound/misc/menu1.wav");
	audio->load(delta_wave);
	delta_source = audio->create_source(false, true);
	audio->select_buffer(delta_source, delta_wave.buffer);
	menu_state = 0;
	history_index = 0;
}

void Menu::render(Global &global)
{
	gfx->SelectTexture(0, menu_object);
	gfx->SelectVertexBuffer(Model::quad_vertex);
	gfx->SelectIndexBuffer(Model::quad_index);
	for(unsigned int i = 0; i < menu_list.size(); i++)
	{
		menu_t *item = menu_list[i];
		if (item->state == menu_state)
		{
			if (item->flag & 1)
			{
				gfx->clear();
				global.Select();
				global.Params(matrix, 0);
				gfx->DrawArrayTri(0, 0, 6, 4);
//				gfx->SelectShader(0);
//				gfx->DeselectTexture(0);
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
				vec3 color(item->color[0], item->color[1], item->color[2]);
				draw_text(item->msg, item->position[0],
					item->position[1], item->scale, color);
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
			float dist  = abs32(item->position[1] - y);
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
	case '~':
	case '`':
		console = !console;
		break;
	case 27:
		ingame = false;
		break;
	}
}

void Menu::draw_text(char *str, float x, float y, float scale, vec3 &color)
{
	float xpos = 0.0f;
	float ypos = 0.0f;

	gfx->Blend(true);
	gfx->Depth(false);
	gfx->BlendFunc(NULL, NULL);
	gfx->SelectTexture(0, font_object);
	gfx->SelectIndexBuffer(Model::quad_index);
	gfx->SelectVertexBuffer(Model::quad_vertex);
	for(unsigned int i = 0; i < strlen(str); i++)
	{
		if (str[i] == '\n')
			continue;

		font.Select();
		font.Params(str[i], x + xpos, y + ypos, scale, color);
		gfx->DrawArrayTri(0, 0, 6, 4);
		movepos(str[i], xpos, ypos, scale);
	}
//	gfx->DeselectTexture(0);
	gfx->Depth(true);
	gfx->Blend(false);
}

void Menu::draw_text(char *str, float x, float y, float z, float scale, vec3 &color)
{
	float xpos = 0.0f;
	float ypos = 0.0f;

	gfx->Blend(true);
	gfx->Depth(false);
	gfx->BlendFunc(NULL, NULL);
	gfx->SelectTexture(0, font_object);
//	gfx->SelectVertexArrayObject(Model::quad_vao);
	gfx->SelectIndexBuffer(Model::quad_index);
	gfx->SelectVertexBuffer(Model::quad_vertex);
	for(unsigned int i = 0; i < strlen(str); i++)
	{
		font.Select();
		font.Params(str[i], x + xpos, y + ypos, z * scale, color);
		gfx->DrawArrayTri(0, 0, 6, 4);
		movepos(str[i], xpos, ypos, scale);
	}
//	gfx->DeselectTexture(0);
	gfx->Depth(true);
	gfx->Blend(false);
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
//	gfx->SelectShader(0);
//	gfx->DeselectTexture(0);

	gfx->cleardepth();
	for(unsigned int i = 0; i < console_buffer.size(); i++)
		draw_text(console_buffer[i], 0.0125f, 0.4f - 0.025f * (console_buffer.size() - 1 - i), 0.025f, color);
	strcat(key_buffer, "\4");
	draw_text(key_buffer, 0.0125f, 0.5f - 0.05f, 0.025f, color);
	key_buffer[strlen(key_buffer) - 1] = '\0';
	draw_text("Console", 0.85f - 0.0125f, 0.5f - 0.025f, 0.025f, color);
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
