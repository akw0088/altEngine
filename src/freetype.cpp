#include "freetype.h"

int FreeType::init(char *ttf_font, int width, int height, int char_width, int char_height)
{
	int error = FT_Init_FreeType(&library);
	if (error)
	{
		printf("FreeType init failed\r\n");
		return -1;
	}

	error = FT_New_Face(library, ttf_font, 0, &face);
	if (error == FT_Err_Unknown_File_Format)
	{
		printf("FreeType load ttf bad format\r\n");
		return -2;
	}
	else if (error)
	{
		printf("FreeType load ttf failed\r\n");
		return -3;
	}

	// (0, 16 * 64, 300, 300)
	error = FT_Set_Char_Size( face,
		char_width,       /* char_width in 1/64th of points  */
		char_height,   /* char_height in 1/64th of points */
		width,     /* horizontal device resolution    */
		height);   /* vertical device resolution      */

	if (error)
	{
		return -4;
	}


	return 0;
}


int FreeType::getBitmap32(int charcode, font_t &font)
{
	int error;

	int glyph_index = FT_Get_Char_Index(face, charcode);

	error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
	if (error)
	{
		return -1;
	}

	error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
	if (error)
	{
		return -2;
	}

	int size = face->glyph->bitmap.rows * face->glyph->bitmap.width;

	font.px = face->glyph->bitmap.width;
	font.py = face->glyph->bitmap.rows;
	font.data = (unsigned int *)malloc(size * 4);

	for (int y = 0; y < (int)face->glyph->bitmap.rows; y++)
	{
		for (int x = 0; x < (int)face->glyph->bitmap.width; x++)
		{
			unsigned char p = face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + x];

			font.data[(font.py - 1 - y) * font.px + x] = (p << 16 | p << 8 | p);
		}
	}
	font.left = face->glyph->bitmap_left;
	font.top = face->glyph->bitmap_top;
	font.advance = face->glyph->advance.x / 64;

	return 0;
}
