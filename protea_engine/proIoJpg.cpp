#include "proIoJpg.h"
#include "proResource.h"

extern "C" {
#if (defined _WIN32 || defined __WIN32__) && defined FAR
#  undef FAR
#endif
#  include <jpeglib.h>
}
#include <string>
#include <cstdio>
#include <iostream>

using namespace std;

Image* ioJpg::load(const string & filename) {
    FILE *fp;
#ifdef _MSC_VER
    if (fopen_s(&fp, filename.c_str(), "rb")!=0) { // open the file
#else
    if ((fp = fopen(filename.c_str(), "rb")) == NULL) { // open the file
#endif
        cerr << "ioJpg::load ERROR: open \"" << filename << "\" failed.\n";
        return 0;
    }
	
    struct jpeg_decompress_struct cinfo;		// decompressor info
    struct jpeg_error_mgr jerr;		// error handler info
    JSAMPROW row;		// sample row pointer

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, fp);
    jpeg_read_header(&cinfo, 1);

    cinfo.quantize_colors      = (boolean)FALSE;
    cinfo.out_color_space      = JCS_RGB;
    cinfo.out_color_components = 3;
    cinfo.output_components = 3;
    jpeg_calc_output_dimensions(&cinfo);
	if(!cinfo.output_width || !cinfo.output_height) return 0;
    
    unsigned char* data = new unsigned char[cinfo.output_width * cinfo.output_height * cinfo.output_components];
    if(!data) {
        cerr << "ioJpg::load() ERROR: could not allocate image memory.\n";
        return 0;
    }

    jpeg_start_decompress(&cinfo);
    while (cinfo.output_scanline < cinfo.output_height) {
        row = (JSAMPROW)(data +  cinfo.output_width * cinfo.output_components *
			(cinfo.output_height - cinfo.output_scanline - 1));
        jpeg_read_scanlines(&cinfo, &row, (JDIMENSION)1);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
	
    fclose(fp);
    return new Image(data, cinfo.output_width, cinfo.output_height, cinfo.output_components);
}

bool ioJpg::save(const Image& img, const std::string & filename) {
	if(!img.width()||!img.height()||!img.data()) return false;
		if((img.depth()!=3)&&(img.depth()!=1)) return false;

	// open  file handle
    FILE *fp;
#ifdef _MSC_VER
    if (fopen_s(&fp, filename.c_str(), "wb")!=0) { // open the file for writing
#else
    if ((fp = fopen(filename.c_str(), "wb")) == NULL) { // open the file for writing
#endif
        fprintf(stderr, "ioJpg::save ERROR: \"%s\" file error.\n", filename.c_str());
        return false;
    }
	
	struct jpeg_error_mgr jerr;	
	struct jpeg_compress_struct cinfo;
	cinfo.err = jpeg_std_error( &jerr );
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, fp);

	cinfo.image_width = img.width();	
	cinfo.image_height = img.height();
	cinfo.input_components = img.depth();
	cinfo.in_color_space = (img.depth()==3) ? JCS_RGB : JCS_GRAYSCALE;
    
	jpeg_set_defaults( &cinfo ); // default compression parameters
	jpeg_start_compress( &cinfo, TRUE );
	JSAMPROW row_pointer[1];
	while( cinfo.next_scanline < cinfo.image_height )	{
		row_pointer[0] = (JSAMPLE*)&img.data()[ (cinfo.image_height-1-cinfo.next_scanline) * cinfo.image_width *  cinfo.input_components];
		jpeg_write_scanlines( &cinfo, row_pointer, 1 );
	}
	jpeg_finish_compress( &cinfo );
	jpeg_destroy_compress( &cinfo );
	
	fclose(fp);
    return true;
}
