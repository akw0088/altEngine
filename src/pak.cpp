//=============================================================================
// This file is part of the altEngine distribution
// (https://github.com/akw0088/altEngine/)
// Copyright (c) 2018 Alexander Wright All Rights Reserved.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//=============================================================================

#include "include.h"

typedef struct
{
	char id[4]; //PACK
	int dir_offset;
	int dir_length;
} pak_header_t;

typedef struct
{
	unsigned char type[4]; //IWAD or PWAD
	int num_lump;
	int dir_offset;
} wad_header_t;

typedef struct
{
	char name[56];
	int offset;
	int length;
} pak_entry_t;

typedef struct
{
	int offset;
	int length;
	char name[8];
} wad_lump_t;


// Audio header
typedef struct
{
	unsigned short int format; // must be 3
	unsigned short sample_rate; // usually 11025, can be 22050
	unsigned int num_sample;
	unsigned char pad[16];
	unsigned char data;
} dmx_header_t;

/*
[8-1] palettes (PLAYPAL)
[8-2] colormaps (COLORMAP)
[8-3] dos exit text (ENDOOM)
[8-6] demos (DEMO1, DEMO2, and DEMO3)
[8-4] texture composition list (TEXTURE1 and TEXTURE2)
[8-5] wall patch "number for name" indexing list (PNAMES)
[7-4] midi mapping (GENMIDI)
[7-5] Gravis UltraSound patch mappings (DMXGUS)
[7-1] PC speaker sound effects (DP*)
[7-2] Soundcard sound effects (DS*)
[7-3] songs (D_*)
[6]   flats (lumpnames between F_START and F_END)
[5]   all other graphics (all other lumps)

// Sounds
Lump 112: name DSSHOTGN size 9453
Lump 114: name DSSGCOCK size 5906
Lump 116: name DSSAWUP size 16297
Lump 118: name DSSAWIDL size 7539
Lump 120: name DSSAWFUL size 18104
Lump 122: name DSSAWHIT size 8018
Lump 124: name DSRLAUNC size 15491
Lump 126: name DSRXPLOD size 14483
Lump 128: name DSFIRSHT size 14748
Lump 130: name DSFIRXPL size 11701
Lump 132: name DSPSTART size 8236
Lump 134: name DSPSTOP size 6666
Lump 136: name DSDOROPN size 13824
Lump 138: name DSDORCLS size 13890
Lump 140: name DSSTNMOV size 2967
Lump 142: name DSSWTCHN size 6263
Lump 144: name DSSWTCHX size 5411
Lump 146: name DSPLPAIN size 15160
Lump 148: name DSDMPAIN size 9519
Lump 150: name DSPOPAIN size 8643
Lump 152: name DSSLOP size 11126
Lump 154: name DSITEMUP size 2271
Lump 156: name DSWPNUP size 5883
Lump 158: name DSOOF size 3918
Lump 160: name DSTELEPT size 15492
Lump 162: name DSPOSIT1 size 5342
Lump 164: name DSPOSIT2 size 11286
Lump 166: name DSPOSIT3 size 11103
Lump 168: name DSBGSIT1 size 13634
Lump 170: name DSBGSIT2 size 16083
Lump 172: name DSSGTSIT size 11172
Lump 174: name DSBRSSIT size 13770
Lump 176: name DSSGTATK size 9358
Lump 178: name DSCLAW size 6412
Lump 180: name DSPLDETH size 11020
Lump 182: name DSPDIEHI size 10843
Lump 184: name DSPODTH1 size 12971
Lump 186: name DSPODTH2 size 9342
Lump 188: name DSPODTH3 size 10954
Lump 190: name DSBGDTH1 size 7129
Lump 192: name DSBGDTH2 size 9319
Lump 194: name DSSGTDTH size 12259
Lump 196: name DSBRSDTH size 11043
Lump 198: name DSPOSACT size 10782
Lump 200: name DSBGACT size 10032
Lump 202: name DSDMACT size 11857
Lump 204: name DSNOWAY size 3918
Lump 206: name DSBAREXP size 18600
Lump 208: name DSPUNCH size 2504
Lump 210: name DSTINK size 205
Lump 212: name DSBDOPN size 4189
Lump 214: name DSBDCLS size 4203
Lump 216: name DSITMBK size 11183
Lump 218: name DSGETPOW size 7936

*/

char *get_wadfile(char *wadfile, char *lump, int *lump_size, char **pdata)
{
	char *lump_data = NULL;
	int size = 0;
	wad_header_t *header = NULL;

	char *data = get_file(wadfile, &size);
	if (data == NULL)
	{
		printf("Unable to open %s\n", wadfile);
		return NULL;
	}
	*pdata = data;

	header =(wad_header_t *)data;

	if (memcmp((char *)header->type, "IWAD", 4) != 0)
	{
		//printf("IWAD %d lumps\n", header->num_lump);
	}
	else if (memcmp((char *)header->type, "PWAD", 4) != 0)
	{
		//printf("PWAD %d lumps\n", header->num_lump);
	}
	else
	{
		printf("Invalid WAD file\n");
		return NULL;
	}

	wad_lump_t *lump_table = (wad_lump_t *)&data[header->dir_offset];

	for (int i = 0; i < header->num_lump; i++)
	{
		char name[9];

		memcpy(name, lump_table[i].name, 8);
		name[8] = '\0';
		//printf("Lump %d: name %s size %d\n", i, name, lump_table[i].size);

		if (strstr(lump, name) != 0)
		{
			lump_data = &data[lump_table[i].offset];
			*lump_size = lump_table[i].length;
			break;
		}
	}

	return lump_data;
}



void lump_to_wave(char *lump_data, int size, wave_t *wave)
{
	dmx_header_t *header = NULL;
	header = (dmx_header_t *)lump_data;

	wave->pcmData = &(header->data);
	wave->format->channels = 1;
	wave->format->format = AL_FORMAT_MONO8;
	wave->format->sampleRate = header->sample_rate;
	wave->format->sampleSize = 8;
	wave->format->avgSampleRate = header->sample_rate;
	wave->format->align = 1;
	wave->dataSize = header->num_sample;
}

char *get_pakfile(char *pakfile, char *file)
{
	int i;
	int ret;    

	FILE *pak = (FILE *)fopen(pakfile, "rb");
	if (pak == NULL)
	{
		printf("error opening %s\n", pakfile);
		return NULL;
	}

	pak_header_t header;
	ret = fread(&header, 1, sizeof(pak_header_t), pak);
	if (ret != sizeof(pak_header_t))
	{
		fclose(pak);
		return NULL;
	}
    
	if (strncmp(header.id, "PACK", 4) != 0)
	{
		printf("invalid header id\n");
		fclose(pak);
		return NULL;
	}
    
	if (header.dir_length % sizeof(pak_entry_t) != 0)
	{
		printf("invalid dir length\n");  
		fclose(pak);
		return NULL;
	}
    
	pak_entry_t *entries = (pak_entry_t *) new char [header.dir_length];
	if (entries == NULL)
	{
		perror("malloc failed");
		fclose(pak);
		return NULL;
	}
    
	fseek(pak, header.dir_offset, SEEK_SET);
	ret = fread(entries, 1, header.dir_length, pak);
	if (ret != header.dir_length)
	{
		delete [] entries;
		fclose(pak);
		return NULL;
	}
    
	pak_entry_t *entry = entries;
	int num_entries = header.dir_length / sizeof(pak_entry_t);
	for (i = 0; i < num_entries; ++i, ++entry)
	{
//		printf("%d: %s (%d, %d)\n", i, entry->name, entry->offset, entry->length);

		if (strcmp(entry->name, file) == 0)
		{
			char *data = (char *) new char [entry->length];
			if (data == NULL)
			{
				perror("malloc failed");
				return NULL;
			}

			fseek(pak, entry->offset, SEEK_SET);
			ret = fread(data, 1, entry->length, pak);
			if (ret != entry->length)
			{
				delete [] data;
				delete [] entries;
				fclose(pak);
				return NULL;
			}
			
			delete [] entries;
			fclose(pak);
			return data;
		}
	}
	delete [] entries;
	fclose(pak);
	return NULL;
}

