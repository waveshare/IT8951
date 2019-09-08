#include "IT8951.h"
#include <png.h>

void abort_(const char * s)
{
    printf("%s\n", s);
    abort();
}

uint8_t *read_png_file(char* file_name, int* width_ptr, int* height_ptr, png_byte *color_type_ptr, png_byte *bit_depth_ptr, uint8_t *buffer_to_write)
{
    char header[8];    // 8 is the maximum size that can be checked

    /* open file and test for it being a png */
    FILE *fp = fopen(file_name, "rb");
    if (!fp)
        abort_("[read_png_file] File %s could not be opened for reading");
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8))
        abort_("[read_png_file] File %s is not recognized as a PNG file");


    /* initialize stuff */
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
        abort_("[read_png_file] png_create_read_struct failed");

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        abort_("[read_png_file] png_create_info_struct failed");

    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[read_png_file] Error during init_io");

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    *width_ptr = png_get_image_width(png_ptr, info_ptr);
    *height_ptr = png_get_image_height(png_ptr, info_ptr);
    *color_type_ptr = png_get_color_type(png_ptr, info_ptr);
    *bit_depth_ptr = png_get_bit_depth(png_ptr, info_ptr);

    int number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    /* read file */
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[read_png_file] Error during read_image");


    png_bytep * row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * *height_ptr);
    png_bytep all_bytes = (png_bytep)buffer_to_write;
    int row_size = png_get_rowbytes(png_ptr,info_ptr);
    for (int y=0; y<*height_ptr; y++)
        row_pointers[y] = all_bytes + (y * row_size);

    png_read_image(png_ptr, row_pointers);

    free(row_pointers);
    fclose(fp);

    return (uint8_t *)all_bytes;
}


int main (int argc, char *argv[])
{
    uint8_t *buffer_to_write;
    printf("init start\n");
	if(!(buffer_to_write = IT8951_Init()))
	{
		printf("IT8951_Init error \n");
		return 1;
	}
    printf("init end\n");
	
	if (argc < 2)
	{
		printf("Error: argc!=2.\n");
		exit(1);
	}

    png_byte color_type;
    png_byte bit_depth;
    int width, height;

    for (int i = 0; i < 10; ++i) {
        printf("read file start\n");
        uint8_t *buffer = read_png_file(argv[i % 2 + 1], &width, &height, &color_type, &bit_depth, buffer_to_write);
        printf("read file end\n");
        printf("update screen start\n");
        IT8951_Display4BppBuffer();
        printf("update screen end\n");
    }

	
	IT8951_Cancel();

	return 0;
}


