#include <stdio.h>
#include <math.h>

#define SWAP(a,b) {temp = a; a = b; b = temp;}


/* Describes a single point (used for a single vertex) */
struct Point {
	int X;   /* X coordinate */
	int Y;   /* Y coordinate */
};

/* Describes a series of points (used to store a list of vertices that
describe a polygon; each vertex is assumed to connect to the two
adjacent vertices, and the last vertex is assumed to connect to the
first) */
struct PointListHeader {
	int Length;                /* # of points */
	struct Point * PointPtr;   /* pointer to list of points */
};

/* Describes the beginning and ending X coordinates of a single
horizontal line */
struct HLine {
	int XStart; /* X coordinate of leftmost pixel in line */
	int XEnd;   /* X coordinate of rightmost pixel in line */
};

/* Describes a Length-long series of horizontal lines, all assumed to
be on contiguous scan lines starting at YStart and proceeding
downward (used to describe a scan-converted polygon to the
low-level hardware-dependent drawing code) */
struct HLineList {
	int Length;                /* # of horizontal lines */
	int YStart;                /* Y coordinate of topmost line */
	struct HLine * HLinePtr;   /* pointer to list of horz lines */
};




#pragma pack(1)
typedef struct
{
	char type[2];
	int file_size;
	int reserved;
	int offset;
} bmpheader_t;
#pragma pack(8)

#pragma pack(1)
typedef struct
{
	int size;
	int width;
	int height;
	short planes;
	short bpp;
	int compression;
	int image_size;
	int xres;
	int yres;
	int clr_used;
	int clr_important;
} dib_t;
#pragma pack(8)

#pragma pack(1)
typedef struct
{
	bmpheader_t	header;
	dib_t		dib;
} bitmap_t;
#pragma pack(8)


struct EdgeState
{
	struct EdgeState *NextEdge;
	int X;
	int StartY;
	int WholePixelXMove;
	int XDirection;
	int ErrorTerm;
	int ErrorTermAdjUp;
	int ErrorTermAdjDown;
	int Count;
};


class Raster2D
{
public:
	// (mine)
	void read_bitmap(char *filename, int &width, int &height, unsigned int **data);
	// (mine)
	void write_bitmap(char *filename, int width, int height, unsigned int *data);

	// draws a line, fastest? (Abrash Black Book)
	void DrawLine(int *pixels, int width, int height, int X0, int Y0, int X1, int Y1, int color);

	// draws a line, fast bresham (Black art of 3d game programming)
	void Draw_Line(unsigned char *pixels, int width, int height, int xo, int yo, int x1, int y1, unsigned char color);

	// basic 2D line drawing (slowest, mine, but hey it works)
	void draw_line(unsigned int *pixels, int width, int height, int x1, int y1, int x2, int y2, int color);

	// draw a convex polygon (Abrash Black Book)
	int FillConvexPolygon(int *pixels, int width, int height, struct PointListHeader * VertexList, int Color, int XOffset, int YOffset);

	// draw a non-convex polygon (Abrash Black Book)
	int FillPolygon(unsigned int *pixels, int width, int height, struct PointListHeader * VertexList, int Color, int XOffset, int YOffset);


	// draws filled non-rotated rect (Black art of 3d game programming)
	void Draw_Rectangle(unsigned int *pixels, int width, int height, int x1, int y1, int x2, int y2, int color);

	// draws filled non-rotated triangle (Black art of 3d game programming)
	void Draw_Triangle(unsigned int *pixels, int width, int height, int x1, int y1, int x2, int y2, int x3, int y3, int color);


	// crappy flood fill algorithm (mine)
	void flood_fill(unsigned int *pixels, int width, int height, int x, int y, int old_color, int new_color);

	// draws a rectangle using line function with rotation (mine)
	void draw_rect(unsigned int *pixels, int width, int height, float angle, int w, int l, int x, int y, int color);

	// draws a rectangle using line function no rotation (mine)
	void Rectangle(unsigned int *vram, int xres, int yres, int x1, int y1, int x2, int y2);

	// draws a triangle using line function (mine)
	void Triangle(unsigned int *vram, int xres, int yres, int x1, int y1, int x2, int y2, int color);

	// draws text from a bitmap atlas (mine)
	void draw_text(unsigned int *pixels, int width, int height, char *msg, int x, int y, unsigned int *tex, int flip);

	// draws a circle non-filled or filled (mine)
	void draw_circle(unsigned int *pixels, int width, int height, int xc, int yc, int radius, int color, int filled);

	// draws an ellipse non-filled or filled (mine)
	void draw_ellipse(unsigned int *pixels, int width, int height, int xc, int yc, int rx, int ry, int color, int filled);

	void BitBlt(unsigned int *pixels, int width, int height,
		int x, int y,
		unsigned int *sprite, unsigned int sprite_width, unsigned int sprite_height);






	// GDI functions to eventually make wrappers of
	/*
	Rectangle(hdc,
	(int)((point.x * scale - size * scale) + offset.x),
	(int)((point.y * scale - size * scale) + offset.y),
	(int)((point.x * scale + size * scale) + offset.x),
	(int)((point.y * scale + size * scale) + offset.y));

	Ellipse(hdc, left, top, right, bottom);

	MoveToEx(hdc,
	(int)(a.x * scale + offset.x),
	(int)(a.y * scale + offset.y),
	NULL);

	LineTo(hdc,
	(int)(b.x * scale + offset.x),
	(int)(b.y * scale + offset.y));


	int ret0 = ExtFloodFill(hdc,
	(int)((point.x * scale) + offset.x),
	(int)((point.y * scale) + offset.y),
	color,
	FLOODFILLBORDER);
	*/


private:
	char *tga_24to32(int width, int height, char *pBits, bool bgr);

	inline int imin(int x, int y)
	{
		return y ^ ((x ^ y) & -(x < y));
	}

	inline int imax(int x, int y)
	{
		return y ^ ((x ^ y) & -(x > y));
	}

	inline void iclamp(int &a, int mi, int ma)
	{
		a = imin(imax(a, mi), ma);
	}

	// Abrash Black Book
	void Octant0(int *pixels, int width, int height, unsigned int X0, unsigned int Y0, unsigned int DeltaX, unsigned int DeltaY, int XDirection, int color);
	void Octant1(int *pixels, int width, int height, unsigned int X0, unsigned int Y0, unsigned int DeltaX, unsigned int DeltaY, int XDirection, int color);
	void ScanEdge(int X1, int Y1, int X2, int Y2, int SetXStart, int SkipFirst, struct HLine **EdgePointPtr);
	void DrawHorizontalLineList(int *pixels, int width, int height, struct HLineList * HLineListPtr, int color);


	// Abrash Black Book
	void DrawHorizontalLineSeg(unsigned int *pixels, int width, int height, int Y, int LeftX, int RightX, int Color);
	void ScanOutAET(unsigned int *pixels, int width, int height, int YToScan, int Color);
	void BuildGET(struct PointListHeader * VertexList,
		struct EdgeState * NextFreeEdgeStruc, int XOffset, int YOffset);
	void XSortAET();
	void AdvanceAET();
	void MoveXSortedToAET(int YToMove);

	// Black art of 3d game programming
	void Triangle_Line(unsigned int *pixels, unsigned int xs, unsigned int xe, int color);
	void Draw_Top_Triangle(unsigned int *pixels, int swidth, int sheight, int x1, int y1, int x2, int y2, int x3, int y3, int color);
	void Draw_Bottom_Triangle(unsigned int *pixels, int swidth, int sheight, int x1, int y1, int x2, int y2, int x3, int y3, int color);



private:
	struct EdgeState *GETPtr, *AETPtr;
};

