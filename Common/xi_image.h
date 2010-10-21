#ifndef __XIMAGE__
#define __XIMAGE__

#include <exec/types.h>

typedef struct {
	int width;
	int height;
	int bytes_per_line;
	unsigned long *data;
} XImage;

typedef struct {
	APTR	memory_location;
	uint32	position;
} MemoryStream;

extern unsigned long XGetPixel(XImage *xi, unsigned int x, unsigned int y);
extern void XPutPixel(XImage *xi, unsigned int x, unsigned int y, unsigned long pixel);
extern void XDestroyImage(XImage *xi);
extern XImage* inmemory_png_to_ximage(unsigned char *png_mem);

#endif
