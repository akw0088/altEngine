#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "raster2d.h"




char *Raster2D::tga_24to32(int width, int height, char *pBits, bool bgr)
{
	int lImageSize = width * height * 4;
	char *pNewBits = new char[lImageSize * sizeof(char)];

	for (int i = 0, j = 0; i < lImageSize; i += 4)
	{
		if (bgr)
		{
			pNewBits[i + 2] = pBits[j++];
			pNewBits[i + 1] = pBits[j++];
			pNewBits[i + 0] = pBits[j++];
		}
		else
		{
			pNewBits[i + 0] = pBits[j++];
			pNewBits[i + 1] = pBits[j++];
			pNewBits[i + 2] = pBits[j++];
		}
		pNewBits[i + 3] = 0;
	}
	return pNewBits;
}


void Raster2D::read_bitmap(char *filename, int &width, int &height, unsigned int **data)
{
	FILE *file;
	bitmap_t	bitmap;

	memset(&bitmap, 0, sizeof(bitmap_t));

	file = fopen(filename, "rb");
	if (file == NULL)
	{
		perror("Unable to write file");
		*data = NULL;
		return;
	}

	fread(&bitmap, 1, sizeof(bitmap_t), file);
	width = bitmap.dib.width;
	height = bitmap.dib.height;
	*data = new unsigned int[width * height];
	fread((void *)*data, 1, width * height * 4, file);
	fclose(file);

	if (bitmap.dib.bpp == 24)
	{
		unsigned int *old = *data;
		*data = (unsigned int *)tga_24to32(width, height, (char *)*data, false);
		delete[] old;
	}
}




void Raster2D::write_bitmap(char *filename, int width, int height, unsigned int *data)
{
	FILE *file;
	bitmap_t	bitmap;

	memset(&bitmap, 0, sizeof(bitmap_t));
	memcpy(bitmap.header.type, "BM", 2);
	bitmap.header.offset = sizeof(bmpheader_t);
	bitmap.dib.size = sizeof(dib_t);
	bitmap.dib.width = width;
	bitmap.dib.height = height;
	bitmap.dib.planes = 1;
	bitmap.dib.bpp = 32;
	bitmap.dib.compression = 0;
	bitmap.dib.image_size = width * height * sizeof(int);
	bitmap.header.file_size = sizeof(bmpheader_t) + sizeof(dib_t) + bitmap.dib.image_size;

	file = fopen(filename, "wb");
	if (file == NULL)
	{
		perror("Unable to write file");
		return;
	}

	fwrite(&bitmap, 1, sizeof(bitmap_t), file);
	fwrite((void *)data, 1, width * height * 4, file);
	fclose(file);
}





/*
* Draws a line in octant 0 or 3 ( |DeltaX| >= DeltaY ).
*/
void Raster2D::Octant0(int *pixels, int width, int height, unsigned int X0, unsigned int Y0, unsigned int DeltaX, unsigned int DeltaY, int XDirection, int color)
{
	int DeltaYx2;
	int DeltaYx2MinusDeltaXx2;
	int ErrorTerm;

	/* Set up initial error term and values used inside drawing loop */
	DeltaYx2 = DeltaY * 2;
	DeltaYx2MinusDeltaXx2 = DeltaYx2 - (int)(DeltaX * 2);
	ErrorTerm = DeltaYx2 - (int)DeltaX;

	/* Draw the line */

	pixels[(unsigned int)X0 + (unsigned int)Y0 * width] = color; /* draw the first pixel */
	while (DeltaX--) {
		/* See if it's time to advance the Y coordinate */
		if (ErrorTerm >= 0) {
			/* Advance the Y coordinate & adjust the error term
			back down */
			Y0++;
			ErrorTerm += DeltaYx2MinusDeltaXx2;
		}
		else {
			/* Add to the error term */
			ErrorTerm += DeltaYx2;
		}
		X0 += XDirection;          /* advance the X coordinate */
		pixels[(unsigned int)X0 + (unsigned int)Y0 * width] = color;           /* draw a pixel */
	}
}

/*
* Draws a line in octant 1 or 2 ( |DeltaX| < DeltaY ).
*/
void Raster2D::Octant1(int *pixels, int width, int height, unsigned int X0, unsigned int Y0, unsigned int DeltaX, unsigned int DeltaY, int XDirection, int color)
{
	int DeltaXx2;
	int DeltaXx2MinusDeltaYx2;
	int ErrorTerm;

	/* Set up initial error term and values used inside drawing loop */
	DeltaXx2 = DeltaX * 2;
	DeltaXx2MinusDeltaYx2 = DeltaXx2 - (int)(DeltaY * 2);
	ErrorTerm = DeltaXx2 - (int)DeltaY;

	pixels[(unsigned int)X0 + (unsigned int)Y0 * width] = color;           /* draw the first pixel */
	while (DeltaY--) {
		/* See if it's time to advance the X coordinate */
		if (ErrorTerm >= 0) {
			/* Advance the X coordinate & adjust the error term
			back down */
			X0 += XDirection;
			ErrorTerm += DeltaXx2MinusDeltaYx2;
		}
		else {
			/* Add to the error term */
			ErrorTerm += DeltaXx2;
		}
		Y0++;                   /* advance the Y coordinate */
		pixels[(unsigned int)X0 + (unsigned int)Y0 * width] = color;        /* draw a pixel */
	}
}

void Raster2D::DrawLine(int *pixels, int width, int height, int X0, int Y0, int X1, int Y1, int color)
{
	int DeltaX, DeltaY;
	int Temp;


	/* Save half the line-drawing cases by swapping Y0 with Y1
	and X0 with X1 if Y0 is greater than Y1. As a result, DeltaY
	is always > 0, and only the octant 0-3 cases need to be
	handled. */
	if (Y0 > Y1)
	{
		Temp = Y0;
		Y0 = Y1;
		Y1 = Temp;
		Temp = X0;
		X0 = X1;
		X1 = Temp;
	}

	/* Handle as four separate cases, for the four octants in which
	Y1 is greater than Y0 */
	DeltaX = X1 - X0;    /* calculate the length of the line
						 in each coordinate */
	DeltaY = Y1 - Y0;
	if (DeltaX > 0)
	{
		if (DeltaX > DeltaY)
		{
			Octant0(pixels, width, height, X0, Y0, DeltaX, DeltaY, 1, color);
		}
		else
		{
			Octant1(pixels, width, height, X0, Y0, DeltaX, DeltaY, 1, color);
		}
	}
	else
	{
		DeltaX = -DeltaX;             /* absolute value of DeltaX */
		if (DeltaX > DeltaY)
		{
			Octant0(pixels, width, height, X0, Y0, DeltaX, DeltaY, -1, color);
		}
		else
		{
			Octant1(pixels, width, height, X0, Y0, DeltaX, DeltaY, -1, color);
		}
	}

}


/* Advances the index by one vertex forward through the vertex list,
wrapping at the end of the list */
void index_forward(int &Index, struct PointListHeader *VertexList)
{
	Index = (Index + 1) % VertexList->Length;
}

/* Advances the index by one vertex backward through the vertex list,
wrapping at the start of the list */
void index_backward(int &Index, struct PointListHeader *VertexList)
{
	Index = (Index - 1 + VertexList->Length) % VertexList->Length;
}

/* Advances the index by one vertex either forward or backward through
the vertex list, wrapping at either end of the list */
void index_move(int &Index, unsigned int Direction, struct PointListHeader *VertexList)
{
	if (Direction > 0)
		Index = (Index + 1) % VertexList->Length;
	else
		Index = (Index - 1 + VertexList->Length) % VertexList->Length;
}

/* Color-fills a convex polygon. All vertices are offset by (XOffset,
YOffset). "Convex" means that every horizontal line drawn through
the polygon at any point would cross exactly two active edges
(neither horizontal lines nor zero-length edges count as active
edges; both are acceptable anywhere in the polygon), and that the
right & left edges never cross. (It's OK for them to touch, though,
so long as the right edge never crosses over to the left of the
left edge.) Nonconvex polygons won't be drawn properly. Returns 1
for success, 0 if memory allocation failed. */
int Raster2D::FillConvexPolygon(int *pixels, int width, int height, struct PointListHeader * VertexList, int Color, int XOffset, int YOffset)
{
	int i, MinIndexL, MaxIndex, MinIndexR, SkipFirst, Temp;
	int MinPoint_Y, MaxPoint_Y, TopIsFlat, LeftEdgeDir;
	int NextIndex, CurrentIndex, PreviousIndex;
	int DeltaXN, DeltaYN, DeltaXP, DeltaYP;
	struct HLineList WorkingHLineList;
	struct HLine *EdgePointPtr;
	struct Point *VertexPtr;

	/* Point to the vertex list */
	VertexPtr = VertexList->PointPtr;

	/* Scan the list to find the top and bottom of the polygon */
	if (VertexList->Length == 0)
		return(1);  /* reject null polygons */

	MaxPoint_Y = MinPoint_Y = VertexPtr[MinIndexL = MaxIndex = 0].Y;

	for (i = 1; i < VertexList->Length; i++)
	{
		if (VertexPtr[i].Y < MinPoint_Y)
			MinPoint_Y = VertexPtr[MinIndexL = i].Y; /* new top */
		else if (VertexPtr[i].Y > MaxPoint_Y)
			MaxPoint_Y = VertexPtr[MaxIndex = i].Y; /* new bottom */
	}

	if (MinPoint_Y == MaxPoint_Y)
		return(1);  /* polygon is 0-height; avoid infinite loop below */

					/* Scan in ascending order to find the last top-edge point */
	MinIndexR = MinIndexL;
	while (VertexPtr[MinIndexR].Y == MinPoint_Y)
		index_forward(MinIndexR, VertexList);

	index_backward(MinIndexR, VertexList); /* back up to last top-edge point */

							   /* Now scan in descending order to find the first top-edge point */
	while (VertexPtr[MinIndexL].Y == MinPoint_Y)
		index_backward(MinIndexL, VertexList);

	index_forward(MinIndexL, VertexList); /* back up to first top-edge point */

							  /* Figure out which direction through the vertex list from the top
							  vertex is the left edge and which is the right */
	LeftEdgeDir = -1; /* assume left edge runs down thru vertex list */
	if ((TopIsFlat = (VertexPtr[MinIndexL].X != VertexPtr[MinIndexR].X) ? 1 : 0) == 1)
	{
		/* If the top is flat, just see which of the ends is leftmost */
		if (VertexPtr[MinIndexL].X > VertexPtr[MinIndexR].X)
		{
			LeftEdgeDir = 1;  /* left edge runs up through vertex list */
			Temp = MinIndexL;       /* swap the indices so MinIndexL   */
			MinIndexL = MinIndexR;  /* points to the start of the left */
			MinIndexR = Temp;       /* edge, similarly for MinIndexR   */
		}
	}
	else
	{
		/* Point to the downward end of the first line of each of the
		two edges down from the top */
		NextIndex = MinIndexR;
		index_forward(NextIndex, VertexList);
		PreviousIndex = MinIndexL;
		index_backward(PreviousIndex, VertexList);
		/* Calculate X and Y lengths from the top vertex to the end of
		the first line down each edge; use those to compare slopes
		and see which line is leftmost */
		DeltaXN = VertexPtr[NextIndex].X - VertexPtr[MinIndexL].X;
		DeltaYN = VertexPtr[NextIndex].Y - VertexPtr[MinIndexL].Y;
		DeltaXP = VertexPtr[PreviousIndex].X - VertexPtr[MinIndexL].X;
		DeltaYP = VertexPtr[PreviousIndex].Y - VertexPtr[MinIndexL].Y;

		if (((long)DeltaXN * DeltaYP - (long)DeltaYN * DeltaXP) < 0L)
		{
			LeftEdgeDir = 1;  /* left edge runs up through vertex list */
			Temp = MinIndexL;       /* swap the indices so MinIndexL   */
			MinIndexL = MinIndexR;  /* points to the start of the left */
			MinIndexR = Temp;       /* edge, similarly for MinIndexR   */
		}
	}

	/* Set the # of scan lines in the polygon, skipping the bottom edge
	and also skipping the top vertex if the top isn't flat because
	in that case the top vertex has a right edge component, and set
	the top scan line to draw, which is likewise the second line of
	the polygon unless the top is flat */
	if ((WorkingHLineList.Length = MaxPoint_Y - MinPoint_Y - 1 + TopIsFlat) <= 0)
		return(1);  /* there's nothing to draw, so we're done */
	WorkingHLineList.YStart = YOffset + MinPoint_Y + 1 - TopIsFlat;

	/* Get memory in which to store the line list we generate */

	struct HLine line_array[4096];

	if (WorkingHLineList.Length >= 4096)
	{
		printf("Warning: Exceeded max lines\r\n");
	}

	WorkingHLineList.HLinePtr = (struct HLine *) &line_array[0];

	/* Scan the left edge and store the boundary points in the list */
	/* Initial pointer for storing scan converted left-edge coords */
	EdgePointPtr = WorkingHLineList.HLinePtr;
	/* Start from the top of the left edge */
	PreviousIndex = CurrentIndex = MinIndexL;
	/* Skip the first point of the first line unless the top is flat;
	if the top isn't flat, the top vertex is exactly on a right
	edge and isn't drawn */
	SkipFirst = TopIsFlat ? 0 : 1;

	/* Scan convert each line in the left edge from top to bottom */
	do
	{
		index_move(CurrentIndex, LeftEdgeDir, VertexList);
		ScanEdge(VertexPtr[PreviousIndex].X + XOffset,
			VertexPtr[PreviousIndex].Y,
			VertexPtr[CurrentIndex].X + XOffset,
			VertexPtr[CurrentIndex].Y, 1, SkipFirst, &EdgePointPtr);
		PreviousIndex = CurrentIndex;
		SkipFirst = 0; /* scan convert the first point from now on */
	} while (CurrentIndex != MaxIndex);

	/* Scan the right edge and store the boundary points in the list */
	EdgePointPtr = WorkingHLineList.HLinePtr;
	PreviousIndex = CurrentIndex = MinIndexR;
	SkipFirst = TopIsFlat ? 0 : 1;
	/* Scan convert the right edge, top to bottom. X coordinates are
	adjusted 1 to the left, effectively causing scan conversion of
	the nearest points to the left of but not exactly on the edge */

	do
	{
		index_move(CurrentIndex, -LeftEdgeDir, VertexList);
		ScanEdge(VertexPtr[PreviousIndex].X + XOffset - 1,
			VertexPtr[PreviousIndex].Y,
			VertexPtr[CurrentIndex].X + XOffset - 1,
			VertexPtr[CurrentIndex].Y, 0, SkipFirst, &EdgePointPtr);
		PreviousIndex = CurrentIndex;
		SkipFirst = 0; /* scan convert the first point from now on */
	} while (CurrentIndex != MaxIndex);

	/* Draw the line list representing the scan converted polygon */
	DrawHorizontalLineList(pixels, width, height, &WorkingHLineList, Color);


	return(1);
}

/* Scan converts an edge from (X1,Y1) to (X2,Y2), not including the
point at (X2,Y2). This avoids overlapping the end of one line with
the start of the next, and causes the bottom scan line of the
polygon not to be drawn. If SkipFirst != 0, the point at (X1,Y1)
isn't drawn. For each scan line, the pixel closest to the scanned
line without being to the left of the scanned line is chosen. */
void Raster2D::ScanEdge(int X1, int Y1, int X2, int Y2, int SetXStart, int SkipFirst, struct HLine **EdgePointPtr)
{
	int Y, DeltaX, DeltaY;
	double InverseSlope;
	struct HLine *WorkingEdgePointPtr;

	/* Calculate X and Y lengths of the line and the inverse slope */
	DeltaX = X2 - X1;
	if ((DeltaY = Y2 - Y1) <= 0)
		return;     /* guard against 0-length and horizontal edges */
	InverseSlope = (double)DeltaX / (double)DeltaY;

	/* Store the X coordinate of the pixel closest to but not to the
	left of the line for each Y coordinate between Y1 and Y2, not
	including Y2 and also not including Y1 if SkipFirst != 0 */
	WorkingEdgePointPtr = *EdgePointPtr; /* avoid double dereference */

	for (Y = Y1 + SkipFirst; Y < Y2; Y++, WorkingEdgePointPtr++)
	{
		/* Store the X coordinate in the appropriate edge list */
		if (SetXStart == 1)
			WorkingEdgePointPtr->XStart =
			X1 + (int)(ceil((Y - Y1) * InverseSlope));
		else
			WorkingEdgePointPtr->XEnd =
			X1 + (int)(ceil((Y - Y1) * InverseSlope));
	}
	*EdgePointPtr = WorkingEdgePointPtr;   /* advance caller's ptr */
}


/* Draws all pixels in the list of horizontal lines passed in, in
mode 13h, the VGA's 320x200 256-color mode. Uses a slow pixel-by-
pixel approach, which does have the virtue of being easily ported
to any environment. */
void Raster2D::DrawHorizontalLineList(int *pixels, int width, int height, struct HLineList * HLineListPtr, int color)
{
	struct HLine *HLinePtr;
	int Y, X;

	/* Point to the XStart/XEnd descriptor for the first (top)
	horizontal line */
	HLinePtr = HLineListPtr->HLinePtr;
	/* Draw each horizontal line in turn, starting with the top one and
	advancing one line each time */
	for (Y = HLineListPtr->YStart; Y < (HLineListPtr->YStart +
		HLineListPtr->Length); Y++, HLinePtr++) {
		/* Draw each pixel in the current horizontal line in turn,
		starting with the leftmost one */
		for (X = HLinePtr->XStart; X <= HLinePtr->XEnd; X++)
			pixels[(unsigned int)X + (unsigned int)Y * width] = color;
	}
}


/* Pointers to global edge table (GET) and active edge table (AET) */
static struct EdgeState *GETPtr, *AETPtr;


void Raster2D::DrawHorizontalLineSeg(unsigned int *pixels, int width, int height, int Y, int LeftX, int RightX, int Color)
{
	int X;

	/* Draw each pixel in the horizontal line segment, starting with
	the leftmost one */
	for (X = LeftX; X <= RightX; X++)
	{
		pixels[Y * width + X] = Color;
	}
}


/* Fills the scan line described by the current AET at the specified Y
coordinate in the specified color, using the odd/even fill rule */
void Raster2D::ScanOutAET(unsigned int *pixels, int width, int height, int YToScan, int Color)
{
	int LeftX;
	struct EdgeState *CurrentEdge;

	/* Scan through the AET, drawing line segments as each pair of edge
	crossings is encountered. The nearest pixel on or to the right
	of left edges is drawn, and the nearest pixel to the left of but
	not on right edges is drawn */
	CurrentEdge = AETPtr;
	while (CurrentEdge != NULL)
	{
		LeftX = CurrentEdge->X;
		CurrentEdge = CurrentEdge->NextEdge;
		DrawHorizontalLineSeg(pixels, width, height, YToScan, LeftX, CurrentEdge->X - 1, Color);
		CurrentEdge = CurrentEdge->NextEdge;
	}
}


// non-convex start
/* Color-fills an arbitrarily-shaped polygon described by VertexList.
If the first and last points in VertexList are not the same, the path
around the polygon is automatically closed. All vertices are offset
by (XOffset, YOffset). Returns 1 for success, 0 if memory allocation
failed. All C code tested with Borland C++.
If the polygon shape is known in advance, speedier processing may be
enabled by specifying the shape as follows: "convex" - a rubber band
stretched around the polygon would touch every vertex in order;
"nonconvex" - the polygon is not self-intersecting, but need not be
convex; "complex" - the polygon may be self-intersecting, or, indeed,
any sort of polygon at all. Complex will work for all polygons; convex
is fastest. Undefined results will occur if convex is specified for a
nonconvex or complex polygon.
Define CONVEX_CODE_LINKED if the fast convex polygon filling code from
Chapter 38 is linked in. Otherwise, convex polygons are
handled by the complex polygon filling code.
Nonconvex is handled as complex in this implementation. See text for a
discussion of faster nonconvex handling. */
int Raster2D::FillPolygon(unsigned int *pixels, int width, int height, struct PointListHeader * VertexList, int Color, int XOffset, int YOffset)
{
	struct EdgeState *EdgeTableBuffer;
	int CurrentY;

	/* It takes a minimum of 3 vertices to cause any pixels to be
	drawn; reject polygons that are guaranteed to be invisible */
	if (VertexList->Length < 3)
		return(1);
	/* Get enough memory to store the entire edge table */
	if ((EdgeTableBuffer =
		(struct EdgeState *) (malloc(sizeof(struct EdgeState) *
			VertexList->Length))) == NULL)
		return(0);  /* couldn''t get memory for the edge table */
					/* Build the global edge table */
	BuildGET(VertexList, EdgeTableBuffer, XOffset, YOffset);
	/* Scan down through the polygon edges, one scan line at a time,
	so long as at least one edge remains in either the GET or AET */
	AETPtr = NULL;    /* initialize the active edge table to empty */
	CurrentY = GETPtr->StartY; /* start at the top polygon vertex */
	while ((GETPtr != NULL) || (AETPtr != NULL)) {
		MoveXSortedToAET(CurrentY);  /* update AET for this scan line */
		ScanOutAET(pixels, width, height, CurrentY, Color); /* draw this scan line from AET */
		AdvanceAET();                /* advance AET edges 1 scan line */
		XSortAET();                  /* resort on X */
		CurrentY++;                  /* advance to the next scan line */
	}
	/* Release the memory we've allocated and we're done */
	free(EdgeTableBuffer);
	return(1);
}

/* Creates a GET in the buffer pointed to by NextFreeEdgeStruc from
the vertex list. Edge endpoints are flipped, if necessary, to
guarantee all edges go top to bottom. The GET is sorted primarily
by ascending Y start coordinate, and secondarily by ascending X
start coordinate within edges with common Y coordinates. */
void Raster2D::BuildGET(struct PointListHeader * VertexList,
	struct EdgeState * NextFreeEdgeStruc, int XOffset, int YOffset)
{
	int i, StartX, StartY, EndX, EndY, DeltaY, DeltaX, Width, temp;
	struct EdgeState *NewEdgePtr;
	struct EdgeState *FollowingEdge, **FollowingEdgeLink;
	struct Point *VertexPtr;

	/* Scan through the vertex list and put all non-0-height edges into
	the GET, sorted by increasing Y start coordinate */
	VertexPtr = VertexList->PointPtr;   /* point to the vertex list */
	GETPtr = NULL;    /* initialize the global edge table to empty */
	for (i = 0; i < VertexList->Length; i++) {
		/* Calculate the edge height and width */
		StartX = VertexPtr[i].X + XOffset;
		StartY = VertexPtr[i].Y + YOffset;
		/* The edge runs from the current point to the previous one */
		if (i == 0) {
			/* Wrap back around to the end of the list */
			EndX = VertexPtr[VertexList->Length - 1].X + XOffset;
			EndY = VertexPtr[VertexList->Length - 1].Y + YOffset;
		}
		else {
			EndX = VertexPtr[i - 1].X + XOffset;
			EndY = VertexPtr[i - 1].Y + YOffset;
		}
		/* Make sure the edge runs top to bottom */
		if (StartY > EndY) {
			SWAP(StartX, EndX);
			SWAP(StartY, EndY);
		}
		/* Skip if this can't ever be an active edge (has 0 height) */
		if ((DeltaY = EndY - StartY) != 0) {
			/* Allocate space for this edge's info, and fill in the
			structure */
			NewEdgePtr = NextFreeEdgeStruc++;
			NewEdgePtr->XDirection =   /* direction in which X moves */
				((DeltaX = EndX - StartX) > 0) ? 1 : -1;
			Width = abs(DeltaX);
			NewEdgePtr->X = StartX;
			NewEdgePtr->StartY = StartY;
			NewEdgePtr->Count = DeltaY;
			NewEdgePtr->ErrorTermAdjDown = DeltaY;
			if (DeltaX >= 0)  /* initial error term going L->R */
				NewEdgePtr->ErrorTerm = 0;
			else              /* initial error term going R->L */
				NewEdgePtr->ErrorTerm = -DeltaY + 1;
			if (DeltaY >= Width) {     /* Y-major edge */
				NewEdgePtr->WholePixelXMove = 0;
				NewEdgePtr->ErrorTermAdjUp = Width;
			}
			else {                   /* X-major edge */
				NewEdgePtr->WholePixelXMove =
					(Width / DeltaY) * NewEdgePtr->XDirection;
				NewEdgePtr->ErrorTermAdjUp = Width % DeltaY;
			}
			/* Link the new edge into the GET so that the edge list is
			still sorted by Y coordinate, and by X coordinate for all
			edges with the same Y coordinate */
			FollowingEdgeLink = &GETPtr;
			for (;;) {
				FollowingEdge = *FollowingEdgeLink;
				if ((FollowingEdge == NULL) ||
					(FollowingEdge->StartY > StartY) ||
					((FollowingEdge->StartY == StartY) &&
					(FollowingEdge->X >= StartX))) {
					NewEdgePtr->NextEdge = FollowingEdge;
					*FollowingEdgeLink = NewEdgePtr;
					break;
				}
				FollowingEdgeLink = &FollowingEdge->NextEdge;
			}
		}
	}
}

/* Sorts all edges currently in the active edge table into ascending
order of current X coordinates */
void Raster2D::XSortAET()
{
	struct EdgeState *CurrentEdge, **CurrentEdgePtr, *TempEdge;
	int SwapOccurred;

	/* Scan through the AET and swap any adjacent edges for which the
	second edge is at a lower current X coord than the first edge.
	Repeat until no further swapping is needed */
	if (AETPtr != NULL)
	{
		do
		{
			SwapOccurred = 0;
			CurrentEdgePtr = &AETPtr;
			while ((CurrentEdge = *CurrentEdgePtr)->NextEdge != NULL)
			{
				if (CurrentEdge->X > CurrentEdge->NextEdge->X)
				{
					/* The second edge has a lower X than the first;
					swap them in the AET */
					TempEdge = CurrentEdge->NextEdge->NextEdge;
					*CurrentEdgePtr = CurrentEdge->NextEdge;
					CurrentEdge->NextEdge->NextEdge = CurrentEdge;
					CurrentEdge->NextEdge = TempEdge;
					SwapOccurred = 1;
				}
				CurrentEdgePtr = &(*CurrentEdgePtr)->NextEdge;
			}
		} while (SwapOccurred != 0);
	}
}

/* Advances each edge in the AET by one scan line.
Removes edges that have been fully scanned. */
void Raster2D::AdvanceAET()
{
	struct EdgeState *CurrentEdge, **CurrentEdgePtr;

	/* Count down and remove or advance each edge in the AET */
	CurrentEdgePtr = &AETPtr;
	while ((CurrentEdge = *CurrentEdgePtr) != NULL) {
		/* Count off one scan line for this edge */
		if ((--(CurrentEdge->Count)) == 0)
		{
			/* This edge is finished, so remove it from the AET */
			*CurrentEdgePtr = CurrentEdge->NextEdge;
		}
		else
		{
			/* Advance the edge's X coordinate by minimum move */
			CurrentEdge->X += CurrentEdge->WholePixelXMove;
			/* Determine whether it's time for X to advance one extra */
			if ((CurrentEdge->ErrorTerm +=
				CurrentEdge->ErrorTermAdjUp) > 0)
			{
				CurrentEdge->X += CurrentEdge->XDirection;
				CurrentEdge->ErrorTerm -= CurrentEdge->ErrorTermAdjDown;
			}
			CurrentEdgePtr = &CurrentEdge->NextEdge;
		}
	}
}

/* Moves all edges that start at the specified Y coordinate from the
GET to the AET, maintaining the X sorting of the AET. */
void Raster2D::MoveXSortedToAET(int YToMove)
{
	struct EdgeState *AETEdge, **AETEdgePtr, *TempEdge;
	int CurrentX;

	/* The GET is Y sorted. Any edges that start at the desired Y
	coordinate will be first in the GET, so we'll move edges from
	the GET to AET until the first edge left in the GET is no longer
	at the desired Y coordinate. Also, the GET is X sorted within
	each Y coordinate, so each successive edge we add to the AET is
	guaranteed to belong later in the AET than the one just added. */
	AETEdgePtr = &AETPtr;
	while ((GETPtr != NULL) && (GETPtr->StartY == YToMove))
	{
		CurrentX = GETPtr->X;
		/* Link the new edge into the AET so that the AET is still
		sorted by X coordinate */
		for (;;)
		{
			AETEdge = *AETEdgePtr;
			if ((AETEdge == NULL) || (AETEdge->X >= CurrentX)) {
				TempEdge = GETPtr->NextEdge;
				*AETEdgePtr = GETPtr;  /* link the edge into the AET */
				GETPtr->NextEdge = AETEdge;
				AETEdgePtr = &GETPtr->NextEdge;
				GETPtr = TempEdge;   /* unlink the edge from the GET */
				break;
			}
			else
			{
				AETEdgePtr = &AETEdge->NextEdge;
			}
		}
	}
}





// this function will draw a rectangle from (x1,y1) - (x2,y2)
// it is assumed that each endpoint is within the screen boundaries
// and (x1,y1) is the upper left hand corner and (x2,y2) is the lower
// right hand corner
void Raster2D::Draw_Rectangle(unsigned int *pixels, int width, int height, int x1, int y1, int x2, int y2, int color)
{
	unsigned int *start_offset; // starting memory offset of first row
	int rect_width; // width of rectangle

					// compute starting offset of first row
	start_offset = &pixels[y1 * width + x1];

	// compute width of rectangle
	rect_width = 1 + x2 - x1; // the "1" to reflect the true width in pixels

							  // draw the rectangle row by row
	while (y1++ <= y2)
	{
		// draw the line
		for (int i = 0; i < rect_width; i++)
		{
			start_offset[i] = color;
		}
		start_offset += width;
	}
}

// this function draws a fast horizontal line by using WORD size writes
// its speed can be doubled by use of an external 32 bit DWORD version in
// assembly...
void Raster2D::Triangle_Line(unsigned int *pixels, unsigned int xs, unsigned int xe, int color)
{
	// vectorize me
	for (unsigned int i = xs; i < xe; i++)
	{
		pixels[i] = color;
	}
}

// this function draws a triangle that has a flat top
void Raster2D::Draw_Top_Triangle(unsigned int *pixels, int swidth, int sheight, int x1, int y1, int x2, int y2, int x3, int y3, int color)
{
	float dx_right, // the dx/dy ratio of the right edge of line
		dx_left, // the dx/dy ratio of the left edge of line
		xs, xe, // the starting and ending points of the edges
		height; // the height of the triangle

	int temp_x, // used during sorting as temps
		temp_y,
		right, // used by clipping
		left;

	unsigned int poly_clip_min_x = 0;
	unsigned int poly_clip_min_y = 0;
	unsigned int poly_clip_max_x = swidth - 1;
	unsigned int poly_clip_max_y = sheight - 1;

	unsigned int *dest_addr;

	// test order of x1 and x2
	if (x2 < x1)
	{
		temp_x = x2;
		x2 = x1;
		x1 = temp_x;
	}

	// compute delta's
	height = y3 - y1;
	dx_left = (x3 - x1) / height;
	dx_right = (x3 - x2) / height;

	// set starting points
	xs = (float)x1;
	xe = (float)x2 + (float)0.5;

	// perform y clipping
	if (y1 < poly_clip_min_y)
	{
		// compute new xs and ys
		xs = xs + dx_left*(float)(-y1 + poly_clip_min_y);
		xe = xe + dx_right*(float)(-y1 + poly_clip_min_y);
		// reset y1
		y1 = poly_clip_min_y;
	} // end if top is off screen

	if (y3 > poly_clip_max_y)
	{
		y3 = poly_clip_max_y;
	}

	// compute starting address in video memory
	dest_addr = &pixels[x1 + y1 * swidth];

	// test if x clipping is needed
	if (x1 >= poly_clip_min_x && x1 <= poly_clip_max_x &&
		x2 >= poly_clip_min_x && x2 <= poly_clip_max_x &&
		x3 >= poly_clip_min_x && x3 <= poly_clip_max_x)
	{
		// draw the triangle
		for (temp_y = y1; temp_y <= y3; temp_y++, dest_addr += swidth)
		{
			Triangle_Line(dest_addr, (unsigned int)xs, (unsigned int)xe, color);

			// adjust starting point and ending point
			xs += dx_left;
			xe += dx_right;
		}
	}
	else
	{
		// clip x axis with slower version
		// draw the triangle
		for (temp_y = y1; temp_y <= y3; temp_y++, dest_addr += swidth)
		{
			// do x clip
			left = (int)xs;
			right = (int)xe;

			// adjust starting point and ending point
			xs += dx_left;
			xe += dx_right;

			// clip line
			if (left < poly_clip_min_x)
			{
				left = poly_clip_min_x;

				if (right < poly_clip_min_x)
					continue;
			}

			if (right > poly_clip_max_x)
			{
				right = poly_clip_max_x;

				if (left > poly_clip_max_x)
					continue;
			}

			Triangle_Line(dest_addr, (unsigned int)left, (unsigned int)right, color);
		}
	}
}


// this function draws a triangle that has a flat bottom
void Raster2D::Draw_Bottom_Triangle(unsigned int *pixels, int swidth, int sheight, int x1, int y1, int x2, int y2, int x3, int y3, int color)
{
	float dx_right, // the dx/dy ratio of the right edge of line
		dx_left, // the dx/dy ratio of the left edge of line
		xs, xe, // the starting and ending points of the edges
		height; // the height of the triangle

	int temp_x, // used during sorting as temps
		temp_y,
		right, // used by clipping
		left;

	unsigned int *dest_addr;
	unsigned int poly_clip_min_x = 0;
	unsigned int poly_clip_min_y = 0;
	unsigned int poly_clip_max_x = swidth - 1;
	unsigned int poly_clip_max_y = sheight - 1;

	// test order of x1 and x2
	if (x3 < x2)
	{
		temp_x = x2;
		x2 = x3;
		x3 = temp_x;
	}

	// compute delta's
	height = y3 - y1;
	dx_left = (x2 - x1) / height;
	dx_right = (x3 - x1) / height;

	// set starting points
	xs = (float)x1;
	xe = (float)x1 + (float)0.5;

	// perform y clipping
	if (y1 < poly_clip_min_y)
	{
		// compute new xs and ys
		xs = xs + dx_left*(float)(-y1 + poly_clip_min_y);
		xe = xe + dx_right*(float)(-y1 + poly_clip_min_y);

		// reset y1
		y1 = poly_clip_min_y;
	} // end if top is off screen

	if (y3 > poly_clip_max_y)
	{
		y3 = poly_clip_max_y;
	}

	// compute starting address in video memory
	dest_addr = &pixels[x1 + y1 * swidth];

	// test if x clipping is needed
	if (x1 >= poly_clip_min_x && x1 <= poly_clip_max_x &&
		x2 >= poly_clip_min_x && x2 <= poly_clip_max_x &&
		x3 >= poly_clip_min_x && x3 <= poly_clip_max_x)
	{
		// draw the triangle
		for (temp_y = y1; temp_y <= y3; temp_y++, dest_addr += swidth)
		{
			Triangle_Line(dest_addr, (unsigned int)xs, (unsigned int)xe, color);

			// adjust starting point and ending point
			xs += dx_left;
			xe += dx_right;
		}
	}
	else
	{
		// clip x axis with slower version
		// draw the triangle
		for (temp_y = y1; temp_y <= y3; temp_y++, dest_addr += swidth)
		{
			// do x clip
			left = (int)xs;
			right = (int)xe;

			// adjust starting point and ending point
			xs += dx_left;
			xe += dx_right;

			// clip line
			if (left < poly_clip_min_x)
			{
				left = poly_clip_min_x;

				if (right < poly_clip_min_x)
					continue;
			}
			if (right > poly_clip_max_x)
			{
				right = poly_clip_max_x;

				if (left > poly_clip_max_x)
					continue;
			}
			Triangle_Line(dest_addr, (unsigned int)left, (unsigned int)right, color);
		}
	}
}


// this function draws a triangle on the screen
void Raster2D::Draw_Triangle(unsigned int *pixels, int width, int height, int x1, int y1, int x2, int y2, int x3, int y3, int color)
{
	int temp_x, // temporaries used for sorting
		temp_y,
		new_x;

	unsigned int poly_clip_min_x = 0;
	unsigned int poly_clip_min_y = 0;
	unsigned int poly_clip_max_x = width - 1;
	unsigned int poly_clip_max_y = height - 1;

	// test for h lines and v lines
	if ((x1 == x2 && x2 == x3) || (y1 == y2 && y2 == y3))
		return;

	// sort p1,p2,p3 in ascending y order
	if (y2 < y1)
	{
		temp_x = x2;
		temp_y = y2;
		x2 = x1;
		y2 = y1;
		x1 = temp_x;
		y1 = temp_y;
	} // end if

	  // now we know that p1 and p2 are in order
	if (y3 < y1)
	{
		temp_x = x3;
		temp_y = y3;
		x3 = x1;
		y3 = y1;
		x1 = temp_x;
		y1 = temp_y;
	}

	// finally test y3 against y2
	if (y3 < y2)
	{
		temp_x = x3;
		temp_y = y3;
		x3 = x2;
		y3 = y2;
		x2 = temp_x;
		y2 = temp_y;
	}

	// do trivial rejection tests
	if (y3 < poly_clip_min_y || y1 > poly_clip_max_y ||
		(x1 < poly_clip_min_x && x2 < poly_clip_min_x && x3 < poly_clip_min_x) ||
		(x1 > poly_clip_max_x && x2 > poly_clip_max_x && x3 > poly_clip_max_x))
	{
		return;
	}

	// test if top of triangle is flat
	if (y1 == y2)
	{
		Draw_Top_Triangle(pixels, width, height, x1, y1, x2, y2, x3, y3, color);
	}
	else if (y2 == y3)
	{
		Draw_Bottom_Triangle(pixels, width, height, x1, y1, x2, y2, x3, y3, color);
	}
	else
	{
		// general triangle that needs to be broken up along long edge
		new_x = x1 + (int)((float)(y2 - y1)*(float)(x3 - x1) / (float)(y3 - y1));

		// draw each sub-triangle
		Draw_Bottom_Triangle(pixels, width, height, x1, y1, new_x, y2, x2, y2, color);
		Draw_Top_Triangle(pixels, width, height, x2, y2, new_x, y2, x3, y3, color);
	}
}



// this function draws a line from xo,yo to x1,y1 using differential error
// terms (based on Bresenham's work)
void Raster2D::Draw_Line(unsigned char *pixels, int width, int height, int xo, int yo, int x1, int y1, unsigned char color)
{

	int dx, // difference in x's
		dy, // difference in y's
		x_inc, // amount in pixel space to move during drawing
		y_inc, // amount in pixel space to move during drawing
		error = 0, // the discriminant i.e. error i.e. decision variable
		index; // used for looping

	unsigned char *vb_start = pixels; // directly access the video
									  // buffer for speed
									  // compute horizontal and vertical deltas
	dx = x1 - xo;
	dy = y1 - yo;

	// test which direction the line is going in i.e. slope angle
	if (dx >= 0)
	{
		x_inc = 1;
	} // end if line is moving right
	else
	{
		x_inc = -1;
		dx = -dx; // need absolute value
	} // end else moving left

	  // test y component of slope
	if (dy >= 0)
	{
		y_inc = 320; // 320 bytes per line
	} // end if line is moving down
	else
	{
		y_inc = -320;
		dy = -dy; // need absolute value
	} // end else moving up

	  // now based on which delta is greater we can draw the line
	if (dx > dy)
	{
		// draw the line
		for (index = 0; index <= dx; index++)
		{
			// set the pixel
			*vb_start = color;
			// adjust the error term
			error += dy;
			// test if error has overflowed
			if (error>dx)
			{
				error -= dx;
				// move to next line
				vb_start += y_inc;
			} // end if error overflowed
			  // move to the next pixel
			vb_start += x_inc;
		} // end for
	} // end if |slope| <= 1
	else
	{
		// draw the line
		for (index = 0; index <= dy; index++)
		{
			// set the pixel
			*vb_start = color;
			// adjust the error term
			error += dx;
			// test if error overflowed
			if (error>0)
			{
				error -= dy;
				// move to next line
				vb_start += x_inc;
			} // end if error overflowed
			  // move to the next pixel
			vb_start += y_inc;
		} // end for
	} // end else |slope| > 1
} // end Draw_Line


  ///=============================================================================
  /// Function: draw_line
  ///=============================================================================
  /// Description: 
  ///
  /// This is a 2d line drawing function, can be used for wireframe rendering
  ///
  ///
  /// Returns:
  ///		None
  ///=============================================================================
void Raster2D::draw_line(unsigned int *pixels, int width, int height, int x1, int y1, int x2, int y2, int color)
{
	int i;
	int	x, y;
	float slope;
	int deltax;
	int deltay;

	iclamp(x1, 0, width - 1);
	iclamp(y1, 0, height - 1);
	iclamp(x2, 0, width - 1);
	iclamp(y2, 0, height - 1);

	deltax = fabs(x2 - x1);
	deltay = fabs(y2 - y1);

	if (deltax == 0 && deltay == 0)
		return;

	//We want x to always move right
	if (x2 - x1 < 0)
	{
		draw_line(pixels, width, height, x2, y2, x1, y1, color);
		return;
	}

	if (x2 - x1 != 0)
	{
		slope = (float)(y2 - y1) / (x2 - x1);
	}
	else
	{
		slope = 1000.0f;
	}

	if (slope > 1.0f)
	{
		//slope is greater than one, flip axis, redo everything
		if (y2 - y1 < 0)
		{
			draw_line(pixels, width, height, x2, y2, x1, y1, color);
			return;
		}

		slope = 1.0f / slope;

		for (i = 0; i <= deltay; i++)
		{
			y = y1 + i;
			x = x1 + (int)(slope * i);

			pixels[x + y * width] = color;
		}
	}
	else if (slope < -1.0f)
	{
		if (y2 - y1 < 0)
		{
			int temp;

			temp = y2;
			y2 = y1;
			y1 = temp;
			temp = x2;
			x2 = x1;
			x1 = temp;
		}

		slope = 1.0f / slope;

		for (i = 0; i <= deltay; i++)
		{
			y = y1 + i;
			x = x1 + (int)(slope * i);

			pixels[x + y * width] = color;
		}
	}
	else
	{
		for (i = 0; i <= deltax; i++)
		{
			x = x1 + i;
			y = y1 + (int)(slope * i);

			pixels[x + y * width] = color;
		}
	}
}


///=============================================================================
/// Function: flood_fill
///=============================================================================
/// Description: 
///
/// This is a crappy recursive flood fill function
///
///
/// Returns:
///		None
///=============================================================================
void Raster2D::flood_fill(unsigned int *pixels, int width, int height, int x, int y, int old_color, int new_color)
{
	if (x < width && x >= 0 && y < height && y >= 0 && pixels[x + y * width] != new_color)
	{
		pixels[x + y * width] = new_color;
		flood_fill(pixels, width, height, x + 1, y, old_color, new_color);
		flood_fill(pixels, width, height, x - 1, y, old_color, new_color);
		flood_fill(pixels, width, height, x, y + 1, old_color, new_color);
		flood_fill(pixels, width, height, x, y - 1, old_color, new_color);
	}
}


///=============================================================================
/// Function: draw_rect
///=============================================================================
/// Description: 
///
/// This draws a rectangle, not filled
///
///
/// Returns:
///		None
///=============================================================================
void Raster2D::draw_rect(unsigned int *pixels, int width, int height, float angle, int w, int l, int x, int y, int color)
{
	float	corner[4][2];
	float	corner_rotated[4][2];
	float	sn, cs;
	int		i;

	sn = sinf(angle);
	cs = cosf(angle);

	corner[0][0] = (float)(-w / 2);
	corner[0][1] = (float)(l / 2);

	corner[1][0] = (float)(w / 2);
	corner[1][1] = (float)(l / 2);

	corner[2][0] = (float)(w / 2);
	corner[2][1] = (float)(-l / 2);

	corner[3][0] = (float)(-w / 2);
	corner[3][1] = (float)(-l / 2);

	for (i = 0; i <= 3; i++)
	{
		corner_rotated[i][0] = cs * corner[i][0] - sn * corner[i][1];
		corner_rotated[i][1] = sn * corner[i][0] + cs * corner[i][1];
		corner[i][0] = corner_rotated[i][0];
		corner[i][1] = corner_rotated[i][1];
	}

	for (i = 0; i <= 3; i++)
	{
		corner[i][0] += x;
		corner[i][1] += y;
	}

	draw_line(pixels, width, height, (int)corner[0][0], (int)corner[0][1], (int)corner[1][0], (int)corner[1][1], color);
	draw_line(pixels, width, height, (int)corner[1][0], (int)corner[1][1], (int)corner[2][0], (int)corner[2][1], color);
	draw_line(pixels, width, height, (int)corner[2][0], (int)corner[2][1], (int)corner[3][0], (int)corner[3][1], color);
	draw_line(pixels, width, height, (int)corner[3][0], (int)corner[3][1], (int)corner[0][0], (int)corner[0][1], color);
}



///=============================================================================
/// Function: draw_rect
///=============================================================================
/// Description: 
///
/// This draws a rectangle, not filled, more closely resembles GDI Rectangle
/// 
///
/// Returns:
///		None
///=============================================================================
void Raster2D::Rectangle(unsigned int *vram, int xres, int yres, int x1, int y1, int x2, int y2)
{
	draw_line((unsigned int *)vram, xres, yres, x1, y1, x2, y1, 0); // top line
	draw_line((unsigned int *)vram, xres, yres, x1, y2, x2, y2, 0); // bottom line

	draw_line((unsigned int *)vram, xres, yres, x1, y1, x1, y2, 0); // left line
	draw_line((unsigned int *)vram, xres, yres, x2, y1, x2, y2, 0); // right line
}


///=============================================================================
/// Function: Triangle
///=============================================================================
/// Description: 
///
/// This draws a triangle, not filled, more closely resembles GDI Rectangle
/// 
///
/// Returns:
///		None
///=============================================================================
void Raster2D::Triangle(unsigned int *vram, int xres, int yres, int x1, int y1, int x2, int y2, int color)
{
	int half_x = abs(x2 - x1) / 2;

	draw_line((unsigned int *)vram, xres, yres, x1, y2, x2, y2, color); // bottom line
	draw_line((unsigned int *)vram, xres, yres, x1 + half_x, y1, x1, y2, color); // left line
	draw_line((unsigned int *)vram, xres, yres, x2 - half_x, y1, x2, y2, color); // right line
}



///=============================================================================
/// Function: draw_text
///=============================================================================
/// Description: 
///
/// This draws a bitmap text from a texture atlas 16x16 pixel characters
/// 256x256 total size
/// 
///
/// Returns:
///		None
///=============================================================================
void Raster2D::draw_text(unsigned int *pixels, int width, int height, char *msg, int x, int y, unsigned int *tex, int flip)
{
	int x_shift = 0;

	for (int c = 0; ; c++)
	{
		if (msg[c] == '\0')
			break;

		//256x256 font image, 16x16 fonts
		int row = msg[c] / 16;
		int col = msg[c] % 16;

		// have a valid character, copy pixels into vram x,y pos (assuming 16 pixel fonts)
		for (int j = 0; j < 16; j++)
		{
			int py = (row * 16 + j);

			for (int i = 0; i < 16; i++)
			{
				int px = (col * 16 + i);

				if ((x + i + x_shift) >= width)
					break;

				// bitmap is upside down, so invert Y
				if (flip)
				{
					pixels[(x + i + x_shift) + (y + j) * width] |= ~tex[((255 - py) * 256) + px];
//					pixels[(x + i + x_shift) + (y + j) * width] = tex[((255 - py) * 256) + px];
				}
				else
				{
					pixels[(x + i + x_shift) + (255 - (y + j)) * width] |= ~tex[((255 - py) * 256) + px];
//					pixels[(x + i + x_shift) + (255 - (y + j)) * width] = tex[((255 - py) * 256) + px];
				}

			}
		}

		// move over to the right 16 pixels for next character
		x_shift += 16;
	}
}


///=============================================================================
/// Function: draw_circle
///=============================================================================
/// Description: 
///
/// This draws a a circle, probably need to test if it still works
/// looks like it might do a filled circle too
/// 
///
/// Returns:
///		None
///=============================================================================
void Raster2D::draw_circle(unsigned int *pixels, int width, int height, int xc, int yc, int radius, int color, int filled)
{
	int x = radius - 1;
	int y = 0;
	int dx = 1;
	int dy = 1;
	int err = dx - (radius << 1);

	while (x >= y)
	{
		if (filled == 0)
		{
			pixels[xc + x + (yc + y) * width] = color;
			pixels[xc - x + (yc + y) * width] = color;
			pixels[xc + y + (yc + x) * width] = color;
			pixels[xc - y + (yc + x) * width] = color;

			pixels[xc - x + (yc - y) * width] = color;
			pixels[xc + x + (yc - y) * width] = color;
			pixels[xc + y + (yc - x) * width] = color;
			pixels[xc - y + (yc - x) * width] = color;
		}
		else
		{
			draw_line(pixels, width, height,
				xc + x, yc + y,
				xc - x, yc + y,
				color);
			draw_line(pixels, width, height,
				xc + x, yc - y,
				xc - x, yc - y,
				color);


			draw_line(pixels, width, height,
				xc + y, yc + x,
				xc - y, yc + x,
				color);
			draw_line(pixels, width, height,
				xc + y, yc - x,
				xc - y, yc - x,
				color);
		}

		if (err <= 0)
		{
			y++;
			err += dy;
			dy += 2;
		}

		if (err > 0)
		{
			x--;
			dx += 2;
			err += dx - (radius << 1);
		}
	}
}



///=============================================================================
/// Function: draw_ellipse
///=============================================================================
/// Description: 
///
/// This draws a a ellipse, probably need to test if it still works
/// looks like it might do a filled circle too
/// 
///
/// Returns:
///		None
///=============================================================================
void Raster2D::draw_ellipse(unsigned int *pixels, int width, int height, int xc, int yc, int rx, int ry, int color, int filled)
{
	int x, y, p;

	x = 0;
	y = ry;
	p = (ry * ry) - (rx * rx * ry) + ((rx * rx) / 4);

	while ((2 * x * ry * ry) < (2 * y * rx * rx))
	{
		if (filled == 0)
		{
			pixels[xc + x + (yc - y) * width] = color;
			pixels[xc - x + (yc + y) * width] = color;
			pixels[xc + x + (yc + y) * width] = color;
			pixels[xc - x + (yc - y) * width] = color;
		}
		else
		{
			draw_line(pixels, width, height,
				xc + x, yc + y,
				xc - x, yc + y,
				color);
			draw_line(pixels, width, height,
				xc + x, yc - y,
				xc - x, yc - y,
				color);
		}

		if (p < 0)
		{
			x = x + 1;
			p = p + (2 * ry * ry * x) + (ry * ry);
		}
		else
		{
			x = x + 1;
			y = y - 1;
			p = p + (2 * ry * ry * x + ry * ry) - (2 * rx * rx * y);
		}
	}
	p = (int)(((float)x + 0.5f) * ((float)x + 0.5f) * ry * ry + (y - 1) * (y - 1) * rx * rx - rx * rx * ry * ry);

	while (y >= 0)
	{
		if (filled == 0)
		{
			pixels[xc + x + (yc - y) * width] = color;
			pixels[xc - x + (yc + y) * width] = color;
			pixels[xc + x + (yc + y) * width] = color;
			pixels[xc - x + (yc - y) * width] = color;
		}
		else
		{
			draw_line(pixels, width, height,
				xc + x, yc + y,
				xc - x, yc + y,
				color);
			draw_line(pixels, width, height,
				xc + x, yc - y,
				xc - x, yc - y,
				color);
		}

		if (p>0)
		{
			y = y - 1;
			p = p - (2 * rx * rx * y) + (rx * rx);

		}
		else
		{
			y = y - 1;
			x = x + 1;
			p = p + (2 * ry * ry * x) - (2 * rx * rx * y) - (rx * rx);
		}
	}
}



void Raster2D::BitBlt(unsigned int *pixels, int width, int height,
	int x, int y,
	unsigned int *sprite, unsigned int sprite_width, unsigned int sprite_height, int mode)
{
	for (int j = 0; j <= sprite_height - 1; j++)
	{
		for (int i = 0; i <= sprite_width - 1; i++)
		{
			if (mode)
				pixels[x + i + (y + j) * width] |= sprite[(j * sprite_width) + i];
			else
				pixels[x + i + (y + j) * width] = sprite[(j * sprite_width) + i];
		}
	}
}

#ifdef FREETYPE_H
void Raster2D::draw_string(unsigned int *pixels, int width, int height, FreeType &font, int x_pos, int y_pos, char *str)
{
	font_t c;

	unsigned int length = strlen(str);
	for (unsigned int i = 0; i < length; i++)
	{
		font.getBitmap32(str[i], c);
		x_pos += c.left;
		BitBlt(pixels, width, height, x_pos, y_pos, (unsigned int *)c.data, c.px, c.py, 1);
		x_pos += c.advance;
		free((void *)c.data);
	}
}
#endif
