#include "include.h"

typedef struct
{
	char id[4];
	int dir_offset;
	int dir_length;
} pak_header_t;

typedef struct
{
	char name[56];
	int offset;
	int length;
} pak_entry_t;

char *get_pakfile(char *pakfile, char *file)
{
	int i;
    
	FILE *pak = (FILE *)fopen(pakfile, "rb");
	if (pak == NULL)
	{
		printf("error opening %s\n", pakfile);
		return NULL;
	}

	pak_header_t header;
	fread(&header, sizeof(pak_header_t), 1, pak);
    
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
    
	pak_entry_t *entries = (pak_entry_t *)malloc(header.dir_length);
	if (entries == NULL)
	{
		perror("malloc failed");
		fclose(pak);
		return NULL;
	}
    
	fseek(pak, header.dir_offset, SEEK_SET);
	fread(entries, header.dir_length, 1, pak);
    
	pak_entry_t *entry = entries;
	int num_entries = header.dir_length / sizeof(pak_entry_t);
	for (i = 0; i < num_entries; ++i, ++entry)
	{
//		printf("%d: %s (%d, %d)\n", i, entry->name, entry->offset, entry->length);

		if (strcmp(entry->name, file) == 0)
		{
			char *data = (char *)malloc(entry->length);
			if (data == NULL)
			{
				perror("malloc failed");
				return NULL;
			}

			fseek(pak, entry->offset, SEEK_SET);
			fread(data, entry->length, 1, pak);
			return data;
		}
	}
    
	free(entries);
	fclose(pak);
	return NULL;
}

