#include <stdio.h>
#include <string.h>


int convert(char *data, char *name)
{
	unsigned int *count = (unsigned int *)data;
	unsigned int *index = (unsigned int *)(data + 4);

	char file[128] = { 0 };

	sprintf(file, "%s.txt", name);
	FILE *fp = fopen(file, "w");
	if (fp == NULL)
	{
		perror("fp");
		return -1;
	}

	for (int i = strlen(name) - 1; i > 0; i--)
	{
		if (name[i] == '.')
		{
			name[i] = '\0';
			break;
		}
	}

	fprintf(fp, "\tconst num_index_%s = %d;\n", name, *count);
	fprintf(fp, "\tconst index_%s = new Int32Array([ \n", name);
	for (unsigned int i = 0; i < *count; i++)
	{
		fprintf(fp, "%u, ",	index[i]);
	}
	fprintf(fp, "\t]);\n");
//	fprintf(fp, "\tnum_index.push(num_index_%s);\r\n", name);
//	fprintf(fp, "\tindex.push(index_%s);\r\n", name);
	fclose(fp);

	return 0;
}

char *get_file(char *filename, unsigned int *size)
{
	FILE	*file;
	char	*buffer;
	int	file_size, bytes_read;

	file = fopen(filename, "rb");
	if (file == NULL)
		return 0;
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	buffer = new char[file_size + 1];
	bytes_read = (int)fread(buffer, sizeof(char), file_size, file);
	if (bytes_read != file_size)
	{
		delete[] buffer;
		fclose(file);
		return 0;
	}
	fclose(file);
	buffer[file_size] = '\0';

	if (size != NULL)
	{
		*size = file_size;
	}

	return buffer;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s <file>\r\n", argv[0]);
		return 0;
	}


	unsigned int size = 0;
	char *data = get_file(argv[1], &size);
	convert(data, argv[1]);

	return 0;
}

