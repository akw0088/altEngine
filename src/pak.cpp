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
	int size = 0;
    
	char *pak = get_file(pakfile, &size);
	if (pak == NULL)
	{
		printf("error opening %s\n", pakfile);
		return -1;
	}

	pak_header_t *header = (pak_header_t *)pak;
	if (strncmp(header->id, "PACK", 4) != 0)
	{
		printf("invalid header id\n");
		return -1;
	}
    
	if (header->dir_length % sizeof(pak_entry_t) != 0)
	{
		printf("invalid dir length\n");  
		return -1;
	}
    
	pak_entry_t *entries = (pak_entry_t *)(pak + header->dir_offset);
	pak_entry_t *entry = entries;
	int num_entries = header->dir_length / sizeof(pak_entry_t);
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

			memcpy(data, pak + entry->offset, entry->length);
			*pdata = data;
			break;	
		}
	}
   	free((void *)pak); 
	return 0;
}

