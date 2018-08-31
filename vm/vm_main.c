#include "vm_stdlib.h"

#define VM_VERSION "1.0.0"

void VM_Printf(const char* fmt, ...);

void VM_Init();
void VM_Step();


enum {
	VM_INIT,
	VM_STEP,
	VM_DESTROY
};


int vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4,
           int arg5, int arg6, int arg7, int arg8, int arg9, int arg10,
           int arg11)
{
	switch (command)
	{
	case VM_STEP:
		VM_Step();
		break;
	case VM_INIT:
		VM_Init();
		break;
	case VM_DESTROY:
		break;
	default:
		VM_Printf("Unknown command %s\n");
	}


    return 0;
}

void VM_Printf(const char* fmt, ...)
{
	va_list argptr;
	char    text[1024];

	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	trap_Printf(text);
}

void VM_Init()
{
	VM_Printf("------- VM Initialization -------\n");
	VM_Printf("Version: %s\n", VM_VERSION);
	VM_Printf("Date: %s\n", __DATE__);
}

void VM_Step()
{
}