export vmMain
code
proc vmMain 0 4
file "g_main.c"
line 8
;1:#include "bg_lib.h"
;2:
;3:void printf(const char* fmt, ...);
;4:
;5:int vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4,
;6:           int arg5, int arg6, int arg7, int arg8, int arg9, int arg10,
;7:           int arg11)
;8:{
line 9
;9:    if (command == 0)
ADDRFP4 0
INDIRI4
CNSTI4 0
NEI4 $2
line 10
;10:    {
line 11
;11:        printf("Game init!\n");
ADDRGP4 $4
ARGP4
ADDRGP4 printf
CALLV
pop
line 12
;12:    }
ADDRGP4 $3
JUMPV
LABELV $2
line 14
;13:    else
;14:    {
line 15
;15:        printf("Unknown command.\n");
ADDRGP4 $5
ARGP4
ADDRGP4 printf
CALLV
pop
line 16
;16:    }
LABELV $3
line 18
;17:
;18:    return 0;
CNSTI4 0
RETI4
LABELV $1
endproc vmMain 0 4
export printf
proc printf 1028 12
line 22
;19:}
;20:
;21:void printf(const char* fmt, ...)
;22:{
line 26
;23:    va_list argptr;
;24:    char    text[1024];
;25:
;26:    va_start(argptr, fmt);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 27
;27:    vsprintf(text, fmt, argptr);
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
line 28
;28:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 30
;29:
;30:    trap_Printf(text);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Printf
CALLI4
pop
line 31
;31:}
LABELV $6
endproc printf 1028 12
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
import rand
import srand
import qsort
lit
align 1
LABELV $5
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
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $4
byte 1 71
byte 1 97
byte 1 109
byte 1 101
byte 1 32
byte 1 105
byte 1 110
byte 1 105
byte 1 116
byte 1 33
byte 1 10
byte 1 0
