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

int get_pakfile(char *pakfile, char *file, char **pdata)
{
	int i;
    
	*pdata = NULL;

	FILE *pak = (FILE *)fopen(pakfile, "rb");
	if (pak == NULL)
	{
		printf("error opening %s\n", pakfile);
		return -1;
	}

	pak_header_t header;
	fread(&header, sizeof(pak_header_t), 1, pak);
    
	if (strncmp(header.id, "PACK", 4) != 0)
	{
		printf("invalid header id\n");
		fclose(pak);
		return -1;
	}
    
	if (header.dir_length % sizeof(pak_entry_t) != 0)
	{
		printf("invalid dir length\n");  
		fclose(pak);
		return 1;
	}
    
	pak_entry_t *entries = (pak_entry_t *)malloc(header.dir_length);
	if (entries == NULL)
	{
		perror("malloc failed");
		fclose(pak);
		return 1;
	}
    
	fseek(pak, header.dir_offset, SEEK_SET);
	fread(entries, header.dir_length, 1, pak);
    
	pak_entry_t *entry = entries;
	int num_entries = header.dir_length / sizeof(pak_entry_t);
	for (i = 0; i < num_entries; ++i, ++entry)
	{
		printf("%d: %s (%d, %d)\n", i, entry->name, entry->offset, entry->length);

		if (strcmp(entry->name, file) == 0)
		{
			char *data = (char *)malloc(entry->length);
			if (data == NULL)
			{
				perror("malloc failed");
				return -1;
			}

			fseek(pak, entry->offset, SEEK_SET);
			if (fread(data, entry->length, 1, pak) < 0)
			{
				fprintf(stderr, "error reading entry\n");
			}
			*pdata = data;
			break;	
		}
	}
    
	free(entries);
	fclose(pak);
	return 0;
}

