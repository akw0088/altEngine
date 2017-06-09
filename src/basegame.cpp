#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BaseGame::BaseGame()
{
	gametype = GAMETYPE_DEATHMATCH;
	fraglimit = 8;
	timelimit = 10;
	capturelimit = 8;
	blue_flag_caps = 0;
	red_flag_caps = 0;
	notif_timer = 0;
	chat_timer = 0;
	engine = NULL;
}
