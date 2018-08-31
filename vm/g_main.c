#include "bg_lib.h"

#define GAMEVERSION "1.0.0"

void G_Printf(const char* fmt, ...);

void G_InitGame();
void G_StepGame();


enum {
	G_INIT,
	G_STEP,
	G_DESTROY
};


int vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4,
           int arg5, int arg6, int arg7, int arg8, int arg9, int arg10,
           int arg11)
{
	switch (command)
	{
	case G_STEP:
		G_StepGame();
		break;
	case G_INIT:
		G_InitGame();
		break;
	case G_DESTROY:
		break;
	default:
		G_Printf("Unknown command %s\n");
	}


    return 0;
}

void G_Printf(const char* fmt, ...)
{
	va_list argptr;
	char    text[1024];

	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	trap_Printf(text);
}

void G_InitGame()
{
	G_Printf("------- Game Initialization -------\n");
	G_Printf("gamename: %s\n", GAMEVERSION);
	G_Printf("gamedate: %s\n", __DATE__);
}

void G_StepGame()
{
}