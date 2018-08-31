code
proc swapfunc 24 0
file "vm_stdlib.c"
line 46
;1:#include "vm_stdlib.h"
;2:
;3:
;4:#ifndef _MSC_VER
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
;21:
;22:
;23:/*
;24: * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
;25: */
;26:#define swapcode(TYPE, parmi, parmj, n)                                        \
;27:    {                                                                          \
;28:        long           i  = (n) / sizeof(TYPE);                                \
;29:        register TYPE* pi = (TYPE*)(parmi);                                    \
;30:        register TYPE* pj = (TYPE*)(parmj);                                    \
;31:        do                                                                     \
;32:        {                                                                      \
;33:            register TYPE t = *pi;                                             \
;34:            *pi++           = *pj;                                             \
;35:            *pj++           = t;                                               \
;36:        } while (--i > 0);                                                     \
;37:    }
;38:
;39:#define SWAPINIT(a, es)                                                        \
;40:    swaptype = ((char*)a - (char*)0) % sizeof(long) || es % sizeof(long)       \
;41:                   ? 2                                                         \
;42:                   : es == sizeof(long) ? 0 : 1;
;43:
;44:static void swapfunc(a, b, n, swaptype) char *a, *b;
;45:int         n, swaptype;
;46:{
line 47
;47:    if (swaptype <= 1)
ADDRFP4 12
INDIRI4
CNSTI4 1
GTI4 $2
line 48
;48:        swapcode(long, a, b, n) else swapcode(char, a, b, n)
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
line 49
;49:}
LABELV $1
endproc swapfunc 24 0
proc med3 40 8
line 67
;50:
;51:#define swap(a, b)                                                             \
;52:    if (swaptype == 0)                                                         \
;53:    {                                                                          \
;54:        long t      = *(long*)(a);                                             \
;55:        *(long*)(a) = *(long*)(b);                                             \
;56:        *(long*)(b) = t;                                                       \
;57:    }                                                                          \
;58:    else                                                                       \
;59:        swapfunc(a, b, es, swaptype)
;60:
;61:#define vecswap(a, b, n)                                                       \
;62:    if ((n) > 0)                                                               \
;63:    swapfunc(a, b, n, swaptype)
;64:
;65:static char *med3(a, b, c, cmp) char *a, *b, *c;
;66:cmp_t*       cmp;
;67:{
line 68
;68:    return cmp(a, b) < 0 ? (cmp(b, c) < 0 ? b : (cmp(a, c) < 0 ? c : a))
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
line 75
;69:                         : (cmp(b, c) > 0 ? b : (cmp(a, c) < 0 ? a : c));
;70:}
;71:
;72:void   qsort(a, n, es, cmp) void* a;
;73:gsize_t n, es;
;74:cmp_t* cmp;
;75:{
LABELV $27
line 80
;76:    char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
;77:    int   d, r, swaptype, swap_cnt;
;78:
;79:loop:
;80:    SWAPINIT(a, es);
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
line 81
;81:    swap_cnt = 0;
ADDRLP4 28
CNSTI4 0
ASGNI4
line 82
;82:    if (n < 7)
ADDRFP4 4
INDIRI4
CNSTI4 7
GEI4 $35
line 83
;83:    {
line 84
;84:        for (pm = (char*)a + es; pm < (char*)a + n * es; pm += es)
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
line 85
;85:            for (pl = pm; pl > (char*)a && cmp(pl - es, pl) > 0; pl -= es)
ADDRLP4 0
ADDRLP4 32
INDIRP4
ASGNP4
ADDRGP4 $44
JUMPV
LABELV $41
line 86
;86:                swap(pl, pl - es);
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
line 85
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
line 84
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
line 87
;87:        return;
ADDRGP4 $26
JUMPV
LABELV $35
line 89
;88:    }
;89:    pm = (char*)a + (n / 2) * es;
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
line 90
;90:    if (n > 7)
ADDRFP4 4
INDIRI4
CNSTI4 7
LEI4 $48
line 91
;91:    {
line 92
;92:        pl = a;
ADDRLP4 0
ADDRFP4 0
INDIRP4
ASGNP4
line 93
;93:        pn = (char*)a + (n - 1) * es;
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
line 94
;94:        if (n > 40)
ADDRFP4 4
INDIRI4
CNSTI4 40
LEI4 $50
line 95
;95:        {
line 96
;96:            d  = (n / 8) * es;
ADDRLP4 40
ADDRFP4 4
INDIRI4
CNSTI4 8
DIVI4
ADDRFP4 8
INDIRI4
MULI4
ASGNI4
line 97
;97:            pl = med3(pl, pl + d, pl + 2 * d, cmp);
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
line 98
;98:            pm = med3(pm - d, pm, pm + d, cmp);
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
line 99
;99:            pn = med3(pn - 2 * d, pn - d, pn, cmp);
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
line 100
;100:        }
LABELV $50
line 101
;101:        pm = med3(pl, pm, pn, cmp);
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
line 102
;102:    }
LABELV $48
line 103
;103:    swap(a, pm);
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
line 104
;104:    pa = pb = (char*)a + es;
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
line 106
;105:
;106:    pc = pd = (char*)a + (n - 1) * es;
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
line 107
;107:    for (;;)
line 108
;108:    {
ADDRGP4 $59
JUMPV
LABELV $58
line 110
;109:        while (pb <= pc && (r = cmp(pb, a)) <= 0)
;110:        {
line 111
;111:            if (r == 0)
ADDRLP4 12
INDIRI4
CNSTI4 0
NEI4 $61
line 112
;112:            {
line 113
;113:                swap_cnt = 1;
ADDRLP4 28
CNSTI4 1
ASGNI4
line 114
;114:                swap(pa, pb);
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
line 115
;115:                pa += es;
ADDRLP4 20
ADDRFP4 8
INDIRI4
ADDRLP4 20
INDIRP4
ADDP4
ASGNP4
line 116
;116:            }
LABELV $61
line 117
;117:            pb += es;
ADDRLP4 4
ADDRFP4 8
INDIRI4
ADDRLP4 4
INDIRP4
ADDP4
ASGNP4
line 118
;118:        }
LABELV $59
line 109
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
line 120
;119:        while (pb <= pc && (r = cmp(pc, a)) >= 0)
;120:        {
line 121
;121:            if (r == 0)
ADDRLP4 12
INDIRI4
CNSTI4 0
NEI4 $69
line 122
;122:            {
line 123
;123:                swap_cnt = 1;
ADDRLP4 28
CNSTI4 1
ASGNI4
line 124
;124:                swap(pc, pd);
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
line 125
;125:                pd -= es;
ADDRLP4 24
ADDRLP4 24
INDIRP4
ADDRFP4 8
INDIRI4
SUBP4
ASGNP4
line 126
;126:            }
LABELV $69
line 127
;127:            pc -= es;
ADDRLP4 8
ADDRLP4 8
INDIRP4
ADDRFP4 8
INDIRI4
SUBP4
ASGNP4
line 128
;128:        }
LABELV $67
line 119
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
line 129
;129:        if (pb > pc)
ADDRLP4 4
INDIRP4
CVPU4 4
ADDRLP4 8
INDIRP4
CVPU4 4
LEU4 $74
line 130
;130:            break;
ADDRGP4 $56
JUMPV
LABELV $74
line 131
;131:        swap(pb, pc);
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
line 132
;132:        swap_cnt = 1;
ADDRLP4 28
CNSTI4 1
ASGNI4
line 133
;133:        pb += es;
ADDRLP4 4
ADDRFP4 8
INDIRI4
ADDRLP4 4
INDIRP4
ADDP4
ASGNP4
line 134
;134:        pc -= es;
ADDRLP4 8
ADDRLP4 8
INDIRP4
ADDRFP4 8
INDIRI4
SUBP4
ASGNP4
line 135
;135:    }
line 107
ADDRGP4 $59
JUMPV
LABELV $56
line 136
;136:    if (swap_cnt == 0)
ADDRLP4 28
INDIRI4
CNSTI4 0
NEI4 $78
line 137
;137:    { /* Switch to insertion sort */
line 138
;138:        for (pm = (char*)a + es; pm < (char*)a + n * es; pm += es)
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
line 139
;139:            for (pl = pm; pl > (char*)a && cmp(pl - es, pl) > 0; pl -= es)
ADDRLP4 0
ADDRLP4 32
INDIRP4
ASGNP4
ADDRGP4 $87
JUMPV
LABELV $84
line 140
;140:                swap(pl, pl - es);
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
line 139
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
line 138
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
line 141
;141:        return;
ADDRGP4 $26
JUMPV
LABELV $78
line 144
;142:    }
;143:
;144:    pn = (char*)a + n * es;
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
line 145
;145:    r  = min(pa - (char*)a, pb - pa);
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
line 146
;146:    vecswap(a, pb - r, r);
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
line 147
;147:    r = min(pd - pc, pn - pd - es);
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
line 148
;148:    vecswap(pb, pn - r, r);
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
line 149
;149:    if ((r = pb - pa) > es)
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
line 150
;150:        qsort(a, r / es, es, cmp);
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
line 151
;151:    if ((r = pd - pc) > es)
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
line 152
;152:    {
line 154
;153:        /* Iterate rather than recurse to save stack space */
;154:        a = pn - r;
ADDRFP4 0
ADDRLP4 36
INDIRP4
ADDRLP4 12
INDIRI4
SUBP4
ASGNP4
line 155
;155:        n = r / es;
ADDRFP4 4
ADDRLP4 12
INDIRI4
ADDRFP4 8
INDIRI4
DIVI4
ASGNI4
line 156
;156:        goto loop;
ADDRGP4 $27
JUMPV
LABELV $103
line 159
;157:    }
;158:    /*      qsort(pn - r, r / es, es, cmp);*/
;159:}
LABELV $26
endproc qsort 104 16
export strlen
proc strlen 0 0
endproc strlen 0 0
export strcat
proc strcat 0 0
endproc strcat 0 0
export strcpy
proc strcpy 0 0
endproc strcpy 0 0
export strcmp
proc strcmp 0 0
endproc strcmp 0 0
export strchr
proc strchr 0 0
endproc strchr 0 0
export strstr
proc strstr 0 0
endproc strstr 0 0
export tolower
proc tolower 0 0
endproc tolower 0 0
export toupper
proc toupper 0 0
endproc toupper 0 0
export memmove
proc memmove 0 0
endproc memmove 0 0
data
align 4
LABELV randSeed
byte 4 0
export srand
code
proc srand 0 0
endproc srand 0 0
export rand
proc rand 0 0
endproc rand 0 0
export atof
proc atof 0 0
endproc atof 0 0
export _atof
proc _atof 0 0
endproc _atof 0 0
export atoi
proc atoi 0 0
endproc atoi 0 0
export _atoi
proc _atoi 0 0
endproc _atoi 0 0
export abs
proc abs 0 0
endproc abs 0 0
export fabs
proc fabs 0 0
endproc fabs 0 0
export AddInt
proc AddInt 0 0
endproc AddInt 0 0
export AddFloat
proc AddFloat 0 0
endproc AddFloat 0 0
export AddString
proc AddString 0 0
endproc AddString 0 0
export vsprintf
proc vsprintf 0 0
endproc vsprintf 0 0
export sscanf
proc sscanf 0 0
endproc sscanf 0 0
bss
export size_t
align 4
LABELV size_t
skip 4
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
