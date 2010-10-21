#include "xi_image.h"
#define PNG_USER_MEM_SUPPORTED
#include <libpng12/png.h>
#include <assert.h>
#include <proto/exec.h>

unsigned long XGetPixel(XImage *xi, unsigned int x, unsigned int y)
{
	return xi->data[y*xi->width + x];
}

void XPutPixel(XImage *xi, unsigned int x, unsigned int y, unsigned long pixel)
{
	xi->data[y*xi->width + x] = pixel;
}

void XDestroyImage(XImage *xi)
{
	if (xi->data) IExec->FreeVec(xi->data);
	IExec->FreeVec(xi);
}

png_voidp user_malloc(png_structp png_ptr, png_size_t size)
{
	return IExec->AllocVec(size, MEMF_CLEAR);
}

void user_free(png_structp png_ptr, png_voidp ptr)
{
	IExec->FreeVec(ptr);
}

static void user_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	MemoryStream* stream = (MemoryStream*)png_ptr->io_ptr;
	IExec->CopyMem((APTR*)(stream->memory_location + stream->position), (APTR*)data, length);
	stream->position += length;
}

XImage* inmemory_png_to_ximage(unsigned char *png_mem)
{
	png_structp png;
	png_infop startinfo;
	png_infop endinfo;
	png_bytep row_pointer;
	png_uint_32 width, height;
	int interlace_type;
	int compression_type;
	int filter_type;
	int bit_depth;
	int color_type;
	int y;

	XImage *xi = NULL;

	if(png_sig_cmp((png_byte*)png_mem, 0, 8) == 0) //  if 0 then it is a png
	{
		xi = IExec->AllocVec(sizeof(XImage), MEMF_CLEAR);
		MemoryStream* stream = IExec->AllocVec(sizeof(MemoryStream), MEMF_CLEAR);
		stream->memory_location = png_mem;
		stream->position = 0;
		png = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL, NULL, user_malloc, user_free);
		startinfo = png_create_info_struct(png);
		endinfo = png_create_info_struct(png);
		png_set_read_fn(png, stream, user_read_data);

		png_read_info(png, startinfo);

		png_get_IHDR(png, startinfo, &width, &height, &bit_depth, &color_type,
		     &interlace_type, &compression_type, &filter_type);

		xi->width = width;
		xi->height = height;

		assert(interlace_type == PNG_INTERLACE_NONE);

		if (bit_depth < 8)
		{
			png_set_gray_1_2_4_to_8(png);
			bit_depth = 8;
		}
		else if (bit_depth > 8)
		{
			png_set_strip_16(png);
		}

		if (color_type == PNG_COLOR_TYPE_PALETTE)
		{
			png_set_palette_to_rgb(png);
			color_type = PNG_COLOR_TYPE_RGB;
		}

		if (color_type == PNG_COLOR_TYPE_RGB)
		{
			png_set_filler(png, 0x00, PNG_FILLER_AFTER);
		}

		png_read_update_info(png, startinfo);

		xi->bytes_per_line = xi->width * 4;
		xi->data = IExec->AllocVec(xi->bytes_per_line * xi->height, MEMF_CLEAR);
		row_pointer = (png_bytep)(xi->data + xi->width * (xi->height - 1));

		for (y = 0; y < xi->height; y++)
		{
			png_read_row(png, row_pointer, NULL);
			row_pointer -= xi->bytes_per_line;
		}

		png_destroy_info_struct(png, &startinfo);
		png_destroy_info_struct(png, &endinfo);
		png_destroy_read_struct(&png, NULL, NULL);

		IExec->FreeVec(stream);
	}

	return xi;
}
