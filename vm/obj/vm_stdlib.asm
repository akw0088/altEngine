export strlen
code
proc strlen 4 0
file "vm_stdlib.c"
line 11
;1:#include "vm_stdlib.h"
;2:
;3:
;4:#ifndef _MSC_VER
;5:#ifndef NULL
;6:#define NULL ((void*)0)
;7:#endif
;8:
;9:
;10:size_t strlen(const char *str)
;11:{
line 14
;12:    const char *s;
;13:
;14:    s = str;
ADDRLP4 0
ADDRFP4 0
INDIRP4
ASGNP4
ADDRGP4 $3
JUMPV
LABELV $2
line 16
;15:    while (*s)
;16:    {
line 17
;17:        s++;
ADDRLP4 0
ADDRLP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 18
;18:    }
LABELV $3
line 15
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $2
line 19
;19:    return s - str;
ADDRLP4 0
INDIRP4
CVPU4 4
ADDRFP4 0
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
RETI4
LABELV $1
endproc strlen 4 0
export strcat
proc strcat 16 0
line 23
;20:}
;21:
;22:char *strcat(char *strDestination, const char *strSource)
;23:{
line 26
;24:    char *s;
;25:
;26:    s = strDestination;
ADDRLP4 0
ADDRFP4 0
INDIRP4
ASGNP4
ADDRGP4 $7
JUMPV
LABELV $6
line 28
;27:    while (*s)
;28:    {
line 29
;29:        s++;
ADDRLP4 0
ADDRLP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 30
;30:    }
LABELV $7
line 27
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $6
ADDRGP4 $10
JUMPV
LABELV $9
line 32
;31:    while (*strSource)
;32:    {
line 33
;33:        *s++ = *strSource++;
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
line 34
;34:    }
LABELV $10
line 31
ADDRFP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $9
line 35
;35:    *s = 0;
ADDRLP4 0
INDIRP4
CNSTI1 0
ASGNI1
line 36
;36:    return strDestination;
ADDRFP4 0
INDIRP4
RETP4
LABELV $5
endproc strcat 16 0
export strcpy
proc strcpy 16 0
line 40
;37:}
;38:
;39:char *strcpy(char *strDestination, const char *strSource)
;40:{
line 43
;41:    char *s;
;42:
;43:    s = strDestination;
ADDRLP4 0
ADDRFP4 0
INDIRP4
ASGNP4
ADDRGP4 $14
JUMPV
LABELV $13
line 45
;44:    while (*strSource)
;45:    {
line 46
;46:        *s++ = *strSource++;
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
line 47
;47:    }
LABELV $14
line 44
ADDRFP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $13
line 48
;48:    *s = 0;
ADDRLP4 0
INDIRP4
CNSTI1 0
ASGNI1
line 49
;49:    return strDestination;
ADDRFP4 0
INDIRP4
RETP4
LABELV $12
endproc strcpy 16 0
export strcmp
proc strcmp 12 0
line 53
;50:}
;51:
;52:int strcmp(const char *string1, const char *string2)
;53:{
ADDRGP4 $18
JUMPV
LABELV $17
line 55
;54:    while (*string1 == *string2 && *string1 && *string2)
;55:    {
line 56
;56:        string1++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 57
;57:        string2++;
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 58
;58:    }
LABELV $18
line 54
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
NEI4 $21
ADDRLP4 8
CNSTI4 0
ASGNI4
ADDRLP4 0
INDIRI4
ADDRLP4 8
INDIRI4
EQI4 $21
ADDRLP4 4
INDIRI4
ADDRLP4 8
INDIRI4
NEI4 $17
LABELV $21
line 59
;59:    return *string1 - *string2;
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
LABELV $16
endproc strcmp 12 0
export strchr
proc strchr 0 0
line 63
;60:}
;61:
;62:char *strchr(const char *string, int c)
;63:{
ADDRGP4 $24
JUMPV
LABELV $23
line 65
;64:    while (*string)
;65:    {
line 66
;66:        if (*string == c)
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
ADDRFP4 4
INDIRI4
NEI4 $26
line 67
;67:        {
line 68
;68:            return (char*)string;
ADDRFP4 0
INDIRP4
RETP4
ADDRGP4 $22
JUMPV
LABELV $26
line 70
;69:        }
;70:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 71
;71:    }
LABELV $24
line 64
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $23
line 72
;72:    return (char*)0;
CNSTP4 0
RETP4
LABELV $22
endproc strchr 0 0
export strstr
proc strstr 8 0
line 76
;73:}
;74:
;75:char *strstr(const char *string, const char *strCharSet)
;76:{
ADDRGP4 $30
JUMPV
LABELV $29
line 78
;77:    while (*string)
;78:    {
line 81
;79:        int i;
;80:
;81:        for (i = 0; strCharSet[i]; i++)
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $35
JUMPV
LABELV $32
line 82
;82:        {
line 83
;83:            if (string[i] != strCharSet[i])
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
EQI4 $36
line 84
;84:            {
line 85
;85:                break;
ADDRGP4 $34
JUMPV
LABELV $36
line 87
;86:            }
;87:        }
LABELV $33
line 81
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $35
ADDRLP4 0
INDIRI4
ADDRFP4 4
INDIRP4
ADDP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $32
LABELV $34
line 88
;88:        if (!strCharSet[i])
ADDRLP4 0
INDIRI4
ADDRFP4 4
INDIRP4
ADDP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $38
line 89
;89:        {
line 90
;90:            return (char*)string;
ADDRFP4 0
INDIRP4
RETP4
ADDRGP4 $28
JUMPV
LABELV $38
line 92
;91:        }
;92:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 93
;93:    }
LABELV $30
line 77
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $29
line 94
;94:    return (char*)0;
CNSTP4 0
RETP4
LABELV $28
endproc strstr 8 0
export tolower
proc tolower 4 0
line 98
;95:}
;96:
;97:int tolower(int c)
;98:{
line 99
;99:    if (c >= 'A' && c <= 'Z')
ADDRLP4 0
ADDRFP4 0
INDIRI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 65
LTI4 $41
ADDRLP4 0
INDIRI4
CNSTI4 90
GTI4 $41
line 100
;100:    {
line 101
;101:        c += 'a' - 'A';
ADDRFP4 0
ADDRFP4 0
INDIRI4
CNSTI4 32
ADDI4
ASGNI4
line 102
;102:    }
LABELV $41
line 103
;103:    return c;
ADDRFP4 0
INDIRI4
RETI4
LABELV $40
endproc tolower 4 0
export toupper
proc toupper 4 0
line 107
;104:}
;105:
;106:int toupper(int c)
;107:{
line 108
;108:    if (c >= 'a' && c <= 'z')
ADDRLP4 0
ADDRFP4 0
INDIRI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 97
LTI4 $44
ADDRLP4 0
INDIRI4
CNSTI4 122
GTI4 $44
line 109
;109:    {
line 110
;110:        c += 'A' - 'a';
ADDRFP4 0
ADDRFP4 0
INDIRI4
CNSTI4 -32
ADDI4
ASGNI4
line 111
;111:    }
LABELV $44
line 112
;112:    return c;
ADDRFP4 0
INDIRI4
RETI4
LABELV $43
endproc toupper 4 0
export memmove
proc memmove 8 0
line 116
;113:}
;114:
;115:void *memmove(void *dest, const void *src, size_t count)
;116:{
line 119
;117:    int i;
;118:
;119:    if (dest > src)
ADDRFP4 0
INDIRP4
CVPU4 4
ADDRFP4 4
INDIRP4
CVPU4 4
LEU4 $47
line 120
;120:    {
line 121
;121:        for (i = count - 1; i >= 0; i--)
ADDRLP4 0
ADDRFP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
ADDRGP4 $52
JUMPV
LABELV $49
line 122
;122:        {
line 123
;123:            ((char*)dest)[i] = ((char*)src)[i];
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
line 124
;124:        }
LABELV $50
line 121
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
LABELV $52
ADDRLP4 0
INDIRI4
CNSTI4 0
GEI4 $49
line 125
;125:    }
ADDRGP4 $48
JUMPV
LABELV $47
line 127
;126:    else
;127:    {
line 128
;128:        for (i = 0; i < count; i++)
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $56
JUMPV
LABELV $53
line 129
;129:        {
line 130
;130:            ((char*)dest)[i] = ((char*)src)[i];
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
line 131
;131:        }
LABELV $54
line 128
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $56
ADDRLP4 0
INDIRI4
ADDRFP4 8
INDIRI4
LTI4 $53
line 132
;132:    }
LABELV $48
line 133
;133:    return dest;
ADDRFP4 0
INDIRP4
RETP4
LABELV $46
endproc memmove 8 0
data
align 4
LABELV randSeed
byte 4 0
export srand
code
proc srand 0 0
line 139
;134:}
;135:
;136:static int randSeed = 0;
;137:
;138:void srand(unsigned seed)
;139:{
line 140
;140:    randSeed = seed;
ADDRGP4 randSeed
ADDRFP4 0
INDIRU4
CVUI4 4
ASGNI4
line 141
;141:}
LABELV $57
endproc srand 0 0
export rand
proc rand 4 0
line 144
;142:
;143:int rand(void)
;144:{
line 145
;145:    randSeed = (69069 * randSeed + 1);
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
line 146
;146:    return randSeed & 0x7fff;
ADDRGP4 randSeed
INDIRI4
CNSTI4 32767
BANDI4
RETI4
LABELV $58
endproc rand 4 0
export atof
proc atof 32 0
line 150
;147:}
;148:
;149:double atof(const char *string)
;150:{
ADDRGP4 $61
JUMPV
LABELV $60
line 157
;151:    float sign;
;152:    float value;
;153:    int   c;
;154:
;155:    // skip whitespace
;156:    while (*string <= ' ')
;157:    {
line 158
;158:        if (!*string)
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $63
line 159
;159:        {
line 160
;160:            return 0;
CNSTF4 0
RETF4
ADDRGP4 $59
JUMPV
LABELV $63
line 162
;161:        }
;162:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 163
;163:    }
LABELV $61
line 156
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 32
LEI4 $60
line 166
;164:
;165:    // check sign
;166:    switch (*string)
ADDRLP4 12
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 12
INDIRI4
CNSTI4 43
EQI4 $68
ADDRLP4 12
INDIRI4
CNSTI4 45
EQI4 $69
ADDRGP4 $65
JUMPV
line 167
;167:    {
LABELV $68
line 169
;168:    case '+':
;169:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 170
;170:        sign = 1;
ADDRLP4 8
CNSTF4 1065353216
ASGNF4
line 171
;171:        break;
ADDRGP4 $66
JUMPV
LABELV $69
line 173
;172:    case '-':
;173:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 174
;174:        sign = -1;
ADDRLP4 8
CNSTF4 3212836864
ASGNF4
line 175
;175:        break;
ADDRGP4 $66
JUMPV
LABELV $65
line 177
;176:    default:
;177:        sign = 1;
ADDRLP4 8
CNSTF4 1065353216
ASGNF4
line 178
;178:        break;
LABELV $66
line 182
;179:    }
;180:
;181:    // read digits
;182:    value = 0;
ADDRLP4 4
CNSTF4 0
ASGNF4
line 183
;183:    c     = string[0];
ADDRLP4 0
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
ASGNI4
line 184
;184:    if (c != '.')
ADDRLP4 0
INDIRI4
CNSTI4 46
EQI4 $70
line 185
;185:    {
LABELV $72
line 187
;186:        do
;187:        {
line 188
;188:            c = *string++;
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
line 189
;189:            if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $77
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $75
LABELV $77
line 190
;190:            {
line 191
;191:                break;
ADDRGP4 $71
JUMPV
LABELV $75
line 193
;192:            }
;193:            c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 194
;194:            value = value * 10 + c;
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
line 195
;195:        } while (1);
LABELV $73
ADDRGP4 $72
JUMPV
line 196
;196:    }
ADDRGP4 $71
JUMPV
LABELV $70
line 198
;197:    else
;198:    {
line 199
;199:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 200
;200:    }
LABELV $71
line 203
;201:
;202:    // check for decimal point
;203:    if (c == '.')
ADDRLP4 0
INDIRI4
CNSTI4 46
NEI4 $78
line 204
;204:    {
line 207
;205:        double fraction;
;206:
;207:        fraction = 0.1;
ADDRLP4 20
CNSTF4 1036831949
ASGNF4
LABELV $80
line 209
;208:        do
;209:        {
line 210
;210:            c = *string++;
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
line 211
;211:            if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $85
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $83
LABELV $85
line 212
;212:            {
line 213
;213:                break;
ADDRGP4 $82
JUMPV
LABELV $83
line 215
;214:            }
;215:            c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 216
;216:            value += c * fraction;
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
line 217
;217:            fraction *= 0.1;
ADDRLP4 20
CNSTF4 1036831949
ADDRLP4 20
INDIRF4
MULF4
ASGNF4
line 218
;218:        } while (1);
LABELV $81
ADDRGP4 $80
JUMPV
LABELV $82
line 219
;219:    }
LABELV $78
line 223
;220:
;221:    // not handling 10e10 notation...
;222:
;223:    return value * sign;
ADDRLP4 4
INDIRF4
ADDRLP4 8
INDIRF4
MULF4
RETF4
LABELV $59
endproc atof 32 0
export _atof
proc _atof 36 0
line 227
;224:}
;225:
;226:double _atof(const char **stringPtr)
;227:{
line 231
;228:    const char* string;
;229:    float       sign;
;230:    float       value;
;231:    int         c = '0'; // uninitialized use possible
ADDRLP4 0
CNSTI4 48
ASGNI4
line 233
;232:
;233:    string = *stringPtr;
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
ADDRGP4 $88
JUMPV
LABELV $87
line 237
;234:
;235:    // skip whitespace
;236:    while (*string <= ' ')
;237:    {
line 238
;238:        if (!*string)
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $90
line 239
;239:        {
line 240
;240:            *stringPtr = string;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 241
;241:            return 0;
CNSTF4 0
RETF4
ADDRGP4 $86
JUMPV
LABELV $90
line 243
;242:        }
;243:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 244
;244:    }
LABELV $88
line 236
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 32
LEI4 $87
line 247
;245:
;246:    // check sign
;247:    switch (*string)
ADDRLP4 16
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 16
INDIRI4
CNSTI4 43
EQI4 $95
ADDRLP4 16
INDIRI4
CNSTI4 45
EQI4 $96
ADDRGP4 $92
JUMPV
line 248
;248:    {
LABELV $95
line 250
;249:    case '+':
;250:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 251
;251:        sign = 1;
ADDRLP4 12
CNSTF4 1065353216
ASGNF4
line 252
;252:        break;
ADDRGP4 $93
JUMPV
LABELV $96
line 254
;253:    case '-':
;254:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 255
;255:        sign = -1;
ADDRLP4 12
CNSTF4 3212836864
ASGNF4
line 256
;256:        break;
ADDRGP4 $93
JUMPV
LABELV $92
line 258
;257:    default:
;258:        sign = 1;
ADDRLP4 12
CNSTF4 1065353216
ASGNF4
line 259
;259:        break;
LABELV $93
line 263
;260:    }
;261:
;262:    // read digits
;263:    value = 0;
ADDRLP4 8
CNSTF4 0
ASGNF4
line 264
;264:    if (string[0] != '.')
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 46
EQI4 $97
line 265
;265:    {
LABELV $99
line 267
;266:        do
;267:        {
line 268
;268:            c = *string++;
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
line 269
;269:            if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $104
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $102
LABELV $104
line 270
;270:            {
line 271
;271:                break;
ADDRGP4 $101
JUMPV
LABELV $102
line 273
;272:            }
;273:            c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 274
;274:            value = value * 10 + c;
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
line 275
;275:        } while (1);
LABELV $100
ADDRGP4 $99
JUMPV
LABELV $101
line 276
;276:    }
LABELV $97
line 279
;277:
;278:    // check for decimal point
;279:    if (c == '.')
ADDRLP4 0
INDIRI4
CNSTI4 46
NEI4 $105
line 280
;280:    {
line 283
;281:        double fraction;
;282:
;283:        fraction = 0.1;
ADDRLP4 24
CNSTF4 1036831949
ASGNF4
LABELV $107
line 285
;284:        do
;285:        {
line 286
;286:            c = *string++;
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
line 287
;287:            if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $112
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $110
LABELV $112
line 288
;288:            {
line 289
;289:                break;
ADDRGP4 $109
JUMPV
LABELV $110
line 291
;290:            }
;291:            c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 292
;292:            value += c * fraction;
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
line 293
;293:            fraction *= 0.1;
ADDRLP4 24
CNSTF4 1036831949
ADDRLP4 24
INDIRF4
MULF4
ASGNF4
line 294
;294:        } while (1);
LABELV $108
ADDRGP4 $107
JUMPV
LABELV $109
line 295
;295:    }
LABELV $105
line 298
;296:
;297:    // not handling 10e10 notation...
;298:    *stringPtr = string;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 300
;299:
;300:    return value * sign;
ADDRLP4 8
INDIRF4
ADDRLP4 12
INDIRF4
MULF4
RETF4
LABELV $86
endproc _atof 36 0
export atoi
proc atoi 28 0
line 304
;301:}
;302:
;303:int atoi(const char *str)
;304:{
ADDRGP4 $115
JUMPV
LABELV $114
line 311
;305:    int sign;
;306:    int value;
;307:    int c;
;308:
;309:    // skip whitespace
;310:    while (*str <= ' ')
;311:    {
line 312
;312:        if (!*str)
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $117
line 313
;313:        {
line 314
;314:            return 0;
CNSTI4 0
RETI4
ADDRGP4 $113
JUMPV
LABELV $117
line 316
;315:        }
;316:		str++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 317
;317:    }
LABELV $115
line 310
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 32
LEI4 $114
line 320
;318:
;319:    // check sign
;320:    switch (*str)
ADDRLP4 12
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 12
INDIRI4
CNSTI4 43
EQI4 $122
ADDRLP4 12
INDIRI4
CNSTI4 45
EQI4 $123
ADDRGP4 $119
JUMPV
line 321
;321:    {
LABELV $122
line 323
;322:    case '+':
;323:		str++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 324
;324:        sign = 1;
ADDRLP4 8
CNSTI4 1
ASGNI4
line 325
;325:        break;
ADDRGP4 $120
JUMPV
LABELV $123
line 327
;326:    case '-':
;327:		str++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 328
;328:        sign = -1;
ADDRLP4 8
CNSTI4 -1
ASGNI4
line 329
;329:        break;
ADDRGP4 $120
JUMPV
LABELV $119
line 331
;330:    default:
;331:        sign = 1;
ADDRLP4 8
CNSTI4 1
ASGNI4
line 332
;332:        break;
LABELV $120
line 336
;333:    }
;334:
;335:    // read digits
;336:    value = 0;
ADDRLP4 4
CNSTI4 0
ASGNI4
LABELV $124
line 338
;337:    do
;338:    {
line 339
;339:        c = *str++;
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
line 340
;340:        if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $129
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $127
LABELV $129
line 341
;341:        {
line 342
;342:            break;
ADDRGP4 $126
JUMPV
LABELV $127
line 344
;343:        }
;344:        c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 345
;345:        value = value * 10 + c;
ADDRLP4 4
CNSTI4 10
ADDRLP4 4
INDIRI4
MULI4
ADDRLP4 0
INDIRI4
ADDI4
ASGNI4
line 346
;346:    } while (1);
LABELV $125
ADDRGP4 $124
JUMPV
LABELV $126
line 350
;347:
;348:    // not handling 10e10 notation...
;349:
;350:    return value * sign;
ADDRLP4 4
INDIRI4
ADDRLP4 8
INDIRI4
MULI4
RETI4
LABELV $113
endproc atoi 28 0
export _atoi
proc _atoi 32 0
line 354
;351:}
;352:
;353:int _atoi(const char **stringPtr)
;354:{
line 360
;355:    int         sign;
;356:    int         value;
;357:    int         c;
;358:    const char *str;
;359:
;360:	str = *stringPtr;
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
ADDRGP4 $132
JUMPV
LABELV $131
line 364
;361:
;362:    // skip whitespace
;363:    while (*str <= ' ')
;364:    {
line 365
;365:        if (!*str)
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $134
line 366
;366:        {
line 367
;367:            return 0;
CNSTI4 0
RETI4
ADDRGP4 $130
JUMPV
LABELV $134
line 369
;368:        }
;369:        str++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 370
;370:    }
LABELV $132
line 363
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 32
LEI4 $131
line 373
;371:
;372:    // check sign
;373:    switch (*str)
ADDRLP4 16
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 16
INDIRI4
CNSTI4 43
EQI4 $139
ADDRLP4 16
INDIRI4
CNSTI4 45
EQI4 $140
ADDRGP4 $136
JUMPV
line 374
;374:    {
LABELV $139
line 376
;375:    case '+':
;376:		str++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 377
;377:        sign = 1;
ADDRLP4 12
CNSTI4 1
ASGNI4
line 378
;378:        break;
ADDRGP4 $137
JUMPV
LABELV $140
line 380
;379:    case '-':
;380:		str++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 381
;381:        sign = -1;
ADDRLP4 12
CNSTI4 -1
ASGNI4
line 382
;382:        break;
ADDRGP4 $137
JUMPV
LABELV $136
line 384
;383:    default:
;384:        sign = 1;
ADDRLP4 12
CNSTI4 1
ASGNI4
line 385
;385:        break;
LABELV $137
line 389
;386:    }
;387:
;388:    // read digits
;389:    value = 0;
ADDRLP4 8
CNSTI4 0
ASGNI4
LABELV $141
line 391
;390:    do
;391:    {
line 392
;392:        c = *str++;
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
line 393
;393:        if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $146
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $144
LABELV $146
line 394
;394:        {
line 395
;395:            break;
ADDRGP4 $143
JUMPV
LABELV $144
line 397
;396:        }
;397:        c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 398
;398:        value = value * 10 + c;
ADDRLP4 8
CNSTI4 10
ADDRLP4 8
INDIRI4
MULI4
ADDRLP4 0
INDIRI4
ADDI4
ASGNI4
line 399
;399:    } while (1);
LABELV $142
ADDRGP4 $141
JUMPV
LABELV $143
line 403
;400:
;401:    // not handling 10e10 notation...
;402:
;403:    *stringPtr = str;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 405
;404:
;405:    return value * sign;
ADDRLP4 8
INDIRI4
ADDRLP4 12
INDIRI4
MULI4
RETI4
LABELV $130
endproc _atoi 32 0
export abs
proc abs 4 0
line 409
;406:}
;407:
;408:int abs(int n)
;409:{
line 410
;410:    return n < 0 ? -n : n;
ADDRFP4 0
INDIRI4
CNSTI4 0
GEI4 $149
ADDRLP4 0
ADDRFP4 0
INDIRI4
NEGI4
ASGNI4
ADDRGP4 $150
JUMPV
LABELV $149
ADDRLP4 0
ADDRFP4 0
INDIRI4
ASGNI4
LABELV $150
ADDRLP4 0
INDIRI4
RETI4
LABELV $147
endproc abs 4 0
export fabs
proc fabs 4 0
line 414
;411:}
;412:
;413:double fabs(double x)
;414:{
line 415
;415:    return x < 0 ? -x : x;
ADDRFP4 0
INDIRF4
CNSTF4 0
GEF4 $153
ADDRLP4 0
ADDRFP4 0
INDIRF4
NEGF4
ASGNF4
ADDRGP4 $154
JUMPV
LABELV $153
ADDRLP4 0
ADDRFP4 0
INDIRF4
ASGNF4
LABELV $154
ADDRLP4 0
INDIRF4
RETF4
LABELV $151
endproc fabs 4 0
export AddInt
proc AddInt 56 0
line 434
;416:}
;417:
;418:
;419:#define ALT 0x00000001       /* alternate form */
;420:#define HEXPREFIX 0x00000002 /* add 0x or 0X prefix */
;421:#define LADJUST 0x00000004   /* left adjustment */
;422:#define LONGDBL 0x00000008   /* long double */
;423:#define LONGINT 0x00000010   /* long integer */
;424:#define QUADINT 0x00000020   /* quad integer */
;425:#define SHORTINT 0x00000040  /* short integer */
;426:#define ZEROPAD 0x00000080   /* zero (as opposed to blank) pad */
;427:#define FPT 0x00000100       /* floating point number */
;428:
;429:#define to_digit(c) ((c) - '0')
;430:#define is_digit(c) ((unsigned)to_digit(c) <= 9)
;431:#define to_char(n) ((n) + '0')
;432:
;433:void AddInt(char** buf_p, int val, int width, int flags)
;434:{
line 440
;435:    char  text[32];
;436:    int   digits;
;437:    int   signedVal;
;438:    char* buf;
;439:
;440:    digits    = 0;
ADDRLP4 0
CNSTI4 0
ASGNI4
line 441
;441:    signedVal = val;
ADDRLP4 40
ADDRFP4 4
INDIRI4
ASGNI4
line 442
;442:    if (val < 0)
ADDRFP4 4
INDIRI4
CNSTI4 0
GEI4 $156
line 443
;443:    {
line 444
;444:        val = -val;
ADDRFP4 4
ADDRFP4 4
INDIRI4
NEGI4
ASGNI4
line 445
;445:    }
LABELV $156
LABELV $158
line 447
;446:    do
;447:    {
line 448
;448:        text[digits++] = '0' + val % 10;
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
line 449
;449:        val /= 10;
ADDRFP4 4
ADDRFP4 4
INDIRI4
CNSTI4 10
DIVI4
ASGNI4
line 450
;450:    } while (val);
LABELV $159
ADDRFP4 4
INDIRI4
CNSTI4 0
NEI4 $158
line 452
;451:
;452:    if (signedVal < 0)
ADDRLP4 40
INDIRI4
CNSTI4 0
GEI4 $161
line 453
;453:    {
line 454
;454:        text[digits++] = '-';
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
line 455
;455:    }
LABELV $161
line 457
;456:
;457:    buf = *buf_p;
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
line 459
;458:
;459:    if (!(flags & LADJUST))
ADDRFP4 12
INDIRI4
CNSTI4 4
BANDI4
CNSTI4 0
NEI4 $172
line 460
;460:    {
ADDRGP4 $166
JUMPV
LABELV $165
line 462
;461:        while (digits < width)
;462:        {
line 463
;463:            *buf++ = (flags & ZEROPAD) ? '0' : ' ';
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
EQI4 $169
ADDRLP4 44
CNSTI4 48
ASGNI4
ADDRGP4 $170
JUMPV
LABELV $169
ADDRLP4 44
CNSTI4 32
ASGNI4
LABELV $170
ADDRLP4 48
INDIRP4
ADDRLP4 44
INDIRI4
CVII1 4
ASGNI1
line 464
;464:            width--;
ADDRFP4 8
ADDRFP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
line 465
;465:        }
LABELV $166
line 461
ADDRLP4 0
INDIRI4
ADDRFP4 8
INDIRI4
LTI4 $165
line 466
;466:    }
ADDRGP4 $172
JUMPV
LABELV $171
line 469
;467:
;468:    while (digits--)
;469:    {
line 470
;470:        *buf++ = text[digits];
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
line 471
;471:        width--;
ADDRFP4 8
ADDRFP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
line 472
;472:    }
LABELV $172
line 468
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
NEI4 $171
line 474
;473:
;474:    if (flags & LADJUST)
ADDRFP4 12
INDIRI4
CNSTI4 4
BANDI4
CNSTI4 0
EQI4 $174
line 475
;475:    {
ADDRGP4 $177
JUMPV
LABELV $176
line 477
;476:        while (width--)
;477:        {
line 478
;478:            *buf++ = (flags & ZEROPAD) ? '0' : ' ';
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
EQI4 $180
ADDRLP4 48
CNSTI4 48
ASGNI4
ADDRGP4 $181
JUMPV
LABELV $180
ADDRLP4 48
CNSTI4 32
ASGNI4
LABELV $181
ADDRLP4 52
INDIRP4
ADDRLP4 48
INDIRI4
CVII1 4
ASGNI1
line 479
;479:        }
LABELV $177
line 476
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
NEI4 $176
line 480
;480:    }
LABELV $174
line 482
;481:
;482:    *buf_p = buf;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 483
;483:}
LABELV $155
endproc AddInt 56 0
export AddFloat
proc AddFloat 60 0
line 486
;484:
;485:void AddFloat(char **buf_p, float fval, int width, int prec)
;486:{
line 494
;487:    char  text[32];
;488:    int   digits;
;489:    float signedVal;
;490:    char* buf;
;491:    int   val;
;492:
;493:    // get the sign
;494:    signedVal = fval;
ADDRLP4 44
ADDRFP4 4
INDIRF4
ASGNF4
line 495
;495:    if (fval < 0)
ADDRFP4 4
INDIRF4
CNSTF4 0
GEF4 $183
line 496
;496:    {
line 497
;497:        fval = -fval;
ADDRFP4 4
ADDRFP4 4
INDIRF4
NEGF4
ASGNF4
line 498
;498:    }
LABELV $183
line 501
;499:
;500:    // write the float number
;501:    digits = 0;
ADDRLP4 0
CNSTI4 0
ASGNI4
line 502
;502:    val    = (int)fval;
ADDRLP4 4
ADDRFP4 4
INDIRF4
CVFI4 4
ASGNI4
LABELV $185
line 504
;503:    do
;504:    {
line 505
;505:        text[digits++] = '0' + val % 10;
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
line 506
;506:        val /= 10;
ADDRLP4 4
ADDRLP4 4
INDIRI4
CNSTI4 10
DIVI4
ASGNI4
line 507
;507:    } while (val);
LABELV $186
ADDRLP4 4
INDIRI4
CNSTI4 0
NEI4 $185
line 509
;508:
;509:    if (signedVal < 0)
ADDRLP4 44
INDIRF4
CNSTF4 0
GEF4 $188
line 510
;510:    {
line 511
;511:        text[digits++] = '-';
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
line 512
;512:    }
LABELV $188
line 514
;513:
;514:    buf = *buf_p;
ADDRLP4 40
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
ADDRGP4 $191
JUMPV
LABELV $190
line 517
;515:
;516:    while (digits < width)
;517:    {
line 518
;518:        *buf++ = ' ';
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
line 519
;519:        width--;
ADDRFP4 8
ADDRFP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
line 520
;520:    }
LABELV $191
line 516
ADDRLP4 0
INDIRI4
ADDRFP4 8
INDIRI4
LTI4 $190
ADDRGP4 $194
JUMPV
LABELV $193
line 523
;521:
;522:    while (digits--)
;523:    {
line 524
;524:        *buf++ = text[digits];
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
line 525
;525:    }
LABELV $194
line 522
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
NEI4 $193
line 527
;526:
;527:    *buf_p = buf;
ADDRFP4 0
INDIRP4
ADDRLP4 40
INDIRP4
ASGNP4
line 529
;528:
;529:    if (prec < 0)
ADDRFP4 12
INDIRI4
CNSTI4 0
GEI4 $196
line 530
;530:        prec = 6;
ADDRFP4 12
CNSTI4 6
ASGNI4
LABELV $196
line 532
;531:    // write the fraction
;532:    digits = 0;
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $199
JUMPV
LABELV $198
line 534
;533:    while (digits < prec)
;534:    {
line 535
;535:        fval -= (int)fval;
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
line 536
;536:        fval *= 10.0;
ADDRFP4 4
CNSTF4 1092616192
ADDRFP4 4
INDIRF4
MULF4
ASGNF4
line 537
;537:        val            = (int)fval;
ADDRLP4 4
ADDRFP4 4
INDIRF4
CVFI4 4
ASGNI4
line 538
;538:        text[digits++] = '0' + val % 10;
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
line 539
;539:    }
LABELV $199
line 533
ADDRLP4 0
INDIRI4
ADDRFP4 12
INDIRI4
LTI4 $198
line 541
;540:
;541:    if (digits > 0)
ADDRLP4 0
INDIRI4
CNSTI4 0
LEI4 $201
line 542
;542:    {
line 543
;543:        buf    = *buf_p;
ADDRLP4 40
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
line 544
;544:        *buf++ = '.';
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
line 545
;545:        for (prec = 0; prec < digits; prec++)
ADDRFP4 12
CNSTI4 0
ASGNI4
ADDRGP4 $206
JUMPV
LABELV $203
line 546
;546:        {
line 547
;547:            *buf++ = text[prec];
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
line 548
;548:        }
LABELV $204
line 545
ADDRFP4 12
ADDRFP4 12
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $206
ADDRFP4 12
INDIRI4
ADDRLP4 0
INDIRI4
LTI4 $203
line 549
;549:        *buf_p = buf;
ADDRFP4 0
INDIRP4
ADDRLP4 40
INDIRP4
ASGNP4
line 550
;550:    }
LABELV $201
line 551
;551:}
LABELV $182
endproc AddFloat 60 0
export AddString
proc AddString 20 4
line 554
;552:
;553:void AddString(char **buf_p, char *string, int width, int prec)
;554:{
line 558
;555:    int   size;
;556:    char *buf;
;557:
;558:    buf = *buf_p;
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
line 560
;559:
;560:    if (string == NULL)
ADDRFP4 4
INDIRP4
CVPU4 4
CNSTU4 0
NEU4 $208
line 561
;561:    {
line 562
;562:        string = "(null)";
ADDRFP4 4
ADDRGP4 $210
ASGNP4
line 563
;563:        prec   = -1;
ADDRFP4 12
CNSTI4 -1
ASGNI4
line 564
;564:    }
LABELV $208
line 566
;565:
;566:    if (prec >= 0)
ADDRFP4 12
INDIRI4
CNSTI4 0
LTI4 $211
line 567
;567:    {
line 568
;568:        for (size = 0; size < prec; size++)
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $216
JUMPV
LABELV $213
line 569
;569:        {
line 570
;570:            if (string[size] == '\0')
ADDRLP4 0
INDIRI4
ADDRFP4 4
INDIRP4
ADDP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $217
line 571
;571:            {
line 572
;572:                break;
ADDRGP4 $212
JUMPV
LABELV $217
line 574
;573:            }
;574:        }
LABELV $214
line 568
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $216
ADDRLP4 0
INDIRI4
ADDRFP4 12
INDIRI4
LTI4 $213
line 575
;575:    }
ADDRGP4 $212
JUMPV
LABELV $211
line 577
;576:    else
;577:    {
line 578
;578:        size = strlen(string);
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
line 579
;579:    }
LABELV $212
line 581
;580:
;581:    width -= size;
ADDRFP4 8
ADDRFP4 8
INDIRI4
ADDRLP4 0
INDIRI4
SUBI4
ASGNI4
ADDRGP4 $220
JUMPV
LABELV $219
line 584
;582:
;583:    while (size--)
;584:    {
line 585
;585:        *buf++ = *string++;
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
line 586
;586:    }
LABELV $220
line 583
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
NEI4 $219
ADDRGP4 $223
JUMPV
LABELV $222
line 589
;587:
;588:    while (width-- > 0)
;589:    {
line 590
;590:        *buf++ = ' ';
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
line 591
;591:    }
LABELV $223
line 588
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
GTI4 $222
line 593
;592:
;593:    *buf_p = buf;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 594
;594:}
LABELV $207
endproc AddString 20 4
export vsprintf
proc vsprintf 72 16
line 605
;595:
;596:/*
;597:vsprintf
;598:
;599:I'm not going to support a bunch of the more arcane stuff in here
;600:just to keep it simpler.  For example, the '*' and '$' are not
;601:currently supported.  I've tried to make it so that it will just
;602:parse and ignore formats we don't support.
;603:*/
;604:int vsprintf(char* buffer, const char* fmt, va_list argptr)
;605:{
line 615
;606:    int*  arg;
;607:    char* buf_p;
;608:    char  ch;
;609:    int   flags;
;610:    int   width;
;611:    int   prec;
;612:    int   n;
;613:    char  sign;
;614:
;615:    buf_p = buffer;
ADDRLP4 4
ADDRFP4 0
INDIRP4
ASGNP4
line 616
;616:    arg   = (int*)argptr;
ADDRLP4 24
ADDRFP4 8
INDIRP4
ASGNP4
ADDRGP4 $227
JUMPV
LABELV $226
line 619
;617:
;618:    while (1)
;619:    {
line 621
;620:        // run through the format string until we hit a '%' or '\0'
;621:        for (ch = *fmt; (ch = *fmt) != '\0' && ch != '%'; fmt++)
ADDRLP4 0
ADDRFP4 4
INDIRP4
INDIRI1
ASGNI1
ADDRGP4 $232
JUMPV
LABELV $229
line 622
;622:        {
line 623
;623:            *buf_p++ = ch;
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
line 624
;624:        }
LABELV $230
line 621
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
LABELV $232
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
EQI4 $233
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 37
NEI4 $229
LABELV $233
line 625
;625:        if (ch == '\0')
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $234
line 626
;626:        {
line 627
;627:            goto done;
ADDRGP4 $236
JUMPV
LABELV $234
line 631
;628:        }
;629:
;630:        // skip over the '%'
;631:        fmt++;
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 634
;632:
;633:        // reset formatting state
;634:        flags = 0;
ADDRLP4 16
CNSTI4 0
ASGNI4
line 635
;635:        width = 0;
ADDRLP4 12
CNSTI4 0
ASGNI4
line 636
;636:        prec  = -1;
ADDRLP4 20
CNSTI4 -1
ASGNI4
line 637
;637:        sign  = '\0';
ADDRLP4 28
CNSTI1 0
ASGNI1
LABELV $237
line 640
;638:
;639:    rflag:
;640:        ch = *fmt++;
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
LABELV $238
line 642
;641:    reswitch:
;642:        switch (ch)
ADDRLP4 36
ADDRLP4 0
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 36
INDIRI4
CNSTI4 99
LTI4 $260
ADDRLP4 36
INDIRI4
CNSTI4 105
GTI4 $261
ADDRLP4 36
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 $262-396
ADDP4
INDIRP4
JUMPV
lit
align 4
LABELV $262
address $255
address $256
address $239
address $257
address $239
address $239
address $256
code
LABELV $260
ADDRLP4 36
INDIRI4
CNSTI4 37
LTI4 $239
ADDRLP4 36
INDIRI4
CNSTI4 57
GTI4 $239
ADDRLP4 36
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 $264-148
ADDP4
INDIRP4
JUMPV
lit
align 4
LABELV $264
address $259
address $239
address $239
address $239
address $239
address $239
address $239
address $239
address $242
address $243
address $239
address $250
address $251
address $251
address $251
address $251
address $251
address $251
address $251
address $251
address $251
code
LABELV $261
ADDRLP4 36
INDIRI4
CNSTI4 115
EQI4 $258
ADDRGP4 $239
JUMPV
line 643
;643:        {
LABELV $242
line 645
;644:        case '-':
;645:            flags |= LADJUST;
ADDRLP4 16
ADDRLP4 16
INDIRI4
CNSTI4 4
BORI4
ASGNI4
line 646
;646:            goto rflag;
ADDRGP4 $237
JUMPV
LABELV $243
line 648
;647:        case '.':
;648:            n = 0;
ADDRLP4 8
CNSTI4 0
ASGNI4
ADDRGP4 $245
JUMPV
LABELV $244
line 650
;649:            while (is_digit((ch = *fmt++)))
;650:            {
line 651
;651:                n = 10 * n + (ch - '0');
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
line 652
;652:            }
LABELV $245
line 649
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
LEU4 $244
line 653
;653:            prec = n < 0 ? -1 : n;
ADDRLP4 8
INDIRI4
CNSTI4 0
GEI4 $248
ADDRLP4 56
CNSTI4 -1
ASGNI4
ADDRGP4 $249
JUMPV
LABELV $248
ADDRLP4 56
ADDRLP4 8
INDIRI4
ASGNI4
LABELV $249
ADDRLP4 20
ADDRLP4 56
INDIRI4
ASGNI4
line 654
;654:            goto reswitch;
ADDRGP4 $238
JUMPV
LABELV $250
line 656
;655:        case '0':
;656:            flags |= ZEROPAD;
ADDRLP4 16
ADDRLP4 16
INDIRI4
CNSTI4 128
BORI4
ASGNI4
line 657
;657:            goto rflag;
ADDRGP4 $237
JUMPV
LABELV $251
line 667
;658:        case '1':
;659:        case '2':
;660:        case '3':
;661:        case '4':
;662:        case '5':
;663:        case '6':
;664:        case '7':
;665:        case '8':
;666:        case '9':
;667:            n = 0;
ADDRLP4 8
CNSTI4 0
ASGNI4
LABELV $252
line 669
;668:            do
;669:            {
line 670
;670:                n  = 10 * n + (ch - '0');
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
line 671
;671:                ch = *fmt++;
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
line 672
;672:            } while (is_digit(ch));
LABELV $253
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 48
SUBI4
CVIU4 4
CNSTU4 9
LEU4 $252
line 673
;673:            width = n;
ADDRLP4 12
ADDRLP4 8
INDIRI4
ASGNI4
line 674
;674:            goto reswitch;
ADDRGP4 $238
JUMPV
LABELV $255
line 676
;675:        case 'c':
;676:            *buf_p++ = (char)*arg;
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
line 677
;677:            arg++;
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 678
;678:            break;
ADDRGP4 $240
JUMPV
LABELV $256
line 681
;679:        case 'd':
;680:        case 'i':
;681:            AddInt(&buf_p, *arg, width, flags);
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
line 682
;682:            arg++;
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 683
;683:            break;
ADDRGP4 $240
JUMPV
LABELV $257
line 685
;684:        case 'f':
;685:            AddFloat(&buf_p, *(double*)arg, width, prec);
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
line 687
;686:#ifdef __LCC__
;687:            arg += 1; // everything is 32 bit in my compiler
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 691
;688:#else
;689:            arg += 2;
;690:#endif
;691:            break;
ADDRGP4 $240
JUMPV
LABELV $258
line 693
;692:        case 's':
;693:            AddString(&buf_p, (char*)*arg, width, prec);
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
line 694
;694:            arg++;
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 695
;695:            break;
ADDRGP4 $240
JUMPV
LABELV $259
line 697
;696:        case '%':
;697:            *buf_p++ = ch;
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
line 698
;698:            break;
ADDRGP4 $240
JUMPV
LABELV $239
line 700
;699:        default:
;700:            *buf_p++ = (char)*arg;
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
line 701
;701:            arg++;
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 702
;702:            break;
LABELV $240
line 704
;703:        }
;704:    }
LABELV $227
line 618
ADDRGP4 $226
JUMPV
LABELV $236
line 707
;705:
;706:done:
;707:    *buf_p = 0;
ADDRLP4 4
INDIRP4
CNSTI1 0
ASGNI1
line 708
;708:    return buf_p - buffer;
ADDRLP4 4
INDIRP4
CVPU4 4
ADDRFP4 0
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
RETI4
LABELV $225
endproc vsprintf 72 16
export sscanf
proc sscanf 28 4
line 713
;709:}
;710:
;711:/* this is really crappy */
;712:int sscanf(const char* buffer, const char* fmt, ...)
;713:{
line 718
;714:    int   cmd;
;715:    int** arg;
;716:    int   count;
;717:
;718:    arg   = (int**)&fmt + 1;
ADDRLP4 4
ADDRFP4 4+4
ASGNP4
line 719
;719:    count = 0;
ADDRLP4 8
CNSTI4 0
ASGNI4
ADDRGP4 $269
JUMPV
LABELV $268
line 722
;720:
;721:    while (*fmt)
;722:    {
line 723
;723:        if (fmt[0] != '%')
ADDRFP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 37
EQI4 $271
line 724
;724:        {
line 725
;725:            fmt++;
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 726
;726:            continue;
ADDRGP4 $269
JUMPV
LABELV $271
line 729
;727:        }
;728:
;729:        cmd = fmt[1];
ADDRLP4 0
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
INDIRI1
CVII4 1
ASGNI4
line 730
;730:        fmt += 2;
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 2
ADDP4
ASGNP4
line 732
;731:
;732:        switch (cmd)
ADDRLP4 16
CNSTI4 100
ASGNI4
ADDRLP4 0
INDIRI4
ADDRLP4 16
INDIRI4
EQI4 $275
ADDRLP4 0
INDIRI4
CNSTI4 102
EQI4 $276
ADDRLP4 0
INDIRI4
CNSTI4 105
EQI4 $275
ADDRLP4 0
INDIRI4
ADDRLP4 16
INDIRI4
LTI4 $273
LABELV $277
ADDRLP4 0
INDIRI4
CNSTI4 117
EQI4 $275
ADDRGP4 $273
JUMPV
line 733
;733:        {
LABELV $275
line 737
;734:        case 'i':
;735:        case 'd':
;736:        case 'u':
;737:            **arg = _atoi(&buffer);
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
line 738
;738:            break;
ADDRGP4 $274
JUMPV
LABELV $276
line 740
;739:        case 'f':
;740:            *(float*)*arg = _atof(&buffer);
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
line 741
;741:            break;
LABELV $273
LABELV $274
line 743
;742:        }
;743:        arg++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 744
;744:    }
LABELV $269
line 721
ADDRFP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $268
line 746
;745:
;746:    return count;
ADDRLP4 8
INDIRI4
RETI4
LABELV $266
endproc sscanf 28 4
import memcpy
import memset
lit
align 1
LABELV $210
byte 1 40
byte 1 110
byte 1 117
byte 1 108
byte 1 108
byte 1 41
byte 1 0
