#ifndef FREETYPE_H
#define FREETYPE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ft2build.h>
#include FT_FREETYPE_H


typedef struct
{
	unsigned int *data;
	int px;
	int py;
	int advance;
	int left;
	int top;
} font_t;

class FreeType
{
public:
	int init(char *ttf_font, int width, int height, int char_width, int char_height);
	int getBitmap32(int charcode, font_t &font);
private:
	FT_Library  library;   /* handle to library     */
	FT_Face     face;      /* handle to face object */
};
#endif