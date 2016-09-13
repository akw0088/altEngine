#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Define targa header.
#pragma pack(1)
typedef struct
{
	byte		identsize;		// Size of ID field that follows header (0)
	byte		colorMapType;		// 0 = None, 1 = paletted
	byte		imageType;		// 0 = none, 1 = indexed, 2 = rgb, 3 = grey, +8=rle
	unsigned short	colorMapStart;		// First colour map entry
	unsigned short	colorMapLength;		// Number of colors
	unsigned char 	colorMapBits;		// bits per palette entry
	unsigned short	xstart;			// image x origin
	unsigned short	ystart;			// image y origin
	unsigned short	width;			// width in pixels
	unsigned short	height;			// height in pixels
	byte		bits;			// bits per pixel (8 16, 24, 32)
	byte		descriptor;		// image descriptor
} TGAHEADER;

#pragma pack(8)

byte *gltLoadTGA(const char *file, int *iWidth, int *iHeight, int *iComponents, int *eFormat)
{
	FILE		*pFile;			// File pointer
	TGAHEADER	tgaHeader;		// TGA file header
	unsigned long	lImageSize;		// Size in bytes of image
	short		sDepth;			// Pixel depth;
	byte		*pBits;          // Pointer to bits

	// Default/Failed values
	*iWidth = 0;
	*iHeight = 0;
#ifndef DIRECTX
	*eFormat = GL_BGR_EXT;
	*iComponents = GL_RGB8;
#endif

	// Attempt to open the file
	pFile = fopen(file, "rb");
	if(pFile == NULL)
		return NULL;

	// Read in header (binary)
	fread(&tgaHeader, 18/* sizeof(TGAHEADER)*/, 1, pFile);

	// Do byte swap for big vs little endian
	// new intel mac's use intel little endian cpu's
	// (doubt an older ppc mac could even run this)
	#ifdef __APPLE__
	/*
		BYTE_SWAP(tgaHeader.colorMapStart);
		BYTE_SWAP(tgaHeader.colorMapLength);
		BYTE_SWAP(tgaHeader.xstart);
		BYTE_SWAP(tgaHeader.ystart);
		BYTE_SWAP(tgaHeader.width);
		BYTE_SWAP(tgaHeader.height);
	*/
	#endif


	// Get width, height, and depth of texture
	*iWidth = tgaHeader.width;
	*iHeight = tgaHeader.height;
	sDepth = tgaHeader.bits / 8;
    
	if(tgaHeader.bits != 8 && tgaHeader.bits != 24 && tgaHeader.bits != 32)
		return NULL;

	lImageSize = tgaHeader.width * tgaHeader.height * sDepth;

	pBits = new byte [lImageSize * sizeof(byte)];
	if(pBits == NULL)
		return NULL;

	if(fread(pBits, lImageSize, 1, pFile) != 1)
	{
		delete [] pBits;
		return NULL;
	}
	fclose(pFile);

	// Set OpenGL format expected
#ifndef DIRECTX
	switch(sDepth)
	{
	case 3:     // Most likely case
		*eFormat = GL_BGR_EXT;
		*iComponents = GL_RGB8;
		break;
	case 4:
		*eFormat = GL_BGRA_EXT;
		*iComponents = GL_RGBA8;
		break;
	case 1:
		*eFormat = GL_LUMINANCE;
		*iComponents = GL_LUMINANCE8;
		break;
	};
#else
	switch(sDepth)
	{
	case 3:
		*eFormat = 3;
		*iComponents = 3;
		break;
	case 4:
		*eFormat = 4;
		*iComponents = 4;
		break;
	case 1:
		*eFormat = 1;
		*iComponents = 1;
		break;
	};

	//directx does not support 24 bit bitmaps conver to 32bit
	if (*eFormat == 3)
	{
		byte *pNewBits;

		*eFormat = 4;
		*iComponents = 4;
		pNewBits = tga_24to32(tgaHeader.width, tgaHeader.height, pBits);
		delete [] pBits;
		return pNewBits;
	}
#endif
	return pBits;
}


byte *tga_24to32(int width, int height, byte *pBits)
{
	int lImageSize = width * height * 4;
	byte *pNewBits = new byte [lImageSize * sizeof(byte)];

	for(int i = 0, j = 0; i < lImageSize; i += 4)
	{
		pNewBits[i] = pBits[j++];
		pNewBits[i+1] = pBits[j++];
		pNewBits[i+2] = pBits[j++];
		pNewBits[i+3] = 0;
	}
	return pNewBits;
}
