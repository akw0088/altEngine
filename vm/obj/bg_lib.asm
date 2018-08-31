code
proc swapfunc 24 0
file "bg_lib.c"
line 44
;1:// bg_lib,c -- standard C library replacement routines used by code compiled for the virtual machine
;2:
;3:#include "bg_lib.h"
;4:
;5:
;6:#ifndef M_PI
;7:#define M_PI 3.14159265358979323846f // matches value in gcc v2 math.h
;8:#endif
;9:
;10:#ifndef NULL
;11:#define NULL ((void*)0)
;12:#endif
;13:
;14:static char* med3(char*, char*, char*, cmp_t*);
;15:static void  swapfunc(char*, char*, int, int);
;16:
;17:#ifndef min
;18:#define min(a, b) (a) < (b) ? a : b
;19:#endif
;20:
;21:/*
;22: * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
;23: */
;24:#define swapcode(TYPE, parmi, parmj, n)                                        \
;25:    {                                                                          \
;26:        long           i  = (n) / sizeof(TYPE);                                \
;27:        register TYPE* pi = (TYPE*)(parmi);                                    \
;28:        register TYPE* pj = (TYPE*)(parmj);                                    \
;29:        do                                                                     \
;30:        {                                                                      \
;31:            register TYPE t = *pi;                                             \
;32:            *pi++           = *pj;                                             \
;33:            *pj++           = t;                                               \
;34:        } while (--i > 0);                                                     \
;35:    }
;36:
;37:#define SWAPINIT(a, es)                                                        \
;38:    swaptype = ((char*)a - (char*)0) % sizeof(long) || es % sizeof(long)       \
;39:                   ? 2                                                         \
;40:                   : es == sizeof(long) ? 0 : 1;
;41:
;42:static void swapfunc(a, b, n, swaptype) char *a, *b;
;43:int         n, swaptype;
;44:{
line 45
;45:    if (swaptype <= 1)
ADDRFP4 12
INDIRI4
CNSTI4 1
GTI4 $2
line 46
;46:        swapcode(long, a, b, n) else swapcode(char, a, b, n)
ADDRLP4 8
ADDRFP4 8
INDIRI4
CVIU4 4
CNSTI4 2
RSHU4
CVUI4 4
ASGNI4
ADDRLP4 0
ADDRFP4 0
INDIRP4
ASGNP4
ADDRLP4 4
ADDRFP4 4
INDIRP4
ASGNP4
LABELV $4
ADDRLP4 12
ADDRLP4 0
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 16
ADDRLP4 0
INDIRP4
ASGNP4
ADDRLP4 0
ADDRLP4 16
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
ADDRLP4 16
INDIRP4
ADDRLP4 4
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 20
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 20
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
ADDRLP4 20
INDIRP4
ADDRLP4 12
INDIRI4
ASGNI4
LABELV $5
ADDRLP4 12
ADDRLP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
ADDRLP4 8
ADDRLP4 12
INDIRI4
ASGNI4
ADDRLP4 12
INDIRI4
CNSTI4 0
GTI4 $4
ADDRGP4 $3
JUMPV
LABELV $2
ADDRLP4 8
ADDRFP4 8
INDIRI4
CVIU4 4
CVUI4 4
ASGNI4
ADDRLP4 0
ADDRFP4 0
INDIRP4
ASGNP4
ADDRLP4 4
ADDRFP4 4
INDIRP4
ASGNP4
LABELV $7
ADDRLP4 12
ADDRLP4 0
INDIRP4
INDIRI1
ASGNI1
ADDRLP4 16
ADDRLP4 0
INDIRP4
ASGNP4
ADDRLP4 0
ADDRLP4 16
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 16
INDIRP4
ADDRLP4 4
INDIRP4
INDIRI1
ASGNI1
ADDRLP4 20
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 20
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 20
INDIRP4
ADDRLP4 12
INDIRI1
ASGNI1
LABELV $8
ADDRLP4 12
ADDRLP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
ADDRLP4 8
ADDRLP4 12
INDIRI4
ASGNI4
ADDRLP4 12
INDIRI4
CNSTI4 0
GTI4 $7
LABELV $3
line 47
;47:}
LABELV $1
endproc swapfunc 24 0
proc med3 40 8
line 65
;48:
;49:#define swap(a, b)                                                             \
;50:    if (swaptype == 0)                                                         \
;51:    {                                                                          \
;52:        long t      = *(long*)(a);                                             \
;53:        *(long*)(a) = *(long*)(b);                                             \
;54:        *(long*)(b) = t;                                                       \
;55:    }                                                                          \
;56:    else                                                                       \
;57:        swapfunc(a, b, es, swaptype)
;58:
;59:#define vecswap(a, b, n)                                                       \
;60:    if ((n) > 0)                                                               \
;61:    swapfunc(a, b, n, swaptype)
;62:
;63:static char *med3(a, b, c, cmp) char *a, *b, *c;
;64:cmp_t*       cmp;
;65:{
line 66
;66:    return cmp(a, b) < 0 ? (cmp(b, c) < 0 ? b : (cmp(a, c) < 0 ? c : a))
ADDRFP4 0
INDIRP4
ARGP4
ADDRFP4 4
INDIRP4
ARGP4
ADDRLP4 20
ADDRFP4 12
INDIRP4
CALLI4
ASGNI4
ADDRLP4 20
INDIRI4
CNSTI4 0
GEI4 $16
ADDRFP4 4
INDIRP4
ARGP4
ADDRFP4 8
INDIRP4
ARGP4
ADDRLP4 24
ADDRFP4 12
INDIRP4
CALLI4
ASGNI4
ADDRLP4 24
INDIRI4
CNSTI4 0
GEI4 $18
ADDRLP4 4
ADDRFP4 4
INDIRP4
ASGNP4
ADDRGP4 $19
JUMPV
LABELV $18
ADDRFP4 0
INDIRP4
ARGP4
ADDRFP4 8
INDIRP4
ARGP4
ADDRLP4 28
ADDRFP4 12
INDIRP4
CALLI4
ASGNI4
ADDRLP4 28
INDIRI4
CNSTI4 0
GEI4 $20
ADDRLP4 8
ADDRFP4 8
INDIRP4
ASGNP4
ADDRGP4 $21
JUMPV
LABELV $20
ADDRLP4 8
ADDRFP4 0
INDIRP4
ASGNP4
LABELV $21
ADDRLP4 4
ADDRLP4 8
INDIRP4
ASGNP4
LABELV $19
ADDRLP4 0
ADDRLP4 4
INDIRP4
ASGNP4
ADDRGP4 $17
JUMPV
LABELV $16
ADDRFP4 4
INDIRP4
ARGP4
ADDRFP4 8
INDIRP4
ARGP4
ADDRLP4 32
ADDRFP4 12
INDIRP4
CALLI4
ASGNI4
ADDRLP4 32
INDIRI4
CNSTI4 0
LEI4 $22
ADDRLP4 12
ADDRFP4 4
INDIRP4
ASGNP4
ADDRGP4 $23
JUMPV
LABELV $22
ADDRFP4 0
INDIRP4
ARGP4
ADDRFP4 8
INDIRP4
ARGP4
ADDRLP4 36
ADDRFP4 12
INDIRP4
CALLI4
ASGNI4
ADDRLP4 36
INDIRI4
CNSTI4 0
GEI4 $24
ADDRLP4 16
ADDRFP4 0
INDIRP4
ASGNP4
ADDRGP4 $25
JUMPV
LABELV $24
ADDRLP4 16
ADDRFP4 8
INDIRP4
ASGNP4
LABELV $25
ADDRLP4 12
ADDRLP4 16
INDIRP4
ASGNP4
LABELV $23
ADDRLP4 0
ADDRLP4 12
INDIRP4
ASGNP4
LABELV $17
ADDRLP4 0
INDIRP4
RETP4
LABELV $10
endproc med3 40 8
export qsort
proc qsort 104 16
line 73
;67:                         : (cmp(b, c) > 0 ? b : (cmp(a, c) < 0 ? a : c));
;68:}
;69:
;70:void   qsort(a, n, es, cmp) void* a;
;71:size_t n, es;
;72:cmp_t* cmp;
;73:{
LABELV $27
line 78
;74:    char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
;75:    int   d, r, swaptype, swap_cnt;
;76:
;77:loop:
;78:    SWAPINIT(a, es);
ADDRLP4 52
CNSTU4 3
ASGNU4
ADDRLP4 56
CNSTU4 0
ASGNU4
ADDRFP4 0
INDIRP4
CVPU4 4
CVUI4 4
CVIU4 4
ADDRLP4 52
INDIRU4
BANDU4
ADDRLP4 56
INDIRU4
NEU4 $32
ADDRFP4 8
INDIRI4
CVIU4 4
ADDRLP4 52
INDIRU4
BANDU4
ADDRLP4 56
INDIRU4
EQU4 $30
LABELV $32
ADDRLP4 44
CNSTI4 2
ASGNI4
ADDRGP4 $31
JUMPV
LABELV $30
ADDRFP4 8
INDIRI4
CVIU4 4
CNSTU4 4
NEU4 $33
ADDRLP4 48
CNSTI4 0
ASGNI4
ADDRGP4 $34
JUMPV
LABELV $33
ADDRLP4 48
CNSTI4 1
ASGNI4
LABELV $34
ADDRLP4 44
ADDRLP4 48
INDIRI4
ASGNI4
LABELV $31
ADDRLP4 16
ADDRLP4 44
INDIRI4
ASGNI4
line 79
;79:    swap_cnt = 0;
ADDRLP4 28
CNSTI4 0
ASGNI4
line 80
;80:    if (n < 7)
ADDRFP4 4
INDIRI4
CNSTI4 7
GEI4 $35
line 81
;81:    {
line 82
;82:        for (pm = (char*)a + es; pm < (char*)a + n * es; pm += es)
ADDRLP4 32
ADDRFP4 8
INDIRI4
ADDRFP4 0
INDIRP4
ADDP4
ASGNP4
ADDRGP4 $40
JUMPV
LABELV $37
line 83
;83:            for (pl = pm; pl > (char*)a && cmp(pl - es, pl) > 0; pl -= es)
ADDRLP4 0
ADDRLP4 32
INDIRP4
ASGNP4
ADDRGP4 $44
JUMPV
LABELV $41
line 84
;84:                swap(pl, pl - es);
ADDRLP4 16
INDIRI4
CNSTI4 0
NEI4 $45
ADDRLP4 60
ADDRLP4 0
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 0
INDIRP4
ADDRLP4 0
INDIRP4
ADDRFP4 8
INDIRI4
SUBP4
INDIRI4
ASGNI4
ADDRLP4 0
INDIRP4
ADDRFP4 8
INDIRI4
SUBP4
ADDRLP4 60
INDIRI4
ASGNI4
ADDRGP4 $46
JUMPV
LABELV $45
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 64
ADDRFP4 8
INDIRI4
ASGNI4
ADDRLP4 0
INDIRP4
ADDRLP4 64
INDIRI4
SUBP4
ARGP4
ADDRLP4 64
INDIRI4
ARGI4
ADDRLP4 16
INDIRI4
ARGI4
ADDRGP4 swapfunc
CALLV
pop
LABELV $46
LABELV $42
line 83
ADDRLP4 0
ADDRLP4 0
INDIRP4
ADDRFP4 8
INDIRI4
SUBP4
ASGNP4
LABELV $44
ADDRLP4 0
INDIRP4
CVPU4 4
ADDRFP4 0
INDIRP4
CVPU4 4
LEU4 $47
ADDRLP4 0
INDIRP4
ADDRFP4 8
INDIRI4
SUBP4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 72
ADDRFP4 12
INDIRP4
CALLI4
ASGNI4
ADDRLP4 72
INDIRI4
CNSTI4 0
GTI4 $41
LABELV $47
LABELV $38
line 82
ADDRLP4 32
ADDRFP4 8
INDIRI4
ADDRLP4 32
INDIRP4
ADDP4
ASGNP4
LABELV $40
ADDRLP4 32
INDIRP4
CVPU4 4
ADDRFP4 4
INDIRI4
ADDRFP4 8
INDIRI4
MULI4
ADDRFP4 0
INDIRP4
ADDP4
CVPU4 4
LTU4 $37
line 85
;85:        return;
ADDRGP4 $26
JUMPV
LABELV $35
line 87
;86:    }
;87:    pm = (char*)a + (n / 2) * es;
ADDRLP4 32
ADDRFP4 4
INDIRI4
CNSTI4 2
DIVI4
ADDRFP4 8
INDIRI4
MULI4
ADDRFP4 0
INDIRP4
ADDP4
ASGNP4
line 88
;88:    if (n > 7)
ADDRFP4 4
INDIRI4
CNSTI4 7
LEI4 $48
line 89
;89:    {
line 90
;90:        pl = a;
ADDRLP4 0
ADDRFP4 0
INDIRP4
ASGNP4
line 91
;91:        pn = (char*)a + (n - 1) * es;
ADDRLP4 36
ADDRFP4 4
INDIRI4
CNSTI4 1
SUBI4
ADDRFP4 8
INDIRI4
MULI4
ADDRFP4 0
INDIRP4
ADDP4
ASGNP4
line 92
;92:        if (n > 40)
ADDRFP4 4
INDIRI4
CNSTI4 40
LEI4 $50
line 93
;93:        {
line 94
;94:            d  = (n / 8) * es;
ADDRLP4 40
ADDRFP4 4
INDIRI4
CNSTI4 8
DIVI4
ADDRFP4 8
INDIRI4
MULI4
ASGNI4
line 95
;95:            pl = med3(pl, pl + d, pl + 2 * d, cmp);
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 64
ADDRLP4 40
INDIRI4
ASGNI4
ADDRLP4 64
INDIRI4
ADDRLP4 0
INDIRP4
ADDP4
ARGP4
ADDRLP4 64
INDIRI4
CNSTI4 1
LSHI4
ADDRLP4 0
INDIRP4
ADDP4
ARGP4
ADDRFP4 12
INDIRP4
ARGP4
ADDRLP4 68
ADDRGP4 med3
CALLP4
ASGNP4
ADDRLP4 0
ADDRLP4 68
INDIRP4
ASGNP4
line 96
;96:            pm = med3(pm - d, pm, pm + d, cmp);
ADDRLP4 76
ADDRLP4 40
INDIRI4
ASGNI4
ADDRLP4 32
INDIRP4
ADDRLP4 76
INDIRI4
SUBP4
ARGP4
ADDRLP4 32
INDIRP4
ARGP4
ADDRLP4 76
INDIRI4
ADDRLP4 32
INDIRP4
ADDP4
ARGP4
ADDRFP4 12
INDIRP4
ARGP4
ADDRLP4 80
ADDRGP4 med3
CALLP4
ASGNP4
ADDRLP4 32
ADDRLP4 80
INDIRP4
ASGNP4
line 97
;97:            pn = med3(pn - 2 * d, pn - d, pn, cmp);
ADDRLP4 88
ADDRLP4 40
INDIRI4
ASGNI4
ADDRLP4 36
INDIRP4
ADDRLP4 88
INDIRI4
CNSTI4 1
LSHI4
SUBP4
ARGP4
ADDRLP4 36
INDIRP4
ADDRLP4 88
INDIRI4
SUBP4
ARGP4
ADDRLP4 36
INDIRP4
ARGP4
ADDRFP4 12
INDIRP4
ARGP4
ADDRLP4 92
ADDRGP4 med3
CALLP4
ASGNP4
ADDRLP4 36
ADDRLP4 92
INDIRP4
ASGNP4
line 98
;98:        }
LABELV $50
line 99
;99:        pm = med3(pl, pm, pn, cmp);
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 32
INDIRP4
ARGP4
ADDRLP4 36
INDIRP4
ARGP4
ADDRFP4 12
INDIRP4
ARGP4
ADDRLP4 60
ADDRGP4 med3
CALLP4
ASGNP4
ADDRLP4 32
ADDRLP4 60
INDIRP4
ASGNP4
line 100
;100:    }
LABELV $48
line 101
;101:    swap(a, pm);
ADDRLP4 16
INDIRI4
CNSTI4 0
NEI4 $52
ADDRLP4 60
ADDRFP4 0
INDIRP4
INDIRI4
ASGNI4
ADDRFP4 0
INDIRP4
ADDRLP4 32
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 32
INDIRP4
ADDRLP4 60
INDIRI4
ASGNI4
ADDRGP4 $53
JUMPV
LABELV $52
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 32
INDIRP4
ARGP4
ADDRFP4 8
INDIRI4
ARGI4
ADDRLP4 16
INDIRI4
ARGI4
ADDRGP4 swapfunc
CALLV
pop
LABELV $53
line 102
;102:    pa = pb = (char*)a + es;
ADDRLP4 60
ADDRFP4 8
INDIRI4
ADDRFP4 0
INDIRP4
ADDP4
ASGNP4
ADDRLP4 4
ADDRLP4 60
INDIRP4
ASGNP4
ADDRLP4 20
ADDRLP4 60
INDIRP4
ASGNP4
line 104
;103:
;104:    pc = pd = (char*)a + (n - 1) * es;
ADDRLP4 64
ADDRFP4 4
INDIRI4
CNSTI4 1
SUBI4
ADDRFP4 8
INDIRI4
MULI4
ADDRFP4 0
INDIRP4
ADDP4
ASGNP4
ADDRLP4 24
ADDRLP4 64
INDIRP4
ASGNP4
ADDRLP4 8
ADDRLP4 64
INDIRP4
ASGNP4
line 105
;105:    for (;;)
line 106
;106:    {
ADDRGP4 $59
JUMPV
LABELV $58
line 108
;107:        while (pb <= pc && (r = cmp(pb, a)) <= 0)
;108:        {
line 109
;109:            if (r == 0)
ADDRLP4 12
INDIRI4
CNSTI4 0
NEI4 $61
line 110
;110:            {
line 111
;111:                swap_cnt = 1;
ADDRLP4 28
CNSTI4 1
ASGNI4
line 112
;112:                swap(pa, pb);
ADDRLP4 16
INDIRI4
CNSTI4 0
NEI4 $63
ADDRLP4 68
ADDRLP4 20
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 20
INDIRP4
ADDRLP4 4
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 4
INDIRP4
ADDRLP4 68
INDIRI4
ASGNI4
ADDRGP4 $64
JUMPV
LABELV $63
ADDRLP4 20
INDIRP4
ARGP4
ADDRLP4 4
INDIRP4
ARGP4
ADDRFP4 8
INDIRI4
ARGI4
ADDRLP4 16
INDIRI4
ARGI4
ADDRGP4 swapfunc
CALLV
pop
LABELV $64
line 113
;113:                pa += es;
ADDRLP4 20
ADDRFP4 8
INDIRI4
ADDRLP4 20
INDIRP4
ADDP4
ASGNP4
line 114
;114:            }
LABELV $61
line 115
;115:            pb += es;
ADDRLP4 4
ADDRFP4 8
INDIRI4
ADDRLP4 4
INDIRP4
ADDP4
ASGNP4
line 116
;116:        }
LABELV $59
line 107
ADDRLP4 4
INDIRP4
CVPU4 4
ADDRLP4 8
INDIRP4
CVPU4 4
GTU4 $65
ADDRLP4 4
INDIRP4
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 72
ADDRFP4 12
INDIRP4
CALLI4
ASGNI4
ADDRLP4 12
ADDRLP4 72
INDIRI4
ASGNI4
ADDRLP4 72
INDIRI4
CNSTI4 0
LEI4 $58
LABELV $65
ADDRGP4 $67
JUMPV
LABELV $66
line 118
;117:        while (pb <= pc && (r = cmp(pc, a)) >= 0)
;118:        {
line 119
;119:            if (r == 0)
ADDRLP4 12
INDIRI4
CNSTI4 0
NEI4 $69
line 120
;120:            {
line 121
;121:                swap_cnt = 1;
ADDRLP4 28
CNSTI4 1
ASGNI4
line 122
;122:                swap(pc, pd);
ADDRLP4 16
INDIRI4
CNSTI4 0
NEI4 $71
ADDRLP4 76
ADDRLP4 8
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 8
INDIRP4
ADDRLP4 24
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 24
INDIRP4
ADDRLP4 76
INDIRI4
ASGNI4
ADDRGP4 $72
JUMPV
LABELV $71
ADDRLP4 8
INDIRP4
ARGP4
ADDRLP4 24
INDIRP4
ARGP4
ADDRFP4 8
INDIRI4
ARGI4
ADDRLP4 16
INDIRI4
ARGI4
ADDRGP4 swapfunc
CALLV
pop
LABELV $72
line 123
;123:                pd -= es;
ADDRLP4 24
ADDRLP4 24
INDIRP4
ADDRFP4 8
INDIRI4
SUBP4
ASGNP4
line 124
;124:            }
LABELV $69
line 125
;125:            pc -= es;
ADDRLP4 8
ADDRLP4 8
INDIRP4
ADDRFP4 8
INDIRI4
SUBP4
ASGNP4
line 126
;126:        }
LABELV $67
line 117
ADDRLP4 4
INDIRP4
CVPU4 4
ADDRLP4 8
INDIRP4
CVPU4 4
GTU4 $73
ADDRLP4 8
INDIRP4
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 80
ADDRFP4 12
INDIRP4
CALLI4
ASGNI4
ADDRLP4 12
ADDRLP4 80
INDIRI4
ASGNI4
ADDRLP4 80
INDIRI4
CNSTI4 0
GEI4 $66
LABELV $73
line 127
;127:        if (pb > pc)
ADDRLP4 4
INDIRP4
CVPU4 4
ADDRLP4 8
INDIRP4
CVPU4 4
LEU4 $74
line 128
;128:            break;
ADDRGP4 $56
JUMPV
LABELV $74
line 129
;129:        swap(pb, pc);
ADDRLP4 16
INDIRI4
CNSTI4 0
NEI4 $76
ADDRLP4 84
ADDRLP4 4
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 4
INDIRP4
ADDRLP4 8
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 8
INDIRP4
ADDRLP4 84
INDIRI4
ASGNI4
ADDRGP4 $77
JUMPV
LABELV $76
ADDRLP4 4
INDIRP4
ARGP4
ADDRLP4 8
INDIRP4
ARGP4
ADDRFP4 8
INDIRI4
ARGI4
ADDRLP4 16
INDIRI4
ARGI4
ADDRGP4 swapfunc
CALLV
pop
LABELV $77
line 130
;130:        swap_cnt = 1;
ADDRLP4 28
CNSTI4 1
ASGNI4
line 131
;131:        pb += es;
ADDRLP4 4
ADDRFP4 8
INDIRI4
ADDRLP4 4
INDIRP4
ADDP4
ASGNP4
line 132
;132:        pc -= es;
ADDRLP4 8
ADDRLP4 8
INDIRP4
ADDRFP4 8
INDIRI4
SUBP4
ASGNP4
line 133
;133:    }
line 105
ADDRGP4 $59
JUMPV
LABELV $56
line 134
;134:    if (swap_cnt == 0)
ADDRLP4 28
INDIRI4
CNSTI4 0
NEI4 $78
line 135
;135:    { /* Switch to insertion sort */
line 136
;136:        for (pm = (char*)a + es; pm < (char*)a + n * es; pm += es)
ADDRLP4 32
ADDRFP4 8
INDIRI4
ADDRFP4 0
INDIRP4
ADDP4
ASGNP4
ADDRGP4 $83
JUMPV
LABELV $80
line 137
;137:            for (pl = pm; pl > (char*)a && cmp(pl - es, pl) > 0; pl -= es)
ADDRLP4 0
ADDRLP4 32
INDIRP4
ASGNP4
ADDRGP4 $87
JUMPV
LABELV $84
line 138
;138:                swap(pl, pl - es);
ADDRLP4 16
INDIRI4
CNSTI4 0
NEI4 $88
ADDRLP4 68
ADDRLP4 0
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 0
INDIRP4
ADDRLP4 0
INDIRP4
ADDRFP4 8
INDIRI4
SUBP4
INDIRI4
ASGNI4
ADDRLP4 0
INDIRP4
ADDRFP4 8
INDIRI4
SUBP4
ADDRLP4 68
INDIRI4
ASGNI4
ADDRGP4 $89
JUMPV
LABELV $88
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 72
ADDRFP4 8
INDIRI4
ASGNI4
ADDRLP4 0
INDIRP4
ADDRLP4 72
INDIRI4
SUBP4
ARGP4
ADDRLP4 72
INDIRI4
ARGI4
ADDRLP4 16
INDIRI4
ARGI4
ADDRGP4 swapfunc
CALLV
pop
LABELV $89
LABELV $85
line 137
ADDRLP4 0
ADDRLP4 0
INDIRP4
ADDRFP4 8
INDIRI4
SUBP4
ASGNP4
LABELV $87
ADDRLP4 0
INDIRP4
CVPU4 4
ADDRFP4 0
INDIRP4
CVPU4 4
LEU4 $90
ADDRLP4 0
INDIRP4
ADDRFP4 8
INDIRI4
SUBP4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 80
ADDRFP4 12
INDIRP4
CALLI4
ASGNI4
ADDRLP4 80
INDIRI4
CNSTI4 0
GTI4 $84
LABELV $90
LABELV $81
line 136
ADDRLP4 32
ADDRFP4 8
INDIRI4
ADDRLP4 32
INDIRP4
ADDP4
ASGNP4
LABELV $83
ADDRLP4 32
INDIRP4
CVPU4 4
ADDRFP4 4
INDIRI4
ADDRFP4 8
INDIRI4
MULI4
ADDRFP4 0
INDIRP4
ADDP4
CVPU4 4
LTU4 $80
line 139
;139:        return;
ADDRGP4 $26
JUMPV
LABELV $78
line 142
;140:    }
;141:
;142:    pn = (char*)a + n * es;
ADDRLP4 36
ADDRFP4 4
INDIRI4
ADDRFP4 8
INDIRI4
MULI4
ADDRFP4 0
INDIRP4
ADDP4
ASGNP4
line 143
;143:    r  = min(pa - (char*)a, pb - pa);
ADDRLP4 72
ADDRLP4 20
INDIRP4
CVPU4 4
ASGNU4
ADDRLP4 72
INDIRU4
ADDRFP4 0
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
ADDRLP4 4
INDIRP4
CVPU4 4
ADDRLP4 72
INDIRU4
SUBU4
CVUI4 4
GEI4 $92
ADDRLP4 68
ADDRLP4 20
INDIRP4
CVPU4 4
ADDRFP4 0
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
ASGNI4
ADDRGP4 $93
JUMPV
LABELV $92
ADDRLP4 68
ADDRLP4 4
INDIRP4
CVPU4 4
ADDRLP4 20
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
ASGNI4
LABELV $93
ADDRLP4 12
ADDRLP4 68
INDIRI4
ASGNI4
line 144
;144:    vecswap(a, pb - r, r);
ADDRLP4 12
INDIRI4
CNSTI4 0
LEI4 $94
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 4
INDIRP4
ADDRLP4 12
INDIRI4
SUBP4
ARGP4
ADDRLP4 12
INDIRI4
ARGI4
ADDRLP4 16
INDIRI4
ARGI4
ADDRGP4 swapfunc
CALLV
pop
LABELV $94
line 145
;145:    r = min(pd - pc, pn - pd - es);
ADDRLP4 84
ADDRLP4 24
INDIRP4
CVPU4 4
ASGNU4
ADDRLP4 84
INDIRU4
ADDRLP4 8
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
ADDRLP4 36
INDIRP4
CVPU4 4
ADDRLP4 84
INDIRU4
SUBU4
CVUI4 4
ADDRFP4 8
INDIRI4
SUBI4
GEI4 $97
ADDRLP4 80
ADDRLP4 24
INDIRP4
CVPU4 4
ADDRLP4 8
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
ASGNI4
ADDRGP4 $98
JUMPV
LABELV $97
ADDRLP4 80
ADDRLP4 36
INDIRP4
CVPU4 4
ADDRLP4 24
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
ADDRFP4 8
INDIRI4
SUBI4
ASGNI4
LABELV $98
ADDRLP4 12
ADDRLP4 80
INDIRI4
ASGNI4
line 146
;146:    vecswap(pb, pn - r, r);
ADDRLP4 12
INDIRI4
CNSTI4 0
LEI4 $99
ADDRLP4 4
INDIRP4
ARGP4
ADDRLP4 36
INDIRP4
ADDRLP4 12
INDIRI4
SUBP4
ARGP4
ADDRLP4 12
INDIRI4
ARGI4
ADDRLP4 16
INDIRI4
ARGI4
ADDRGP4 swapfunc
CALLV
pop
LABELV $99
line 147
;147:    if ((r = pb - pa) > es)
ADDRLP4 92
ADDRLP4 4
INDIRP4
CVPU4 4
ADDRLP4 20
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
ASGNI4
ADDRLP4 12
ADDRLP4 92
INDIRI4
ASGNI4
ADDRLP4 92
INDIRI4
ADDRFP4 8
INDIRI4
LEI4 $101
line 148
;148:        qsort(a, r / es, es, cmp);
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 96
ADDRFP4 8
INDIRI4
ASGNI4
ADDRLP4 12
INDIRI4
ADDRLP4 96
INDIRI4
DIVI4
ARGI4
ADDRLP4 96
INDIRI4
ARGI4
ADDRFP4 12
INDIRP4
ARGP4
ADDRGP4 qsort
CALLV
pop
LABELV $101
line 149
;149:    if ((r = pd - pc) > es)
ADDRLP4 100
ADDRLP4 24
INDIRP4
CVPU4 4
ADDRLP4 8
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
ASGNI4
ADDRLP4 12
ADDRLP4 100
INDIRI4
ASGNI4
ADDRLP4 100
INDIRI4
ADDRFP4 8
INDIRI4
LEI4 $103
line 150
;150:    {
line 152
;151:        /* Iterate rather than recurse to save stack space */
;152:        a = pn - r;
ADDRFP4 0
ADDRLP4 36
INDIRP4
ADDRLP4 12
INDIRI4
SUBP4
ASGNP4
line 153
;153:        n = r / es;
ADDRFP4 4
ADDRLP4 12
INDIRI4
ADDRFP4 8
INDIRI4
DIVI4
ASGNI4
line 154
;154:        goto loop;
ADDRGP4 $27
JUMPV
LABELV $103
line 157
;155:    }
;156:    /*      qsort(pn - r, r / es, es, cmp);*/
;157:}
LABELV $26
endproc qsort 104 16
export strlen
proc strlen 4 0
line 168
;158:
;159://==================================================================================
;160:
;161:// this file is excluded from release builds because of intrinsics
;162:
;163:// bk001211 - gcc errors on compiling strcpy:  parse error before
;164:// `__extension__'
;165:#if defined(Q3_VM)
;166:
;167:size_t strlen(const char* string)
;168:{
line 171
;169:    const char* s;
;170:
;171:    s = string;
ADDRLP4 0
ADDRFP4 0
INDIRP4
ASGNP4
ADDRGP4 $107
JUMPV
LABELV $106
line 173
;172:    while (*s)
;173:    {
line 174
;174:        s++;
ADDRLP4 0
ADDRLP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 175
;175:    }
LABELV $107
line 172
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $106
line 176
;176:    return s - string;
ADDRLP4 0
INDIRP4
CVPU4 4
ADDRFP4 0
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
RETI4
LABELV $105
endproc strlen 4 0
export strcat
proc strcat 16 0
line 180
;177:}
;178:
;179:char* strcat(char* strDestination, const char* strSource)
;180:{
line 183
;181:    char* s;
;182:
;183:    s = strDestination;
ADDRLP4 0
ADDRFP4 0
INDIRP4
ASGNP4
ADDRGP4 $111
JUMPV
LABELV $110
line 185
;184:    while (*s)
;185:    {
line 186
;186:        s++;
ADDRLP4 0
ADDRLP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 187
;187:    }
LABELV $111
line 184
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $110
ADDRGP4 $114
JUMPV
LABELV $113
line 189
;188:    while (*strSource)
;189:    {
line 190
;190:        *s++ = *strSource++;
ADDRLP4 4
ADDRLP4 0
INDIRP4
ASGNP4
ADDRLP4 12
CNSTI4 1
ASGNI4
ADDRLP4 0
ADDRLP4 4
INDIRP4
ADDRLP4 12
INDIRI4
ADDP4
ASGNP4
ADDRLP4 8
ADDRFP4 4
INDIRP4
ASGNP4
ADDRFP4 4
ADDRLP4 8
INDIRP4
ADDRLP4 12
INDIRI4
ADDP4
ASGNP4
ADDRLP4 4
INDIRP4
ADDRLP4 8
INDIRP4
INDIRI1
ASGNI1
line 191
;191:    }
LABELV $114
line 188
ADDRFP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $113
line 192
;192:    *s = 0;
ADDRLP4 0
INDIRP4
CNSTI1 0
ASGNI1
line 193
;193:    return strDestination;
ADDRFP4 0
INDIRP4
RETP4
LABELV $109
endproc strcat 16 0
export strcpy
proc strcpy 16 0
line 197
;194:}
;195:
;196:char* strcpy(char* strDestination, const char* strSource)
;197:{
line 200
;198:    char* s;
;199:
;200:    s = strDestination;
ADDRLP4 0
ADDRFP4 0
INDIRP4
ASGNP4
ADDRGP4 $118
JUMPV
LABELV $117
line 202
;201:    while (*strSource)
;202:    {
line 203
;203:        *s++ = *strSource++;
ADDRLP4 4
ADDRLP4 0
INDIRP4
ASGNP4
ADDRLP4 12
CNSTI4 1
ASGNI4
ADDRLP4 0
ADDRLP4 4
INDIRP4
ADDRLP4 12
INDIRI4
ADDP4
ASGNP4
ADDRLP4 8
ADDRFP4 4
INDIRP4
ASGNP4
ADDRFP4 4
ADDRLP4 8
INDIRP4
ADDRLP4 12
INDIRI4
ADDP4
ASGNP4
ADDRLP4 4
INDIRP4
ADDRLP4 8
INDIRP4
INDIRI1
ASGNI1
line 204
;204:    }
LABELV $118
line 201
ADDRFP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $117
line 205
;205:    *s = 0;
ADDRLP4 0
INDIRP4
CNSTI1 0
ASGNI1
line 206
;206:    return strDestination;
ADDRFP4 0
INDIRP4
RETP4
LABELV $116
endproc strcpy 16 0
export strcmp
proc strcmp 12 0
line 210
;207:}
;208:
;209:int strcmp(const char* string1, const char* string2)
;210:{
ADDRGP4 $122
JUMPV
LABELV $121
line 212
;211:    while (*string1 == *string2 && *string1 && *string2)
;212:    {
line 213
;213:        string1++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 214
;214:        string2++;
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 215
;215:    }
LABELV $122
line 211
ADDRLP4 0
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 4
ADDRFP4 4
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 0
INDIRI4
ADDRLP4 4
INDIRI4
NEI4 $125
ADDRLP4 8
CNSTI4 0
ASGNI4
ADDRLP4 0
INDIRI4
ADDRLP4 8
INDIRI4
EQI4 $125
ADDRLP4 4
INDIRI4
ADDRLP4 8
INDIRI4
NEI4 $121
LABELV $125
line 216
;216:    return *string1 - *string2;
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
ADDRFP4 4
INDIRP4
INDIRI1
CVII4 1
SUBI4
RETI4
LABELV $120
endproc strcmp 12 0
export strchr
proc strchr 0 0
line 220
;217:}
;218:
;219:char* strchr(const char* string, int c)
;220:{
ADDRGP4 $128
JUMPV
LABELV $127
line 222
;221:    while (*string)
;222:    {
line 223
;223:        if (*string == c)
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
ADDRFP4 4
INDIRI4
NEI4 $130
line 224
;224:        {
line 225
;225:            return (char*)string;
ADDRFP4 0
INDIRP4
RETP4
ADDRGP4 $126
JUMPV
LABELV $130
line 227
;226:        }
;227:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 228
;228:    }
LABELV $128
line 221
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $127
line 229
;229:    return (char*)0;
CNSTP4 0
RETP4
LABELV $126
endproc strchr 0 0
export strstr
proc strstr 8 0
line 233
;230:}
;231:
;232:char* strstr(const char* string, const char* strCharSet)
;233:{
ADDRGP4 $134
JUMPV
LABELV $133
line 235
;234:    while (*string)
;235:    {
line 238
;236:        int i;
;237:
;238:        for (i = 0; strCharSet[i]; i++)
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $139
JUMPV
LABELV $136
line 239
;239:        {
line 240
;240:            if (string[i] != strCharSet[i])
ADDRLP4 0
INDIRI4
ADDRFP4 0
INDIRP4
ADDP4
INDIRI1
CVII4 1
ADDRLP4 0
INDIRI4
ADDRFP4 4
INDIRP4
ADDP4
INDIRI1
CVII4 1
EQI4 $140
line 241
;241:            {
line 242
;242:                break;
ADDRGP4 $138
JUMPV
LABELV $140
line 244
;243:            }
;244:        }
LABELV $137
line 238
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $139
ADDRLP4 0
INDIRI4
ADDRFP4 4
INDIRP4
ADDP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $136
LABELV $138
line 245
;245:        if (!strCharSet[i])
ADDRLP4 0
INDIRI4
ADDRFP4 4
INDIRP4
ADDP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $142
line 246
;246:        {
line 247
;247:            return (char*)string;
ADDRFP4 0
INDIRP4
RETP4
ADDRGP4 $132
JUMPV
LABELV $142
line 249
;248:        }
;249:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 250
;250:    }
LABELV $134
line 234
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $133
line 251
;251:    return (char*)0;
CNSTP4 0
RETP4
LABELV $132
endproc strstr 8 0
export tolower
proc tolower 4 0
line 257
;252:}
;253:#endif // bk001211
;254:
;255:#if defined(Q3_VM)
;256:int tolower(int c)
;257:{
line 258
;258:    if (c >= 'A' && c <= 'Z')
ADDRLP4 0
ADDRFP4 0
INDIRI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 65
LTI4 $145
ADDRLP4 0
INDIRI4
CNSTI4 90
GTI4 $145
line 259
;259:    {
line 260
;260:        c += 'a' - 'A';
ADDRFP4 0
ADDRFP4 0
INDIRI4
CNSTI4 32
ADDI4
ASGNI4
line 261
;261:    }
LABELV $145
line 262
;262:    return c;
ADDRFP4 0
INDIRI4
RETI4
LABELV $144
endproc tolower 4 0
export toupper
proc toupper 4 0
line 266
;263:}
;264:
;265:int toupper(int c)
;266:{
line 267
;267:    if (c >= 'a' && c <= 'z')
ADDRLP4 0
ADDRFP4 0
INDIRI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 97
LTI4 $148
ADDRLP4 0
INDIRI4
CNSTI4 122
GTI4 $148
line 268
;268:    {
line 269
;269:        c += 'A' - 'a';
ADDRFP4 0
ADDRFP4 0
INDIRI4
CNSTI4 -32
ADDI4
ASGNI4
line 270
;270:    }
LABELV $148
line 271
;271:    return c;
ADDRFP4 0
INDIRI4
RETI4
LABELV $147
endproc toupper 4 0
export memmove
proc memmove 8 0
line 278
;272:}
;273:
;274:#endif
;275://#ifndef _MSC_VER
;276:
;277:void* memmove(void* dest, const void* src, size_t count)
;278:{
line 281
;279:    int i;
;280:
;281:    if (dest > src)
ADDRFP4 0
INDIRP4
CVPU4 4
ADDRFP4 4
INDIRP4
CVPU4 4
LEU4 $151
line 282
;282:    {
line 283
;283:        for (i = count - 1; i >= 0; i--)
ADDRLP4 0
ADDRFP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
ADDRGP4 $156
JUMPV
LABELV $153
line 284
;284:        {
line 285
;285:            ((char*)dest)[i] = ((char*)src)[i];
ADDRLP4 0
INDIRI4
ADDRFP4 0
INDIRP4
ADDP4
ADDRLP4 0
INDIRI4
ADDRFP4 4
INDIRP4
ADDP4
INDIRI1
ASGNI1
line 286
;286:        }
LABELV $154
line 283
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
LABELV $156
ADDRLP4 0
INDIRI4
CNSTI4 0
GEI4 $153
line 287
;287:    }
ADDRGP4 $152
JUMPV
LABELV $151
line 289
;288:    else
;289:    {
line 290
;290:        for (i = 0; i < count; i++)
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $160
JUMPV
LABELV $157
line 291
;291:        {
line 292
;292:            ((char*)dest)[i] = ((char*)src)[i];
ADDRLP4 0
INDIRI4
ADDRFP4 0
INDIRP4
ADDP4
ADDRLP4 0
INDIRI4
ADDRFP4 4
INDIRP4
ADDP4
INDIRI1
ASGNI1
line 293
;293:        }
LABELV $158
line 290
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $160
ADDRLP4 0
INDIRI4
ADDRFP4 8
INDIRI4
LTI4 $157
line 294
;294:    }
LABELV $152
line 295
;295:    return dest;
ADDRFP4 0
INDIRP4
RETP4
LABELV $150
endproc memmove 8 0
data
align 4
LABELV randSeed
byte 4 0
export srand
code
proc srand 0 0
line 301
;296:}
;297:
;298:static int randSeed = 0;
;299:
;300:void srand(unsigned seed)
;301:{
line 302
;302:    randSeed = seed;
ADDRGP4 randSeed
ADDRFP4 0
INDIRU4
CVUI4 4
ASGNI4
line 303
;303:}
LABELV $161
endproc srand 0 0
export rand
proc rand 4 0
line 306
;304:
;305:int rand(void)
;306:{
line 307
;307:    randSeed = (69069 * randSeed + 1);
ADDRLP4 0
ADDRGP4 randSeed
ASGNP4
ADDRLP4 0
INDIRP4
CNSTI4 69069
ADDRLP4 0
INDIRP4
INDIRI4
MULI4
CNSTI4 1
ADDI4
ASGNI4
line 308
;308:    return randSeed & 0x7fff;
ADDRGP4 randSeed
INDIRI4
CNSTI4 32767
BANDI4
RETI4
LABELV $162
endproc rand 4 0
export atof
proc atof 32 0
line 312
;309:}
;310:
;311:double atof(const char* string)
;312:{
ADDRGP4 $165
JUMPV
LABELV $164
line 319
;313:    float sign;
;314:    float value;
;315:    int   c;
;316:
;317:    // skip whitespace
;318:    while (*string <= ' ')
;319:    {
line 320
;320:        if (!*string)
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $167
line 321
;321:        {
line 322
;322:            return 0;
CNSTF4 0
RETF4
ADDRGP4 $163
JUMPV
LABELV $167
line 324
;323:        }
;324:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 325
;325:    }
LABELV $165
line 318
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 32
LEI4 $164
line 328
;326:
;327:    // check sign
;328:    switch (*string)
ADDRLP4 12
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 12
INDIRI4
CNSTI4 43
EQI4 $172
ADDRLP4 12
INDIRI4
CNSTI4 45
EQI4 $173
ADDRGP4 $169
JUMPV
line 329
;329:    {
LABELV $172
line 331
;330:    case '+':
;331:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 332
;332:        sign = 1;
ADDRLP4 8
CNSTF4 1065353216
ASGNF4
line 333
;333:        break;
ADDRGP4 $170
JUMPV
LABELV $173
line 335
;334:    case '-':
;335:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 336
;336:        sign = -1;
ADDRLP4 8
CNSTF4 3212836864
ASGNF4
line 337
;337:        break;
ADDRGP4 $170
JUMPV
LABELV $169
line 339
;338:    default:
;339:        sign = 1;
ADDRLP4 8
CNSTF4 1065353216
ASGNF4
line 340
;340:        break;
LABELV $170
line 344
;341:    }
;342:
;343:    // read digits
;344:    value = 0;
ADDRLP4 4
CNSTF4 0
ASGNF4
line 345
;345:    c     = string[0];
ADDRLP4 0
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
ASGNI4
line 346
;346:    if (c != '.')
ADDRLP4 0
INDIRI4
CNSTI4 46
EQI4 $174
line 347
;347:    {
LABELV $176
line 349
;348:        do
;349:        {
line 350
;350:            c = *string++;
ADDRLP4 20
ADDRFP4 0
INDIRP4
ASGNP4
ADDRFP4 0
ADDRLP4 20
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 0
ADDRLP4 20
INDIRP4
INDIRI1
CVII4 1
ASGNI4
line 351
;351:            if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $181
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $179
LABELV $181
line 352
;352:            {
line 353
;353:                break;
ADDRGP4 $175
JUMPV
LABELV $179
line 355
;354:            }
;355:            c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 356
;356:            value = value * 10 + c;
ADDRLP4 4
CNSTF4 1092616192
ADDRLP4 4
INDIRF4
MULF4
ADDRLP4 0
INDIRI4
CVIF4 4
ADDF4
ASGNF4
line 357
;357:        } while (1);
LABELV $177
ADDRGP4 $176
JUMPV
line 358
;358:    }
ADDRGP4 $175
JUMPV
LABELV $174
line 360
;359:    else
;360:    {
line 361
;361:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 362
;362:    }
LABELV $175
line 365
;363:
;364:    // check for decimal point
;365:    if (c == '.')
ADDRLP4 0
INDIRI4
CNSTI4 46
NEI4 $182
line 366
;366:    {
line 369
;367:        double fraction;
;368:
;369:        fraction = 0.1;
ADDRLP4 20
CNSTF4 1036831949
ASGNF4
LABELV $184
line 371
;370:        do
;371:        {
line 372
;372:            c = *string++;
ADDRLP4 24
ADDRFP4 0
INDIRP4
ASGNP4
ADDRFP4 0
ADDRLP4 24
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 0
ADDRLP4 24
INDIRP4
INDIRI1
CVII4 1
ASGNI4
line 373
;373:            if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $189
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $187
LABELV $189
line 374
;374:            {
line 375
;375:                break;
ADDRGP4 $186
JUMPV
LABELV $187
line 377
;376:            }
;377:            c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 378
;378:            value += c * fraction;
ADDRLP4 4
ADDRLP4 4
INDIRF4
ADDRLP4 0
INDIRI4
CVIF4 4
ADDRLP4 20
INDIRF4
MULF4
ADDF4
ASGNF4
line 379
;379:            fraction *= 0.1;
ADDRLP4 20
CNSTF4 1036831949
ADDRLP4 20
INDIRF4
MULF4
ASGNF4
line 380
;380:        } while (1);
LABELV $185
ADDRGP4 $184
JUMPV
LABELV $186
line 381
;381:    }
LABELV $182
line 385
;382:
;383:    // not handling 10e10 notation...
;384:
;385:    return value * sign;
ADDRLP4 4
INDIRF4
ADDRLP4 8
INDIRF4
MULF4
RETF4
LABELV $163
endproc atof 32 0
export _atof
proc _atof 36 0
line 389
;386:}
;387:
;388:double _atof(const char** stringPtr)
;389:{
line 393
;390:    const char* string;
;391:    float       sign;
;392:    float       value;
;393:    int         c = '0'; // bk001211 - uninitialized use possible
ADDRLP4 0
CNSTI4 48
ASGNI4
line 395
;394:
;395:    string = *stringPtr;
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
ADDRGP4 $192
JUMPV
LABELV $191
line 399
;396:
;397:    // skip whitespace
;398:    while (*string <= ' ')
;399:    {
line 400
;400:        if (!*string)
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $194
line 401
;401:        {
line 402
;402:            *stringPtr = string;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 403
;403:            return 0;
CNSTF4 0
RETF4
ADDRGP4 $190
JUMPV
LABELV $194
line 405
;404:        }
;405:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 406
;406:    }
LABELV $192
line 398
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 32
LEI4 $191
line 409
;407:
;408:    // check sign
;409:    switch (*string)
ADDRLP4 16
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 16
INDIRI4
CNSTI4 43
EQI4 $199
ADDRLP4 16
INDIRI4
CNSTI4 45
EQI4 $200
ADDRGP4 $196
JUMPV
line 410
;410:    {
LABELV $199
line 412
;411:    case '+':
;412:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 413
;413:        sign = 1;
ADDRLP4 12
CNSTF4 1065353216
ASGNF4
line 414
;414:        break;
ADDRGP4 $197
JUMPV
LABELV $200
line 416
;415:    case '-':
;416:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 417
;417:        sign = -1;
ADDRLP4 12
CNSTF4 3212836864
ASGNF4
line 418
;418:        break;
ADDRGP4 $197
JUMPV
LABELV $196
line 420
;419:    default:
;420:        sign = 1;
ADDRLP4 12
CNSTF4 1065353216
ASGNF4
line 421
;421:        break;
LABELV $197
line 425
;422:    }
;423:
;424:    // read digits
;425:    value = 0;
ADDRLP4 8
CNSTF4 0
ASGNF4
line 426
;426:    if (string[0] != '.')
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 46
EQI4 $201
line 427
;427:    {
LABELV $203
line 429
;428:        do
;429:        {
line 430
;430:            c = *string++;
ADDRLP4 24
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 24
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 0
ADDRLP4 24
INDIRP4
INDIRI1
CVII4 1
ASGNI4
line 431
;431:            if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $208
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $206
LABELV $208
line 432
;432:            {
line 433
;433:                break;
ADDRGP4 $205
JUMPV
LABELV $206
line 435
;434:            }
;435:            c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 436
;436:            value = value * 10 + c;
ADDRLP4 8
CNSTF4 1092616192
ADDRLP4 8
INDIRF4
MULF4
ADDRLP4 0
INDIRI4
CVIF4 4
ADDF4
ASGNF4
line 437
;437:        } while (1);
LABELV $204
ADDRGP4 $203
JUMPV
LABELV $205
line 438
;438:    }
LABELV $201
line 441
;439:
;440:    // check for decimal point
;441:    if (c == '.')
ADDRLP4 0
INDIRI4
CNSTI4 46
NEI4 $209
line 442
;442:    {
line 445
;443:        double fraction;
;444:
;445:        fraction = 0.1;
ADDRLP4 24
CNSTF4 1036831949
ASGNF4
LABELV $211
line 447
;446:        do
;447:        {
line 448
;448:            c = *string++;
ADDRLP4 28
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 28
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 0
ADDRLP4 28
INDIRP4
INDIRI1
CVII4 1
ASGNI4
line 449
;449:            if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $216
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $214
LABELV $216
line 450
;450:            {
line 451
;451:                break;
ADDRGP4 $213
JUMPV
LABELV $214
line 453
;452:            }
;453:            c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 454
;454:            value += c * fraction;
ADDRLP4 8
ADDRLP4 8
INDIRF4
ADDRLP4 0
INDIRI4
CVIF4 4
ADDRLP4 24
INDIRF4
MULF4
ADDF4
ASGNF4
line 455
;455:            fraction *= 0.1;
ADDRLP4 24
CNSTF4 1036831949
ADDRLP4 24
INDIRF4
MULF4
ASGNF4
line 456
;456:        } while (1);
LABELV $212
ADDRGP4 $211
JUMPV
LABELV $213
line 457
;457:    }
LABELV $209
line 460
;458:
;459:    // not handling 10e10 notation...
;460:    *stringPtr = string;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 462
;461:
;462:    return value * sign;
ADDRLP4 8
INDIRF4
ADDRLP4 12
INDIRF4
MULF4
RETF4
LABELV $190
endproc _atof 36 0
export atoi
proc atoi 28 0
line 467
;463:}
;464:
;465:#if defined(Q3_VM)
;466:int atoi(const char* string)
;467:{
ADDRGP4 $219
JUMPV
LABELV $218
line 474
;468:    int sign;
;469:    int value;
;470:    int c;
;471:
;472:    // skip whitespace
;473:    while (*string <= ' ')
;474:    {
line 475
;475:        if (!*string)
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $221
line 476
;476:        {
line 477
;477:            return 0;
CNSTI4 0
RETI4
ADDRGP4 $217
JUMPV
LABELV $221
line 479
;478:        }
;479:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 480
;480:    }
LABELV $219
line 473
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 32
LEI4 $218
line 483
;481:
;482:    // check sign
;483:    switch (*string)
ADDRLP4 12
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 12
INDIRI4
CNSTI4 43
EQI4 $226
ADDRLP4 12
INDIRI4
CNSTI4 45
EQI4 $227
ADDRGP4 $223
JUMPV
line 484
;484:    {
LABELV $226
line 486
;485:    case '+':
;486:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 487
;487:        sign = 1;
ADDRLP4 8
CNSTI4 1
ASGNI4
line 488
;488:        break;
ADDRGP4 $224
JUMPV
LABELV $227
line 490
;489:    case '-':
;490:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 491
;491:        sign = -1;
ADDRLP4 8
CNSTI4 -1
ASGNI4
line 492
;492:        break;
ADDRGP4 $224
JUMPV
LABELV $223
line 494
;493:    default:
;494:        sign = 1;
ADDRLP4 8
CNSTI4 1
ASGNI4
line 495
;495:        break;
LABELV $224
line 499
;496:    }
;497:
;498:    // read digits
;499:    value = 0;
ADDRLP4 4
CNSTI4 0
ASGNI4
LABELV $228
line 501
;500:    do
;501:    {
line 502
;502:        c = *string++;
ADDRLP4 20
ADDRFP4 0
INDIRP4
ASGNP4
ADDRFP4 0
ADDRLP4 20
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 0
ADDRLP4 20
INDIRP4
INDIRI1
CVII4 1
ASGNI4
line 503
;503:        if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $233
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $231
LABELV $233
line 504
;504:        {
line 505
;505:            break;
ADDRGP4 $230
JUMPV
LABELV $231
line 507
;506:        }
;507:        c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 508
;508:        value = value * 10 + c;
ADDRLP4 4
CNSTI4 10
ADDRLP4 4
INDIRI4
MULI4
ADDRLP4 0
INDIRI4
ADDI4
ASGNI4
line 509
;509:    } while (1);
LABELV $229
ADDRGP4 $228
JUMPV
LABELV $230
line 513
;510:
;511:    // not handling 10e10 notation...
;512:
;513:    return value * sign;
ADDRLP4 4
INDIRI4
ADDRLP4 8
INDIRI4
MULI4
RETI4
LABELV $217
endproc atoi 28 0
export _atoi
proc _atoi 32 0
line 517
;514:}
;515:
;516:int _atoi(const char** stringPtr)
;517:{
line 523
;518:    int         sign;
;519:    int         value;
;520:    int         c;
;521:    const char* string;
;522:
;523:    string = *stringPtr;
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
ADDRGP4 $236
JUMPV
LABELV $235
line 527
;524:
;525:    // skip whitespace
;526:    while (*string <= ' ')
;527:    {
line 528
;528:        if (!*string)
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $238
line 529
;529:        {
line 530
;530:            return 0;
CNSTI4 0
RETI4
ADDRGP4 $234
JUMPV
LABELV $238
line 532
;531:        }
;532:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 533
;533:    }
LABELV $236
line 526
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 32
LEI4 $235
line 536
;534:
;535:    // check sign
;536:    switch (*string)
ADDRLP4 16
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 16
INDIRI4
CNSTI4 43
EQI4 $243
ADDRLP4 16
INDIRI4
CNSTI4 45
EQI4 $244
ADDRGP4 $240
JUMPV
line 537
;537:    {
LABELV $243
line 539
;538:    case '+':
;539:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 540
;540:        sign = 1;
ADDRLP4 12
CNSTI4 1
ASGNI4
line 541
;541:        break;
ADDRGP4 $241
JUMPV
LABELV $244
line 543
;542:    case '-':
;543:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 544
;544:        sign = -1;
ADDRLP4 12
CNSTI4 -1
ASGNI4
line 545
;545:        break;
ADDRGP4 $241
JUMPV
LABELV $240
line 547
;546:    default:
;547:        sign = 1;
ADDRLP4 12
CNSTI4 1
ASGNI4
line 548
;548:        break;
LABELV $241
line 552
;549:    }
;550:
;551:    // read digits
;552:    value = 0;
ADDRLP4 8
CNSTI4 0
ASGNI4
LABELV $245
line 554
;553:    do
;554:    {
line 555
;555:        c = *string++;
ADDRLP4 24
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 24
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 0
ADDRLP4 24
INDIRP4
INDIRI1
CVII4 1
ASGNI4
line 556
;556:        if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $250
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $248
LABELV $250
line 557
;557:        {
line 558
;558:            break;
ADDRGP4 $247
JUMPV
LABELV $248
line 560
;559:        }
;560:        c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 561
;561:        value = value * 10 + c;
ADDRLP4 8
CNSTI4 10
ADDRLP4 8
INDIRI4
MULI4
ADDRLP4 0
INDIRI4
ADDI4
ASGNI4
line 562
;562:    } while (1);
LABELV $246
ADDRGP4 $245
JUMPV
LABELV $247
line 566
;563:
;564:    // not handling 10e10 notation...
;565:
;566:    *stringPtr = string;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 568
;567:
;568:    return value * sign;
ADDRLP4 8
INDIRI4
ADDRLP4 12
INDIRI4
MULI4
RETI4
LABELV $234
endproc _atoi 32 0
export abs
proc abs 4 0
line 572
;569:}
;570:
;571:int abs(int n)
;572:{
line 573
;573:    return n < 0 ? -n : n;
ADDRFP4 0
INDIRI4
CNSTI4 0
GEI4 $253
ADDRLP4 0
ADDRFP4 0
INDIRI4
NEGI4
ASGNI4
ADDRGP4 $254
JUMPV
LABELV $253
ADDRLP4 0
ADDRFP4 0
INDIRI4
ASGNI4
LABELV $254
ADDRLP4 0
INDIRI4
RETI4
LABELV $251
endproc abs 4 0
export fabs
proc fabs 4 0
line 577
;574:}
;575:
;576:double fabs(double x)
;577:{
line 578
;578:    return x < 0 ? -x : x;
ADDRFP4 0
INDIRF4
CNSTF4 0
GEF4 $257
ADDRLP4 0
ADDRFP4 0
INDIRF4
NEGF4
ASGNF4
ADDRGP4 $258
JUMPV
LABELV $257
ADDRLP4 0
ADDRFP4 0
INDIRF4
ASGNF4
LABELV $258
ADDRLP4 0
INDIRF4
RETF4
LABELV $255
endproc fabs 4 0
export AddInt
proc AddInt 56 0
line 598
;579:}
;580:
;581://=========================================================
;582:
;583:#define ALT 0x00000001       /* alternate form */
;584:#define HEXPREFIX 0x00000002 /* add 0x or 0X prefix */
;585:#define LADJUST 0x00000004   /* left adjustment */
;586:#define LONGDBL 0x00000008   /* long double */
;587:#define LONGINT 0x00000010   /* long integer */
;588:#define QUADINT 0x00000020   /* quad integer */
;589:#define SHORTINT 0x00000040  /* short integer */
;590:#define ZEROPAD 0x00000080   /* zero (as opposed to blank) pad */
;591:#define FPT 0x00000100       /* floating point number */
;592:
;593:#define to_digit(c) ((c) - '0')
;594:#define is_digit(c) ((unsigned)to_digit(c) <= 9)
;595:#define to_char(n) ((n) + '0')
;596:
;597:void AddInt(char** buf_p, int val, int width, int flags)
;598:{
line 604
;599:    char  text[32];
;600:    int   digits;
;601:    int   signedVal;
;602:    char* buf;
;603:
;604:    digits    = 0;
ADDRLP4 0
CNSTI4 0
ASGNI4
line 605
;605:    signedVal = val;
ADDRLP4 40
ADDRFP4 4
INDIRI4
ASGNI4
line 606
;606:    if (val < 0)
ADDRFP4 4
INDIRI4
CNSTI4 0
GEI4 $260
line 607
;607:    {
line 608
;608:        val = -val;
ADDRFP4 4
ADDRFP4 4
INDIRI4
NEGI4
ASGNI4
line 609
;609:    }
LABELV $260
LABELV $262
line 611
;610:    do
;611:    {
line 612
;612:        text[digits++] = '0' + val % 10;
ADDRLP4 44
ADDRLP4 0
INDIRI4
ASGNI4
ADDRLP4 0
ADDRLP4 44
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 44
INDIRI4
ADDRLP4 8
ADDP4
ADDRFP4 4
INDIRI4
CNSTI4 10
MODI4
CNSTI4 48
ADDI4
CVII1 4
ASGNI1
line 613
;613:        val /= 10;
ADDRFP4 4
ADDRFP4 4
INDIRI4
CNSTI4 10
DIVI4
ASGNI4
line 614
;614:    } while (val);
LABELV $263
ADDRFP4 4
INDIRI4
CNSTI4 0
NEI4 $262
line 616
;615:
;616:    if (signedVal < 0)
ADDRLP4 40
INDIRI4
CNSTI4 0
GEI4 $265
line 617
;617:    {
line 618
;618:        text[digits++] = '-';
ADDRLP4 44
ADDRLP4 0
INDIRI4
ASGNI4
ADDRLP4 0
ADDRLP4 44
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 44
INDIRI4
ADDRLP4 8
ADDP4
CNSTI1 45
ASGNI1
line 619
;619:    }
LABELV $265
line 621
;620:
;621:    buf = *buf_p;
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
line 623
;622:
;623:    if (!(flags & LADJUST))
ADDRFP4 12
INDIRI4
CNSTI4 4
BANDI4
CNSTI4 0
NEI4 $276
line 624
;624:    {
ADDRGP4 $270
JUMPV
LABELV $269
line 626
;625:        while (digits < width)
;626:        {
line 627
;627:            *buf++ = (flags & ZEROPAD) ? '0' : ' ';
ADDRLP4 48
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 48
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRFP4 12
INDIRI4
CNSTI4 128
BANDI4
CNSTI4 0
EQI4 $273
ADDRLP4 44
CNSTI4 48
ASGNI4
ADDRGP4 $274
JUMPV
LABELV $273
ADDRLP4 44
CNSTI4 32
ASGNI4
LABELV $274
ADDRLP4 48
INDIRP4
ADDRLP4 44
INDIRI4
CVII1 4
ASGNI1
line 628
;628:            width--;
ADDRFP4 8
ADDRFP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
line 629
;629:        }
LABELV $270
line 625
ADDRLP4 0
INDIRI4
ADDRFP4 8
INDIRI4
LTI4 $269
line 630
;630:    }
ADDRGP4 $276
JUMPV
LABELV $275
line 633
;631:
;632:    while (digits--)
;633:    {
line 634
;634:        *buf++ = text[digits];
ADDRLP4 44
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 44
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 44
INDIRP4
ADDRLP4 0
INDIRI4
ADDRLP4 8
ADDP4
INDIRI1
ASGNI1
line 635
;635:        width--;
ADDRFP4 8
ADDRFP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
line 636
;636:    }
LABELV $276
line 632
ADDRLP4 44
ADDRLP4 0
INDIRI4
ASGNI4
ADDRLP4 0
ADDRLP4 44
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
ADDRLP4 44
INDIRI4
CNSTI4 0
NEI4 $275
line 638
;637:
;638:    if (flags & LADJUST)
ADDRFP4 12
INDIRI4
CNSTI4 4
BANDI4
CNSTI4 0
EQI4 $278
line 639
;639:    {
ADDRGP4 $281
JUMPV
LABELV $280
line 641
;640:        while (width--)
;641:        {
line 642
;642:            *buf++ = (flags & ZEROPAD) ? '0' : ' ';
ADDRLP4 52
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 52
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRFP4 12
INDIRI4
CNSTI4 128
BANDI4
CNSTI4 0
EQI4 $284
ADDRLP4 48
CNSTI4 48
ASGNI4
ADDRGP4 $285
JUMPV
LABELV $284
ADDRLP4 48
CNSTI4 32
ASGNI4
LABELV $285
ADDRLP4 52
INDIRP4
ADDRLP4 48
INDIRI4
CVII1 4
ASGNI1
line 643
;643:        }
LABELV $281
line 640
ADDRLP4 48
ADDRFP4 8
INDIRI4
ASGNI4
ADDRFP4 8
ADDRLP4 48
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
ADDRLP4 48
INDIRI4
CNSTI4 0
NEI4 $280
line 644
;644:    }
LABELV $278
line 646
;645:
;646:    *buf_p = buf;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 647
;647:}
LABELV $259
endproc AddInt 56 0
export AddFloat
proc AddFloat 60 0
line 650
;648:
;649:void AddFloat(char** buf_p, float fval, int width, int prec)
;650:{
line 658
;651:    char  text[32];
;652:    int   digits;
;653:    float signedVal;
;654:    char* buf;
;655:    int   val;
;656:
;657:    // get the sign
;658:    signedVal = fval;
ADDRLP4 44
ADDRFP4 4
INDIRF4
ASGNF4
line 659
;659:    if (fval < 0)
ADDRFP4 4
INDIRF4
CNSTF4 0
GEF4 $287
line 660
;660:    {
line 661
;661:        fval = -fval;
ADDRFP4 4
ADDRFP4 4
INDIRF4
NEGF4
ASGNF4
line 662
;662:    }
LABELV $287
line 665
;663:
;664:    // write the float number
;665:    digits = 0;
ADDRLP4 0
CNSTI4 0
ASGNI4
line 666
;666:    val    = (int)fval;
ADDRLP4 4
ADDRFP4 4
INDIRF4
CVFI4 4
ASGNI4
LABELV $289
line 668
;667:    do
;668:    {
line 669
;669:        text[digits++] = '0' + val % 10;
ADDRLP4 48
ADDRLP4 0
INDIRI4
ASGNI4
ADDRLP4 0
ADDRLP4 48
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 48
INDIRI4
ADDRLP4 8
ADDP4
ADDRLP4 4
INDIRI4
CNSTI4 10
MODI4
CNSTI4 48
ADDI4
CVII1 4
ASGNI1
line 670
;670:        val /= 10;
ADDRLP4 4
ADDRLP4 4
INDIRI4
CNSTI4 10
DIVI4
ASGNI4
line 671
;671:    } while (val);
LABELV $290
ADDRLP4 4
INDIRI4
CNSTI4 0
NEI4 $289
line 673
;672:
;673:    if (signedVal < 0)
ADDRLP4 44
INDIRF4
CNSTF4 0
GEF4 $292
line 674
;674:    {
line 675
;675:        text[digits++] = '-';
ADDRLP4 48
ADDRLP4 0
INDIRI4
ASGNI4
ADDRLP4 0
ADDRLP4 48
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 48
INDIRI4
ADDRLP4 8
ADDP4
CNSTI1 45
ASGNI1
line 676
;676:    }
LABELV $292
line 678
;677:
;678:    buf = *buf_p;
ADDRLP4 40
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
ADDRGP4 $295
JUMPV
LABELV $294
line 681
;679:
;680:    while (digits < width)
;681:    {
line 682
;682:        *buf++ = ' ';
ADDRLP4 48
ADDRLP4 40
INDIRP4
ASGNP4
ADDRLP4 40
ADDRLP4 48
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 48
INDIRP4
CNSTI1 32
ASGNI1
line 683
;683:        width--;
ADDRFP4 8
ADDRFP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
line 684
;684:    }
LABELV $295
line 680
ADDRLP4 0
INDIRI4
ADDRFP4 8
INDIRI4
LTI4 $294
ADDRGP4 $298
JUMPV
LABELV $297
line 687
;685:
;686:    while (digits--)
;687:    {
line 688
;688:        *buf++ = text[digits];
ADDRLP4 48
ADDRLP4 40
INDIRP4
ASGNP4
ADDRLP4 40
ADDRLP4 48
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 48
INDIRP4
ADDRLP4 0
INDIRI4
ADDRLP4 8
ADDP4
INDIRI1
ASGNI1
line 689
;689:    }
LABELV $298
line 686
ADDRLP4 48
ADDRLP4 0
INDIRI4
ASGNI4
ADDRLP4 0
ADDRLP4 48
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
ADDRLP4 48
INDIRI4
CNSTI4 0
NEI4 $297
line 691
;690:
;691:    *buf_p = buf;
ADDRFP4 0
INDIRP4
ADDRLP4 40
INDIRP4
ASGNP4
line 693
;692:
;693:    if (prec < 0)
ADDRFP4 12
INDIRI4
CNSTI4 0
GEI4 $300
line 694
;694:        prec = 6;
ADDRFP4 12
CNSTI4 6
ASGNI4
LABELV $300
line 696
;695:    // write the fraction
;696:    digits = 0;
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $303
JUMPV
LABELV $302
line 698
;697:    while (digits < prec)
;698:    {
line 699
;699:        fval -= (int)fval;
ADDRLP4 52
ADDRFP4 4
INDIRF4
ASGNF4
ADDRFP4 4
ADDRLP4 52
INDIRF4
ADDRLP4 52
INDIRF4
CVFI4 4
CVIF4 4
SUBF4
ASGNF4
line 700
;700:        fval *= 10.0;
ADDRFP4 4
CNSTF4 1092616192
ADDRFP4 4
INDIRF4
MULF4
ASGNF4
line 701
;701:        val            = (int)fval;
ADDRLP4 4
ADDRFP4 4
INDIRF4
CVFI4 4
ASGNI4
line 702
;702:        text[digits++] = '0' + val % 10;
ADDRLP4 56
ADDRLP4 0
INDIRI4
ASGNI4
ADDRLP4 0
ADDRLP4 56
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 56
INDIRI4
ADDRLP4 8
ADDP4
ADDRLP4 4
INDIRI4
CNSTI4 10
MODI4
CNSTI4 48
ADDI4
CVII1 4
ASGNI1
line 703
;703:    }
LABELV $303
line 697
ADDRLP4 0
INDIRI4
ADDRFP4 12
INDIRI4
LTI4 $302
line 705
;704:
;705:    if (digits > 0)
ADDRLP4 0
INDIRI4
CNSTI4 0
LEI4 $305
line 706
;706:    {
line 707
;707:        buf    = *buf_p;
ADDRLP4 40
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
line 708
;708:        *buf++ = '.';
ADDRLP4 52
ADDRLP4 40
INDIRP4
ASGNP4
ADDRLP4 40
ADDRLP4 52
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 52
INDIRP4
CNSTI1 46
ASGNI1
line 709
;709:        for (prec = 0; prec < digits; prec++)
ADDRFP4 12
CNSTI4 0
ASGNI4
ADDRGP4 $310
JUMPV
LABELV $307
line 710
;710:        {
line 711
;711:            *buf++ = text[prec];
ADDRLP4 56
ADDRLP4 40
INDIRP4
ASGNP4
ADDRLP4 40
ADDRLP4 56
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 56
INDIRP4
ADDRFP4 12
INDIRI4
ADDRLP4 8
ADDP4
INDIRI1
ASGNI1
line 712
;712:        }
LABELV $308
line 709
ADDRFP4 12
ADDRFP4 12
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $310
ADDRFP4 12
INDIRI4
ADDRLP4 0
INDIRI4
LTI4 $307
line 713
;713:        *buf_p = buf;
ADDRFP4 0
INDIRP4
ADDRLP4 40
INDIRP4
ASGNP4
line 714
;714:    }
LABELV $305
line 715
;715:}
LABELV $286
endproc AddFloat 60 0
export AddString
proc AddString 20 4
line 718
;716:
;717:void AddString(char** buf_p, char* string, int width, int prec)
;718:{
line 722
;719:    int   size;
;720:    char* buf;
;721:
;722:    buf = *buf_p;
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
line 724
;723:
;724:    if (string == NULL)
ADDRFP4 4
INDIRP4
CVPU4 4
CNSTU4 0
NEU4 $312
line 725
;725:    {
line 726
;726:        string = "(null)";
ADDRFP4 4
ADDRGP4 $314
ASGNP4
line 727
;727:        prec   = -1;
ADDRFP4 12
CNSTI4 -1
ASGNI4
line 728
;728:    }
LABELV $312
line 730
;729:
;730:    if (prec >= 0)
ADDRFP4 12
INDIRI4
CNSTI4 0
LTI4 $315
line 731
;731:    {
line 732
;732:        for (size = 0; size < prec; size++)
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $320
JUMPV
LABELV $317
line 733
;733:        {
line 734
;734:            if (string[size] == '\0')
ADDRLP4 0
INDIRI4
ADDRFP4 4
INDIRP4
ADDP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $321
line 735
;735:            {
line 736
;736:                break;
ADDRGP4 $316
JUMPV
LABELV $321
line 738
;737:            }
;738:        }
LABELV $318
line 732
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $320
ADDRLP4 0
INDIRI4
ADDRFP4 12
INDIRI4
LTI4 $317
line 739
;739:    }
ADDRGP4 $316
JUMPV
LABELV $315
line 741
;740:    else
;741:    {
line 742
;742:        size = strlen(string);
ADDRFP4 4
INDIRP4
ARGP4
ADDRLP4 8
ADDRGP4 strlen
CALLI4
ASGNI4
ADDRLP4 0
ADDRLP4 8
INDIRI4
ASGNI4
line 743
;743:    }
LABELV $316
line 745
;744:
;745:    width -= size;
ADDRFP4 8
ADDRFP4 8
INDIRI4
ADDRLP4 0
INDIRI4
SUBI4
ASGNI4
ADDRGP4 $324
JUMPV
LABELV $323
line 748
;746:
;747:    while (size--)
;748:    {
line 749
;749:        *buf++ = *string++;
ADDRLP4 8
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 16
CNSTI4 1
ASGNI4
ADDRLP4 4
ADDRLP4 8
INDIRP4
ADDRLP4 16
INDIRI4
ADDP4
ASGNP4
ADDRLP4 12
ADDRFP4 4
INDIRP4
ASGNP4
ADDRFP4 4
ADDRLP4 12
INDIRP4
ADDRLP4 16
INDIRI4
ADDP4
ASGNP4
ADDRLP4 8
INDIRP4
ADDRLP4 12
INDIRP4
INDIRI1
ASGNI1
line 750
;750:    }
LABELV $324
line 747
ADDRLP4 8
ADDRLP4 0
INDIRI4
ASGNI4
ADDRLP4 0
ADDRLP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
ADDRLP4 8
INDIRI4
CNSTI4 0
NEI4 $323
ADDRGP4 $327
JUMPV
LABELV $326
line 753
;751:
;752:    while (width-- > 0)
;753:    {
line 754
;754:        *buf++ = ' ';
ADDRLP4 12
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 12
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 12
INDIRP4
CNSTI1 32
ASGNI1
line 755
;755:    }
LABELV $327
line 752
ADDRLP4 12
ADDRFP4 8
INDIRI4
ASGNI4
ADDRFP4 8
ADDRLP4 12
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
ADDRLP4 12
INDIRI4
CNSTI4 0
GTI4 $326
line 757
;756:
;757:    *buf_p = buf;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 758
;758:}
LABELV $311
endproc AddString 20 4
export vsprintf
proc vsprintf 72 16
line 769
;759:
;760:/*
;761:vsprintf
;762:
;763:I'm not going to support a bunch of the more arcane stuff in here
;764:just to keep it simpler.  For example, the '*' and '$' are not
;765:currently supported.  I've tried to make it so that it will just
;766:parse and ignore formats we don't support.
;767:*/
;768:int vsprintf(char* buffer, const char* fmt, va_list argptr)
;769:{
line 779
;770:    int*  arg;
;771:    char* buf_p;
;772:    char  ch;
;773:    int   flags;
;774:    int   width;
;775:    int   prec;
;776:    int   n;
;777:    char  sign;
;778:
;779:    buf_p = buffer;
ADDRLP4 4
ADDRFP4 0
INDIRP4
ASGNP4
line 780
;780:    arg   = (int*)argptr;
ADDRLP4 24
ADDRFP4 8
INDIRP4
ASGNP4
ADDRGP4 $331
JUMPV
LABELV $330
line 783
;781:
;782:    while (1)
;783:    {
line 785
;784:        // run through the format string until we hit a '%' or '\0'
;785:        for (ch = *fmt; (ch = *fmt) != '\0' && ch != '%'; fmt++)
ADDRLP4 0
ADDRFP4 4
INDIRP4
INDIRI1
ASGNI1
ADDRGP4 $336
JUMPV
LABELV $333
line 786
;786:        {
line 787
;787:            *buf_p++ = ch;
ADDRLP4 32
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 32
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 32
INDIRP4
ADDRLP4 0
INDIRI1
ASGNI1
line 788
;788:        }
LABELV $334
line 785
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
LABELV $336
ADDRLP4 29
ADDRFP4 4
INDIRP4
INDIRI1
ASGNI1
ADDRLP4 0
ADDRLP4 29
INDIRI1
ASGNI1
ADDRLP4 29
INDIRI1
CVII4 1
CNSTI4 0
EQI4 $337
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 37
NEI4 $333
LABELV $337
line 789
;789:        if (ch == '\0')
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $338
line 790
;790:        {
line 791
;791:            goto done;
ADDRGP4 $340
JUMPV
LABELV $338
line 795
;792:        }
;793:
;794:        // skip over the '%'
;795:        fmt++;
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 798
;796:
;797:        // reset formatting state
;798:        flags = 0;
ADDRLP4 16
CNSTI4 0
ASGNI4
line 799
;799:        width = 0;
ADDRLP4 12
CNSTI4 0
ASGNI4
line 800
;800:        prec  = -1;
ADDRLP4 20
CNSTI4 -1
ASGNI4
line 801
;801:        sign  = '\0';
ADDRLP4 28
CNSTI1 0
ASGNI1
LABELV $341
line 804
;802:
;803:    rflag:
;804:        ch = *fmt++;
ADDRLP4 32
ADDRFP4 4
INDIRP4
ASGNP4
ADDRFP4 4
ADDRLP4 32
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 0
ADDRLP4 32
INDIRP4
INDIRI1
ASGNI1
LABELV $342
line 806
;805:    reswitch:
;806:        switch (ch)
ADDRLP4 36
ADDRLP4 0
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 36
INDIRI4
CNSTI4 99
LTI4 $364
ADDRLP4 36
INDIRI4
CNSTI4 105
GTI4 $365
ADDRLP4 36
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 $366-396
ADDP4
INDIRP4
JUMPV
lit
align 4
LABELV $366
address $359
address $360
address $343
address $361
address $343
address $343
address $360
code
LABELV $364
ADDRLP4 36
INDIRI4
CNSTI4 37
LTI4 $343
ADDRLP4 36
INDIRI4
CNSTI4 57
GTI4 $343
ADDRLP4 36
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 $368-148
ADDP4
INDIRP4
JUMPV
lit
align 4
LABELV $368
address $363
address $343
address $343
address $343
address $343
address $343
address $343
address $343
address $346
address $347
address $343
address $354
address $355
address $355
address $355
address $355
address $355
address $355
address $355
address $355
address $355
code
LABELV $365
ADDRLP4 36
INDIRI4
CNSTI4 115
EQI4 $362
ADDRGP4 $343
JUMPV
line 807
;807:        {
LABELV $346
line 809
;808:        case '-':
;809:            flags |= LADJUST;
ADDRLP4 16
ADDRLP4 16
INDIRI4
CNSTI4 4
BORI4
ASGNI4
line 810
;810:            goto rflag;
ADDRGP4 $341
JUMPV
LABELV $347
line 812
;811:        case '.':
;812:            n = 0;
ADDRLP4 8
CNSTI4 0
ASGNI4
ADDRGP4 $349
JUMPV
LABELV $348
line 814
;813:            while (is_digit((ch = *fmt++)))
;814:            {
line 815
;815:                n = 10 * n + (ch - '0');
ADDRLP4 8
CNSTI4 10
ADDRLP4 8
INDIRI4
MULI4
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 48
SUBI4
ADDI4
ASGNI4
line 816
;816:            }
LABELV $349
line 813
ADDRLP4 48
ADDRFP4 4
INDIRP4
ASGNP4
ADDRFP4 4
ADDRLP4 48
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 52
ADDRLP4 48
INDIRP4
INDIRI1
ASGNI1
ADDRLP4 0
ADDRLP4 52
INDIRI1
ASGNI1
ADDRLP4 52
INDIRI1
CVII4 1
CNSTI4 48
SUBI4
CVIU4 4
CNSTU4 9
LEU4 $348
line 817
;817:            prec = n < 0 ? -1 : n;
ADDRLP4 8
INDIRI4
CNSTI4 0
GEI4 $352
ADDRLP4 56
CNSTI4 -1
ASGNI4
ADDRGP4 $353
JUMPV
LABELV $352
ADDRLP4 56
ADDRLP4 8
INDIRI4
ASGNI4
LABELV $353
ADDRLP4 20
ADDRLP4 56
INDIRI4
ASGNI4
line 818
;818:            goto reswitch;
ADDRGP4 $342
JUMPV
LABELV $354
line 820
;819:        case '0':
;820:            flags |= ZEROPAD;
ADDRLP4 16
ADDRLP4 16
INDIRI4
CNSTI4 128
BORI4
ASGNI4
line 821
;821:            goto rflag;
ADDRGP4 $341
JUMPV
LABELV $355
line 831
;822:        case '1':
;823:        case '2':
;824:        case '3':
;825:        case '4':
;826:        case '5':
;827:        case '6':
;828:        case '7':
;829:        case '8':
;830:        case '9':
;831:            n = 0;
ADDRLP4 8
CNSTI4 0
ASGNI4
LABELV $356
line 833
;832:            do
;833:            {
line 834
;834:                n  = 10 * n + (ch - '0');
ADDRLP4 8
CNSTI4 10
ADDRLP4 8
INDIRI4
MULI4
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 48
SUBI4
ADDI4
ASGNI4
line 835
;835:                ch = *fmt++;
ADDRLP4 60
ADDRFP4 4
INDIRP4
ASGNP4
ADDRFP4 4
ADDRLP4 60
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 0
ADDRLP4 60
INDIRP4
INDIRI1
ASGNI1
line 836
;836:            } while (is_digit(ch));
LABELV $357
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 48
SUBI4
CVIU4 4
CNSTU4 9
LEU4 $356
line 837
;837:            width = n;
ADDRLP4 12
ADDRLP4 8
INDIRI4
ASGNI4
line 838
;838:            goto reswitch;
ADDRGP4 $342
JUMPV
LABELV $359
line 840
;839:        case 'c':
;840:            *buf_p++ = (char)*arg;
ADDRLP4 60
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 60
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 60
INDIRP4
ADDRLP4 24
INDIRP4
INDIRI4
CVII1 4
ASGNI1
line 841
;841:            arg++;
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 842
;842:            break;
ADDRGP4 $344
JUMPV
LABELV $360
line 845
;843:        case 'd':
;844:        case 'i':
;845:            AddInt(&buf_p, *arg, width, flags);
ADDRLP4 4
ARGP4
ADDRLP4 24
INDIRP4
INDIRI4
ARGI4
ADDRLP4 12
INDIRI4
ARGI4
ADDRLP4 16
INDIRI4
ARGI4
ADDRGP4 AddInt
CALLV
pop
line 846
;846:            arg++;
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 847
;847:            break;
ADDRGP4 $344
JUMPV
LABELV $361
line 849
;848:        case 'f':
;849:            AddFloat(&buf_p, *(double*)arg, width, prec);
ADDRLP4 4
ARGP4
ADDRLP4 24
INDIRP4
INDIRF4
ARGF4
ADDRLP4 12
INDIRI4
ARGI4
ADDRLP4 20
INDIRI4
ARGI4
ADDRGP4 AddFloat
CALLV
pop
line 851
;850:#ifdef __LCC__
;851:            arg += 1; // everything is 32 bit in my compiler
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 855
;852:#else
;853:            arg += 2;
;854:#endif
;855:            break;
ADDRGP4 $344
JUMPV
LABELV $362
line 857
;856:        case 's':
;857:            AddString(&buf_p, (char*)*arg, width, prec);
ADDRLP4 4
ARGP4
ADDRLP4 24
INDIRP4
INDIRI4
CVIU4 4
CVUP4 4
ARGP4
ADDRLP4 12
INDIRI4
ARGI4
ADDRLP4 20
INDIRI4
ARGI4
ADDRGP4 AddString
CALLV
pop
line 858
;858:            arg++;
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 859
;859:            break;
ADDRGP4 $344
JUMPV
LABELV $363
line 861
;860:        case '%':
;861:            *buf_p++ = ch;
ADDRLP4 64
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 64
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 64
INDIRP4
ADDRLP4 0
INDIRI1
ASGNI1
line 862
;862:            break;
ADDRGP4 $344
JUMPV
LABELV $343
line 864
;863:        default:
;864:            *buf_p++ = (char)*arg;
ADDRLP4 68
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 68
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 68
INDIRP4
ADDRLP4 24
INDIRP4
INDIRI4
CVII1 4
ASGNI1
line 865
;865:            arg++;
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 866
;866:            break;
LABELV $344
line 868
;867:        }
;868:    }
LABELV $331
line 782
ADDRGP4 $330
JUMPV
LABELV $340
line 871
;869:
;870:done:
;871:    *buf_p = 0;
ADDRLP4 4
INDIRP4
CNSTI1 0
ASGNI1
line 872
;872:    return buf_p - buffer;
ADDRLP4 4
INDIRP4
CVPU4 4
ADDRFP4 0
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
RETI4
LABELV $329
endproc vsprintf 72 16
export sscanf
proc sscanf 28 4
line 877
;873:}
;874:
;875:/* this is really crappy */
;876:int sscanf(const char* buffer, const char* fmt, ...)
;877:{
line 882
;878:    int   cmd;
;879:    int** arg;
;880:    int   count;
;881:
;882:    arg   = (int**)&fmt + 1;
ADDRLP4 4
ADDRFP4 4+4
ASGNP4
line 883
;883:    count = 0;
ADDRLP4 8
CNSTI4 0
ASGNI4
ADDRGP4 $373
JUMPV
LABELV $372
line 886
;884:
;885:    while (*fmt)
;886:    {
line 887
;887:        if (fmt[0] != '%')
ADDRFP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 37
EQI4 $375
line 888
;888:        {
line 889
;889:            fmt++;
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 890
;890:            continue;
ADDRGP4 $373
JUMPV
LABELV $375
line 893
;891:        }
;892:
;893:        cmd = fmt[1];
ADDRLP4 0
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
INDIRI1
CVII4 1
ASGNI4
line 894
;894:        fmt += 2;
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 2
ADDP4
ASGNP4
line 896
;895:
;896:        switch (cmd)
ADDRLP4 16
CNSTI4 100
ASGNI4
ADDRLP4 0
INDIRI4
ADDRLP4 16
INDIRI4
EQI4 $379
ADDRLP4 0
INDIRI4
CNSTI4 102
EQI4 $380
ADDRLP4 0
INDIRI4
CNSTI4 105
EQI4 $379
ADDRLP4 0
INDIRI4
ADDRLP4 16
INDIRI4
LTI4 $377
LABELV $381
ADDRLP4 0
INDIRI4
CNSTI4 117
EQI4 $379
ADDRGP4 $377
JUMPV
line 897
;897:        {
LABELV $379
line 901
;898:        case 'i':
;899:        case 'd':
;900:        case 'u':
;901:            **arg = _atoi(&buffer);
ADDRFP4 0
ARGP4
ADDRLP4 20
ADDRGP4 _atoi
CALLI4
ASGNI4
ADDRLP4 4
INDIRP4
INDIRP4
ADDRLP4 20
INDIRI4
ASGNI4
line 902
;902:            break;
ADDRGP4 $378
JUMPV
LABELV $380
line 904
;903:        case 'f':
;904:            *(float*)*arg = _atof(&buffer);
ADDRFP4 0
ARGP4
ADDRLP4 24
ADDRGP4 _atof
CALLF4
ASGNF4
ADDRLP4 4
INDIRP4
INDIRP4
ADDRLP4 24
INDIRF4
ASGNF4
line 905
;905:            break;
LABELV $377
LABELV $378
line 907
;906:        }
;907:        arg++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 908
;908:    }
LABELV $373
line 885
ADDRFP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $372
line 910
;909:
;910:    return count;
ADDRLP4 8
INDIRI4
RETI4
LABELV $370
endproc sscanf 28 4
import memcpy
import memset
lit
align 1
LABELV $314
byte 1 40
byte 1 110
byte 1 117
byte 1 108
byte 1 108
byte 1 41
byte 1 0
