#include "bg_lib.h"

void printf(const char* fmt, ...)
{
    va_list argptr;
    char    text[1024];

    va_start(argptr, fmt);
    vsprintf(text, fmt, argptr);
    va_end(argptr);

    trap_Printf(text);
}


int vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4,
           int arg5, int arg6, int arg7, int arg8, int arg9, int arg10,
           int arg11)
{
    if (command == 0)
    {
        printf("Game init!\n");
    }
    else
    {
        printf("Unknown command.\n");
    }

    return 0;
}

