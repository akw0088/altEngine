// Extract a PAK file (from Quake 1 and 2)

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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

char *dirname(const char *path)
{
	const char *p = strrchr(path, '/');
	if (p)
	{
		size_t n = p - path;
		if (n > 0)
		{
			char *dir = (char *)malloc(n + 1);
			strncpy(dir, path, n);
			dir[n] = 0;
			return dir;
		}
	}
	return NULL;
}

int mkdir_p(const char *dir, mode_t mode)
{
	int rv = mkdir(dir, mode);
	if (rv < 0 && errno == ENOENT)
	{
		char *parent_dir = dirname(dir);
		if (parent_dir)
		{
			mkdir_p(parent_dir, mode);
			free(parent_dir);
			rv = mkdir(dir, mode);
		}
	}
	return rv;
}

int get_pakfile(char *pakfile, char *file)
{
	int i;
	char buf[4096];
    
	printf("opening pak %s\n", pakfile);
	FILE *pak = (FILE *)fopen(pakfile, "rb");
	if (pak == NULL)
	{
		printf("error opening %s\n", pak);
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
	if (entries = NULL)
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
			break;	
		}
/*
		char *dir = dirname(entry->name);
		if (dir)
		{
			mkdir_p(dir, 0777);
			free(dir);
	        }
        
		FILE *out = fopen(entry->name, "wb");
		if (!out)
		{
			fprintf(stderr, "error opening %s for writing\n", entry->name);
			continue;
		}
        
		char *buf = (char *)malloc(entry->length);
		if (!buf)
		{
			fprintf(stderr, "error allocating buffer for entry\n");
			continue;
		}
        
		fseek(pak, entry->offset, SEEK_SET);
		if (fread(buf, entry->length, 1, pak) < 0)
		{
			fprintf(stderr, "error reading entry\n");
		}
		else if (fwrite(buf, entry->length, 1, out) < 0)
		{
			fprintf(stderr, "error writing entry\n");
		}
        
		free(buf);
        
		fclose(out);
*/
	}
    
	free(entries);
	fclose(pak);
	return 0;
}

