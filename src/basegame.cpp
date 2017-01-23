#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BaseGame::BaseGame()
{
	gametype = GAMETYPE_DEATHMATCH;
	fraglimit = 8;
	notif_timer = 0;
	chat_timer = 0;
}
