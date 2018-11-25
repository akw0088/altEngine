export vmMain
code
proc vmMain 4 4
file "vm_main.c"
line 21
;1:#include "vm_stdlib.h"
;2:
;3:#define VM_VERSION "1.0.0"
;4:
;5:void VM_Printf(const char* fmt, ...);
;6:
;7:void VM_Init();
;8:void VM_Step();
;9:
;10:
;11:enum {
;12:	VM_INIT,
;13:	VM_STEP,
;14:	VM_DESTROY
;15:};
;16:
;17:
;18:int vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4,
;19:           int arg5, int arg6, int arg7, int arg8, int arg9, int arg10,
;20:           int arg11)
;21:{
line 22
;22:	switch (command)
ADDRLP4 0
ADDRFP4 0
INDIRI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 0
EQI4 $6
ADDRLP4 0
INDIRI4
CNSTI4 1
EQI4 $5
ADDRLP4 0
INDIRI4
CNSTI4 2
EQI4 $4
ADDRGP4 $3
JUMPV
line 23
;23:	{
LABELV $5
line 25
;24:	case VM_STEP:
;25:		VM_Step();
ADDRGP4 VM_Step
CALLV
pop
line 26
;26:		break;
ADDRGP4 $4
JUMPV
LABELV $6
line 28
;27:	case VM_INIT:
;28:		VM_Init();
ADDRGP4 VM_Init
CALLV
pop
line 29
;29:		break;
ADDRGP4 $4
JUMPV
line 31
;30:	case VM_DESTROY:
;31:		break;
LABELV $3
line 33
;32:	default:
;33:		VM_Printf("Unknown command %s\n");
ADDRGP4 $8
ARGP4
ADDRGP4 VM_Printf
CALLV
pop
line 34
;34:	}
LABELV $4
line 37
;35:
;36:
;37:    return 0;
CNSTI4 0
RETI4
LABELV $2
endproc vmMain 4 4
export VM_Printf
proc VM_Printf 1028 12
line 41
;38:}
;39:
;40:void VM_Printf(const char* fmt, ...)
;41:{
line 45
;42:	va_list argptr;
;43:	char    text[1024];
;44:
;45:	va_start(argptr, fmt);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 46
;46:	vsprintf(text, fmt, argptr);
ADDRLP4 4
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRGP4 vsprintf
CALLI4
pop
line 47
;47:	va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 49
;48:
;49:	trap_Printf(text);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Printf
CALLI4
pop
line 50
;50:}
LABELV $9
endproc VM_Printf 1028 12
export VM_Init
proc VM_Init 0 8
line 53
;51:
;52:void VM_Init()
;53:{
line 54
;54:	VM_Printf("------- VM Initialization -------\n");
ADDRGP4 $12
ARGP4
ADDRGP4 VM_Printf
CALLV
pop
line 55
;55:	VM_Printf("Version: %s\n", VM_VERSION);
ADDRGP4 $13
ARGP4
ADDRGP4 $14
ARGP4
ADDRGP4 VM_Printf
CALLV
pop
line 56
;56:	VM_Printf("Date: %s\n", __DATE__);
ADDRGP4 $15
ARGP4
ADDRGP4 $16
ARGP4
ADDRGP4 VM_Printf
CALLV
pop
line 57
;57:}
LABELV $11
endproc VM_Init 0 8
export VM_Step
proc VM_Step 0 0
line 60
;58:
;59:void VM_Step()
;60:{
line 61
;61:}ออ
LABELV $17
endproc VM_Step 0 0
import trap_Printf
import fabs
import abs
import memcpy
import memset
import memmove
import sscanf
import vsprintf
import _atoi
import atoi
import _atof
import atof
import toupper
import tolower
import strstr
import strchr
import strcmp
import strcpy
import strcat
import strlen
lit
align 1
LABELV $16
byte 1 83
byte 1 101
byte 1 112
byte 1 32
byte 1 48
byte 1 49
byte 1 32
byte 1 50
byte 1 48
byte 1 49
byte 1 56
byte 1 0
align 1
LABELV $15
byte 1 68
byte 1 97
byte 1 116
byte 1 101
byte 1 58
byte 1 32
byte 1 37
byte 1 115
byte 1 10
byte 1 0
align 1
LABELV $14
byte 1 49
byte 1 46
byte 1 48
byte 1 46
byte 1 48
byte 1 0
align 1
LABELV $13
byte 1 86
byte 1 101
byte 1 114
byte 1 115
byte 1 105
byte 1 111
byte 1 110
byte 1 58
byte 1 32
byte 1 37
byte 1 115
byte 1 10
byte 1 0
align 1
LABELV $12
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 32
byte 1 86
byte 1 77
byte 1 32
byte 1 73
byte 1 110
byte 1 105
byte 1 116
byte 1 105
byte 1 97
byte 1 108
byte 1 105
byte 1 122
byte 1 97
byte 1 116
byte 1 105
byte 1 111
byte 1 110
byte 1 32
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 10
byte 1 0
align 1
LABELV $8
byte 1 85
byte 1 110
byte 1 107
byte 1 110
byte 1 111
byte 1 119
byte 1 110
byte 1 32
byte 1 99
byte 1 111
byte 1 109
byte 1 109
byte 1 97
byte 1 110
byte 1 100
byte 1 32
byte 1 37
byte 1 115
byte 1 10
byte 1 0
