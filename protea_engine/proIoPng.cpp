#include "proIoPng.h"
#include "proResource.h"

extern "C" {
#include <png.h>
}
#include <string>
#include <cstdio>
using namespace std;

Image* ioPng::load(const string & filename) {
    FILE *fp;
#ifdef _MSC_VER
    if (fopen_s(&fp, filename.c_str(), "rb")!=0) { // open the file
#else
    if ((fp = fopen(filename.c_str(), "rb")) == NULL) { // open the file
#endif
        fprintf(stderr, "ioPng::load ERROR: open \"%s\" failed.\n", filename.c_str());
        return 0;
    }
		
    // setup the PNG data structures and initialize:
    png_structp pp = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    png_infop info = png_create_info_struct(pp);
    png_init_io(pp, fp);

    // get image dimensions:
    png_read_info(pp, info);
    if (info->color_type == PNG_COLOR_TYPE_PALETTE) png_set_expand(pp);
    unsigned int depth = (info->color_type & PNG_COLOR_MASK_COLOR) ?  3 : 1;    
    if ((info->color_type & PNG_COLOR_MASK_ALPHA) || info->num_trans) ++depth;
    unsigned int width  = (unsigned int)(info->width);
    unsigned int height = (unsigned int)(info->height);
	if(!width||!height||!depth) return 0;
	// convert to grayscale or RGB:
    if (info->bit_depth < 8) {
        png_set_packing(pp);
        png_set_expand(pp);
    }
    else if (info->bit_depth == 16) png_set_strip_16(pp);

    // handle transparency:
    if (png_get_valid(pp, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(pp);

    unsigned char* data = new unsigned char[width * height * depth];
    if(!data) {
        fprintf(stderr, "ioPng::load() ERROR: could not allocate image memory.\n");
        return 0;
    }

    // allocate and assign row pointers:
    png_bytep  *rows = new png_bytep[height];
    unsigned int i;
    for (i = 0; i < height; ++i) 
		rows[height-1-i] = (png_bytep)(data + i * width * depth);
    // read the image, handle interlacing:
    for (i = png_set_interlace_handling(pp); i > 0; --i)
        png_read_rows(pp, rows, 0, height);
    delete[] rows;

    png_read_end(pp, info);
    png_destroy_read_struct(&pp, &info, 0);
    fclose(fp);
    return new Image(data, width, height, depth);
}

static void PNG_write(png_structp png_ptr, png_bytep data, png_size_t length) {
	FILE* fp = (FILE*)png_get_io_ptr(png_ptr);
	if (fwrite(data, 1, length, fp) != length)
	  png_error(png_ptr, "Write error");
}

static void PNG_flush(png_structp png_ptr) { }

bool ioPng::save(const Image& img, const std::string & filename) {
	if(!img.width()||!img.height()||!img.depth()||!img.data()) return false;
		
    // create write struct
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0,0,0);
    if (!png_ptr) return false;
    // error handling
    if (setjmp(png_jmpbuf(png_ptr))) {
      png_destroy_write_struct(&png_ptr, 0);
      return false;
    }

    // create info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
      png_destroy_write_struct(&png_ptr, 0);
      return false;
    }

	// open  file handle
    FILE *fp;
#ifdef _MSC_VER
    if (fopen_s(&fp, filename.c_str(), "wb")!=0) { // open the file for writing
#else
    if ((fp = fopen(filename.c_str(), "wb")) == NULL) { // open the file for writing
#endif
        fprintf(stderr, "ioPng::save ERROR: \"%s\" file error.\n", filename.c_str());
        return false;
    }
    // set callbacks
    png_set_write_fn(png_ptr, fp, PNG_write, PNG_flush);

    int width  = img.width();
    int height = img.height();

    png_set_IHDR(
      png_ptr, info_ptr,
      width, height, 8, 
	  (img.depth()==4) ? PNG_COLOR_TYPE_RGB_ALPHA : (img.depth()==3) ? PNG_COLOR_TYPE_RGB : (img.depth()==2) ? PNG_COLOR_TYPE_GRAY_ALPHA : PNG_COLOR_TYPE_GRAY,
      PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_DEFAULT,
      PNG_FILTER_TYPE_DEFAULT);

    // build rows
    const unsigned char** rows = (const unsigned char**)png_malloc(png_ptr, sizeof(const unsigned char*) * height);
    for (int i = 0; i < height; ++i)
      rows[height-1-i] = &img.data()[i*img.depth() * width];
    png_set_rows(png_ptr, info_ptr, (png_bytepp)rows);
    info_ptr->valid |= PNG_INFO_IDAT;

    // write image
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, 0);
	
    // cleanup memory
    png_free(png_ptr, rows);
    png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(fp);
    return true;
}
