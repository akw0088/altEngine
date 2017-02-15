#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BaseGame::BaseGame()
{
	gametype = GAMETYPE_DEATHMATCH;
	fraglimit = 8;
	capturelimit = 8;
	red_flag_caps = 0;
	blue_flag_caps = 0;
	notif_timer = 0;
	chat_timer = 0;
}
