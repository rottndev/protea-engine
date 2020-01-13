#include <cstring>
#include <cstdlib>
#include <cstdio>

#ifdef _HAVE_GL
# ifdef _MSC_VER
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
# endif
# include <GL/gl.h>
# include <GL/glu.h>
#endif

#include "proResource.h"
#include "proIo.h"
#include "proStr.h"

using namespace std;

Color Color::null(1.0f,0.0f,1.0f,-1.0f);
Color Color::transparent(1.0f,1.0f,1.0f,0.0f);

//--- class Image ----------------------------------------------------

Image* Image::loadTGA(const string & name) {
    FILE *file;
    if (!(file = fopen (name.c_str(), "rb"))) {
        fprintf(stderr, "ERROR Image::loadTGA: file \"%s\" not found.\n", name.c_str());
        return 0;
    }
    // read tga header:
    fseek (file, 0, SEEK_SET); // seek begin
    unsigned char id_length;
    fread ( &id_length,       ( sizeof (unsigned char )), 1, file );
    unsigned char color_map_type;
    fread ( &color_map_type, ( sizeof (unsigned char )), 1, file );
    unsigned char image_type=0;
    fread ( &image_type,      ( sizeof (unsigned char )), 1, file );

    short int     color_map_first_entry;
    short int     color_map_length;
    unsigned char color_map_entry_size;
    fread ( &color_map_first_entry, ( sizeof (short int )), 1, file );
    fread ( &color_map_length     , ( sizeof (short int )), 1, file );
    fread ( &color_map_entry_size , ( sizeof (unsigned char )), 1, file );
    short int     x_origin, y_origin;
    fread ( &x_origin , ( sizeof (short int )), 1, file );
    fread ( &y_origin , ( sizeof (short int )), 1, file );
    short int w, h;
    fread ( &w,   ( sizeof (short int )), 1, file );
    fread ( &h,  ( sizeof (short int )), 1, file );
    unsigned char pixelDepth;
    fread ( &pixelDepth,     ( sizeof (unsigned char )), 1, file );
    unsigned char image_descriptor;
    fread ( &image_descriptor,( sizeof (unsigned char )), 1, file );
    unsigned int width = (unsigned int) w;
    unsigned int height= (unsigned int) h;
    unsigned int depth = pixelDepth / 8;

    if (image_type != 1 && image_type != 2 && image_type != 3 && image_type != 9 && image_type != 10 && image_type != 11) {
        fprintf(stderr, "ERROR Image::loadTGA: file has wrong tga format %i\n", (int)image_type);
        return 0;
    }
    unsigned char * colorMap=0;
    unsigned int mapSize=0;
    if(color_map_type==1) {
        mapSize=color_map_length*color_map_entry_size/8;
        colorMap=new unsigned char[mapSize];
        if(fread (colorMap, sizeof (unsigned char), mapSize,file) != mapSize) {
            if(colorMap) delete [] colorMap;
            fclose(file);
            return 0;
        }
    }

    fseek (file, 18+mapSize, SEEK_SET); // seek begin of pixel data part
    unsigned int size=width*height*(depth!=2 ? depth : 3);
    unsigned char* data = new unsigned char[size];
    if (!data) return 0;

    if (image_type == 1 || image_type == 2 || image_type == 3 ) { // uncompressed data
        if(depth!=2) {
            if(fread (data, sizeof (unsigned char), size,file) != size) {
                delete [] data;
                if(colorMap) delete [] colorMap;
                fclose(file);
                return 0;
            }
            if (depth >= 3) // convert BGR(A) to RGB(A):
                for (unsigned int i = 0; i < size; i += depth ) {
                    char ch = data[i];
                    data[i] = data[i + 2];
                    data[i + 2] = ch;
                }
        }
        else { // 16 bit per pixel
            unsigned char * pImg = new unsigned char[width*height*depth];
            if(fread (pImg, sizeof (unsigned char), width*height*depth,file) != size) {
                delete [] pImg;
                delete [] data;
                if(colorMap) delete [] colorMap;
                fclose(file);
                return 0;
            }
            for (unsigned int i = 0; i*3 < size; ++i ) {
                unsigned short color = *((unsigned short *)(&pImg[i*2]));
                data[i*3]   = ((color & 0x7C00) >> 10) << 3;	// b->r
                data[i*3+1] = ((color & 0x03E0) >>  5) << 3;	// g->g
                data[i*3+2] = ((color & 0x001F) >>  0) << 3;	// r->b
            }
	    depth=3;
            delete [] pImg;
        }
    }
    else if (image_type == 9 || image_type == 10 || image_type == 11 ) { // compressed data
        unsigned int currByte = 0;
        while ( currByte < size ) {
            unsigned char  header;  // RLE package header
            unsigned char  tmpColor[4] = { 0x00, 0x00, 0x00, 0x00 };

            fread ( &header, ( sizeof ( unsigned char )), 1, file );
            int runLength = ( header&0x7F ) + 1;

            if(currByte+runLength > (unsigned int)width*height*depth) {
                runLength=((unsigned int)width*height*depth-currByte)/depth;
                fprintf(stderr, "WARNING Image::loadTGA: cannot write beyond size of compressed image\n");
            }

            if ( header&0x80 ) { // RLE packet
                fread ( tmpColor, ( sizeof ( unsigned char )* depth ), 1, file );
                if ( depth == 1 ) {
                    memset ( data + currByte, tmpColor[0], runLength );
                    currByte += runLength;
                }
                else if(depth == 2) {
                    unsigned short color = *((unsigned short *)tmpColor);
                    tmpColor[0]=((color & 0x7C00) >> 10) << 3;
                    tmpColor[1]=((color & 0x7C00) >>  5) << 3;
                    tmpColor[2]=((color & 0x7C00) >>  0) << 3;
                    for ( int i = 0; i < runLength; i++ ) {
                        data[currByte++] = tmpColor[0];
                        data[currByte++] = tmpColor[1];
                        data[currByte++] = tmpColor[2];
                    }
                }
                else for ( int i = 0; i < runLength; i++ ) {
                    data[currByte++] = tmpColor[2];
                    data[currByte++] = tmpColor[1];
                    data[currByte++] = tmpColor[0];
                    if (depth==4) data[currByte++] = tmpColor[3];
                }
            }
            else for ( int i = 0; i < runLength; i++ ) {  // RAW packets
                fread ( tmpColor, ( sizeof ( unsigned char )* depth ), 1, file );
                if ( depth == 1 )
                    data[currByte++]=tmpColor[0];
                else if(depth == 2) {
                    unsigned short color = *((unsigned short *)tmpColor);
                    data[currByte++] = ((color & 0x7C00) >> 10) << 3;
                    data[currByte++] = ((color & 0x03E0) >>  5) << 3;
                    data[currByte++] = ((color & 0x001F) >>  0) << 3;
                }
                else {
                    data[currByte++] = tmpColor[2];
                    data[currByte++] = tmpColor[1];
                    data[currByte++] = tmpColor[0];
                    if (depth==4) data[currByte++] = tmpColor[3];
                }
            }
        }
    }
    fclose (file);

    if(colorMap && depth==1 && color_map_entry_size>23) { // replace color map ids by color values:
        depth=color_map_entry_size/8;
        size=width*height*depth;
        unsigned char * pImg=new unsigned char[size];
        unsigned char * currByte=pImg;
        for (unsigned int i=0;i<width*height;++i) {
            *currByte++ = colorMap[data[i]*depth+2];
            *currByte++ = colorMap[data[i]*depth+1];
            *currByte++ = colorMap[data[i]*depth];
            if (depth==4) *currByte++ = colorMap[data[i]*depth+3];
        }
        delete [] colorMap;
        delete [] data;
        data=pImg;
    }

    if (!data) {
        fprintf(stderr,"ERROR Image::loadTGA: image contained no data.\n");
        return 0;
    }
    
    Image* pImg= new Image(data,width,height,depth);
    if(image_descriptor & 32) pImg->flipV(); // image is stored upside down, correct this now 
    return pImg;
}

void Image::flipV() { 
	unsigned int lineLength=m_width*m_depth;
	unsigned int size=m_width*m_height*m_depth;
	unsigned char *lineBuffer= new unsigned char[lineLength];
	for (unsigned int i=0;i< (m_height*lineLength/2);i+=lineLength) {
		memcpy(lineBuffer,mp_data+i,lineLength);
		memcpy(mp_data+i,mp_data+size-i-lineLength,lineLength);
		memcpy(mp_data+size-i-lineLength,lineBuffer,lineLength);
	}
	delete[] lineBuffer;
}

/** adapted from glfw, http://glfw.sf.net, reuse permitted by its zlib license */
unsigned char * Image::upsample(int w2, int h2) const {
	int bpp = m_depth;
	unsigned char* data = (unsigned char*)malloc(w2*h2*bpp);
	unsigned char* dst = data;
    int m, n, k, x, y, col8;
    float dx, dy, xstep, ystep, col, col1, col2;
    unsigned char *src1, *src2, *src3, *src4;

    // Calculate scaling factor
    xstep = (float)(m_width-1) / (float)(w2-1);
    ystep = (float)(m_height-1) / (float)(h2-1);

    // Copy source data to destination data with bilinear interpolation
    dy = 0.0f;
    y = 0;
    for( n = 0; n < h2; n ++ ) {
        dx = 0.0f;
        src1 = &mp_data[ y*m_width*bpp ];
        src3 = y < (int)(m_height-1) ? src1 + m_width*bpp : src1;
        src2 = src1 + bpp;
        src4 = src3 + bpp;
        x = 0;
        for( m = 0; m < w2; m ++ ) {
            for( k = 0; k < bpp; k ++ ) {
                col1 = *src1 ++;
                col2 = *src2 ++;
                col = col1 + (col2 - col1) * dx;
                col1 = *src3 ++;
                col2 = *src4 ++;
                col2 = col1 + (col2 - col1) * dx;
                col += (col2 - col) * dy;
                col8 = (int) (col + 0.5);
                if( col8 >= 256 ) col8 = 255;
                *dst++ = (unsigned char) col8;
            }
            dx += xstep;
            if( dx >= 1.0f ) {
                x ++;
                dx -= 1.0f;
                if( x >= (int)(m_width-1) ) {
                    src2 = src1;
                    src4 = src3;
                }
            }
            else {
                src1 -= bpp;
                src2 -= bpp;
                src3 -= bpp;
                src4 -= bpp;
            }
        }
        dy += ystep;
        if( dy >= 1.0f ) {
            y ++;
            dy -= 1.0f;
        }
    }
    return data;
}

bool Image::saveTGA(const string & filename) const {
	const size_t headerSize = 18;
	unsigned char hdr[headerSize];
	memset(hdr, 0, headerSize);
    hdr [2] = (m_depth==1) ? 3 : 2; // either RGB or gray uncompreesd
    hdr[12] = m_width & 0x00FF; // image width
    hdr[13] =(m_width & 0xFF00) / 256;
    hdr[14] = m_height& 0x00FF; // image height
    hdr[15] =(m_height& 0xFF00) / 256;
    hdr[16] = m_depth*8; // pixel depth (8,24,32)

	FILE *file;
	if (!(file = fopen (filename.c_str(), "wb"))) {
		fprintf(stderr, "ERROR Image::save(): \"%s\" write file error!\n", filename.c_str());
		return false;
	}
	unsigned int i;
	fwrite(hdr, headerSize, 1, file); // write header
	switch(m_depth) { // write pixel data:
	default: // gray
      fwrite(mp_data, m_height*m_width, 1, file);
      break;
	case 3:  // rgb
      for(i=0; i<m_height*m_width; i++) {
          fputc(mp_data[i*3+2], file);
          fputc(mp_data[i*3+1], file);
          fputc(mp_data[i*3+0], file);
      }
      break;
	case 4:  // rgba
      for(i=0; i<m_height*m_width; i++) {
          fputc(mp_data[i*4+2], file);
          fputc(mp_data[i*4+1], file);
          fputc(mp_data[i*4+0], file);
          fputc(mp_data[i*4+3], file);
      }
	}
	fclose(file);
	return true;
}


//--- image from memory data ---------------------------------------
Image* Image::createFromXPM(char **xpm) {
    unsigned int width, height, depth; // image dimensions
    unsigned int nColors, charsPerPixel;
    sscanf(xpm[0],"%d%d%d%d", &width, &height, &nColors, &charsPerPixel);
    //cout << width << ' ' << height << ' ' << nColors << ' ' << charsPerPixel << endl;
    // read colors:
	bool isGrayScale = true;
    map<string,unsigned int> colorMap;
    unsigned int i;
    for(i=0; i<nColors; ++i) {
        //cout << "[" << xpm[i+1] << "]" << endl;
        vector<string> vStr;
        split(xpm[i+1],vStr);
        if(vStr.size()>2) {
            string colStr(vStr[2]);
            unsigned int color=0;
            if(colStr[0]=='#') colStr=colStr.substr(1);
            if((toLower(colStr)!="none")&&(toLower(colStr)!="background")) {
                if((colStr.size()!=6)&&(colStr.size()!=12)) {
                    fprintf(stderr, "Image::createFromXPM() WARNING: color map value has incorrect length.\n");
                    continue;
                }
                unsigned int szChunk=colStr.size()/3;
                unsigned int r=hex2ui(colStr.substr(0,2));
                unsigned int g=hex2ui(colStr.substr(szChunk,2));
                unsigned int b=hex2ui(colStr.substr(2*szChunk,2));
				if((r!=g)||(r!=b)) isGrayScale=false;
                // FIXME: take care of endian and byte order issues
                color= 255*256*256*256 + b*256*256 + g*256 + r;
            }
            colorMap.insert(make_pair(vStr[0],color));
        }
    }

    depth = isGrayScale ? 1 : 4;
    unsigned int size = width * height * depth;
    unsigned char *data = new unsigned char [size]; // allocate memory for image data
    if(!data) {
        fprintf(stderr, "Image::createFromXPM() ERROR: could not allocate image memory.\n");
        return 0;
    }

    // read image:
    for(i=0; i<height; ++i) {
        //cout << "[" << xpm[i+1+nColors] << "]" << endl;
        string currLine(xpm[i+1+nColors]);
        for(unsigned int j=0; (j<width)&&(j<currLine.size()/charsPerPixel); ++j) {
            string currStr=currLine.substr(j*charsPerPixel,charsPerPixel);
            memcpy(&data[((height-i-1)*width+j)*depth],&(colorMap[currStr]),depth);
        }
    }
	return new Image(data,width,height,depth);
}

//--- class TextureMgr ---------------------------------------------

TextureMgr* TextureMgr::sp_instance = 0;

TextureMgr::TextureMgr() {
	loaderRegister(Image::loadTGA,"tga");
}

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

unsigned int TextureMgr::genTexture(const Image & img, bool repeatX, bool repeatY) {
#ifdef _HAVE_GL
    int colorType;
    switch (img.depth()) {
    case 1: colorType = GL_ALPHA; break;
    case 3: colorType = GL_RGB;   break;
    case 4: colorType = GL_RGBA;  break;
    default:
        fprintf(stderr, "ERROR TextureMgr::genTexture: unsupported color format.");
        return 0;
    }
    unsigned int id=0;
	glGenTextures(1,&id);
    glBindTexture (GL_TEXTURE_2D, id);
    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeatX ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeatY ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    gluBuild2DMipmaps ( GL_TEXTURE_2D, colorType, img.width(), img.height(),
                       colorType, GL_UNSIGNED_BYTE, img.data());
    return id;
#else 
	return 0;
#endif
}

unsigned int TextureMgr::getTextureId(const std::string & filename, bool repeatX, bool repeatY, bool reload) {
	if(!reload) {
		map<string,TexData*>::iterator jt=mm_texDataName.find(filename);
		if(jt!=mm_texDataName.end()) return jt->second->texId;
	}
	Image * pImg = load(filename);
	if(!pImg) return 0;
	unsigned int texId=genTexture(*pImg, repeatX, repeatY);
	if(texId) mm_texDataName.insert(make_pair(filename, new TexData(filename, texId,pImg->width(),pImg->height(),pImg->depth())));
	delete pImg;
	return texId;
}

Image* TextureMgr::load(const std::string & filename) {
	if(filename.rfind('.')>filename.size()) return 0; // suffix required for loader
	string suffix=toLower(filename.substr(filename.rfind('.')+1));
	map<string, Image* (*)(const string &)>::iterator it = mm_loader.find(suffix);
	if(it==mm_loader.end()) return 0;
	string fname(io::unifyPath(filename));
	bool found = io::fileExist(fname);
	if(!found) for(vector<string>::const_iterator it = mv_searchPath.begin(); it!=mv_searchPath.end(); ++it) {
		fname = *it+filename;
		if(!io::fileExist(fname)) continue;
		found = true;
	}
	return found ? (*(it->second))(fname) : 0;
}

Image* TextureMgr::grabScreen(unsigned int screenW, unsigned int screenH, bool frontBuffer) {
#ifdef _HAVE_GL
    unsigned int nBytes = 3 * screenW * screenH;
    unsigned char *data = new unsigned char[nBytes];
    if(frontBuffer) glReadBuffer(GL_FRONT);
    glReadPixels(0,0,screenW,screenH,GL_RGB,GL_UNSIGNED_BYTE, data);
    if(frontBuffer) glReadBuffer(GL_BACK);
    return new Image(data, screenW, screenH,3);
#else
    return 0;
#endif

}

bool TextureMgr::properties(const std::string & name, unsigned int & id, unsigned int & width, unsigned int & height,  unsigned int & depth) const {
	map<string,TexData*>::const_iterator jt=mm_texDataName.find(name);
	if(jt==mm_texDataName.end()) return false;
	id=jt->second->texId;
	width=jt->second->width;
	height=jt->second->height;
	depth=jt->second->depth;
	return true;
}

void TextureMgr::loaderRegister(Image *(*loadFunc)(const std::string &), const std::string & suffix) {
	mm_loader.insert(make_pair(toLower(suffix),loadFunc)); 
}

bool TextureMgr::loaderAvailable(const std::string & suffix) const {
	return mm_loader.find(toLower(suffix))!=mm_loader.end(); 
}

void TextureMgr::searchPathAppend(const std::string & path) {
	string p=io::unifyPath(path);
	for(vector<string>::iterator it = mv_searchPath.begin(); it!=mv_searchPath.end(); ++it)
		if(p==*it) return; // nothing to do
	mv_searchPath.push_back(p);
}

//--- class Font ------------------------------------------------
void Font::print(float x, float y, float w, float h, const char* text, float alignHor, float lineHeight) const {
	const char WHITESPACE[] = ("\n\t\r ");
	float currY=y-m_fontHeight;
	float currW;
	lineHeight*=m_fontHeight;
	size_t text_size = strlen(text);
	size_t pos = strspn(text, WHITESPACE);
	if(pos>=text_size) return;
	do {
		currW=0.0f;
		size_t eow = pos+min(text_size-pos, strcspn(text+pos, WHITESPACE));
		float chunkWidth=width(text+pos, eow-pos);

		if(chunkWidth>w) {
			while(chunkWidth>w) chunkWidth=width(text+pos,(--eow)-pos);
			if(eow<=pos) return;
			currW = chunkWidth;
		}
		else {
			currW = chunkWidth;
			while(text[eow]&&(text[eow]!='\n')&&(text[eow]!='\r')) {
				currW = chunkWidth;
				size_t eon = eow+1+min(text_size-eow-1, strcspn(text+eow+1, WHITESPACE));
				chunkWidth = width(text+pos,eon-pos);
				if(chunkWidth>w) break;
				eow = eon;
				currW = chunkWidth;
				if(!text[eon]||(text[eon]=='\n')||(text[eon]=='\r')) break;
			}
		} 

		print(x+((alignHor-1.0f)*0.5f*currW), currY, text+pos, eow-pos);
		
		pos = eow+1;
		currY-=lineHeight;

		if((h>0.0f) && (currY< y-h)) break;
	}
	while(pos<text_size);
}

float Font::height(const char* text, float w, float lineHeight) const {
	unsigned int nLines = 0;
	const char WHITESPACE[] = ("\n\t\r ");
	float currW;
	size_t text_size = strlen(text);
	size_t pos = strspn(text, WHITESPACE);
	if(pos>=text_size) return 0.0f;
	do {
		currW=0.0f;
		size_t eow = pos+min(text_size-pos, strcspn(text+pos, WHITESPACE));
		float chunkWidth=width(text+pos, eow-pos);

		if(chunkWidth>w) {
			while(chunkWidth>w) chunkWidth=width(text+pos,(--eow)-pos);
			if(eow<=pos) return nLines*m_fontHeight*lineHeight;
			currW = chunkWidth;
		}
		else {
			currW = chunkWidth;
			while(text[eow]&&(text[eow]!='\n')&&(text[eow]!='\r')) {
				currW = chunkWidth;
				size_t eon = eow+1+min(text_size-eow-1, strcspn(text+eow+1, WHITESPACE));
				chunkWidth = width(text+pos,eon-pos);
				if(chunkWidth>w) break;
				eow = eon;
				currW = chunkWidth;
				if(!text[eon]||(text[eon]=='\n')||(text[eon]=='\r')) break;
			}
		} 
		
		pos = eow+1;
		++nLines;
	}
	while(pos<text_size);
		
	return nLines* m_fontHeight *lineHeight;
}

void Font::glyphPrint(unsigned char c, int width, int height, const unsigned char* const data, int stride) {
	if(!stride) stride=width;
	int metrics[4];
	glyphMetrics(width,height,data,stride,metrics);
	printf("--- %i/%c origin:(%i|%i) dim:(%i|%i)\n",
		(int)c,c, metrics[0], metrics[1], metrics[2], metrics[3]);
	for(int i=height-1; i>=0; --i) {
		for(int j=0; j<width; ++j)
		 if(data[i*stride+j]>128) printf("*");
		else if(data[i*stride+j]>64) printf("+");
		else if(data[i*stride+j]) printf(".");
		else printf(" ");
		printf("|\n");
	}
	fflush(stdout);
}

void Font::glyphMetrics(int widthMax, int heightMax, const unsigned char* const data, int stride, int metrics[4]) {
	int xMin=-1, yMin=-1, xMax=-1, yMax=-1;
	for(int i=0; i<heightMax; ++i)
		for(int j=0; j<widthMax; ++j) if(data[i*stride+j]) {
			 if((xMin<0)||(j<xMin)) xMin=j;
			 if((xMax<0)||(j>xMax)) xMax=j;
			 if((yMin<0)||(i<yMin)) yMin=i;
			 if((yMax<0)||(i>yMax)) yMax=i;
		}
	metrics[0]=xMin;
	metrics[1]=yMin;
	metrics[2]=xMax;
	metrics[3]=yMax;
}



//--- class FontMap --------------------------------------

FontMap::FontMap(const Image & img, unsigned int fontTextureId) : Font(0), m_texId(fontTextureId) {
	if(!m_texId) m_texId = TextureMgr::singleton().genTexture(img, false, false); 
	if(!m_texId) return;
		
	unsigned int wGlyphMax = img.width()/16;
	unsigned int hGlyphMax = img.height()/16;
	unsigned int  stride = img.width()*img.depth();
	
	int yMin=-1, yMax=-1;
	//int baseLine=-1;
	unsigned int aMinX[NUM_CHARS];
	for(unsigned int i=0; i<NUM_CHARS; ++i) {
		unsigned int originX=(i%16)*wGlyphMax;
		unsigned int originY=(15-i/16)*hGlyphMax;
		const unsigned char* glyphData = img.data() + originY*stride + originX*img.depth();
		int metrics[4];
		glyphMetrics(wGlyphMax,hGlyphMax,glyphData, stride, metrics );
		ma_advanceX[i] = metrics[2]-metrics[0];
		aMinX[i] = metrics[0];
		if((metrics[1]>=0)&&((yMin==-1)||(metrics[1]<yMin))) yMin=metrics[1];
		if((metrics[3]>=0)&&((yMax==-1)||(metrics[3]>yMax))) yMax=metrics[3];
		//if(i==(unsigned int)'H') baseLine=metrics[1];
	}
	unsigned int wCaret = ma_advanceX[(unsigned char)'|'];
	unsigned int wSpace = ma_advanceX[(unsigned char)'M'];
	
#ifdef _HAVE_GL
	float wPixel = 1.0f/float(img.width());
	m_fontHeight = static_cast<float>(yMax-yMin);
	m_listId = glGenLists(NUM_CHARS);
	for (unsigned int i=0; i<NUM_CHARS; ++i) {
		float originX=(float)(i%16)*wGlyphMax + aMinX[i];
		float originY=(float)(15-i/16)*hGlyphMax + yMin;

		glNewList(m_listId+i, GL_COMPILE);
		if(ma_advanceX[i]) {
			glBegin(GL_QUADS);
			glTexCoord2f(originX*wPixel,originY*wPixel);
			glVertex2f(0.0f, 0.0f);
			glTexCoord2f( (originX+ma_advanceX[i]+1)*wPixel, originY*wPixel );
			glVertex2f(ma_advanceX[i]+1.0f, 0.0f);
			glTexCoord2f( (originX+ma_advanceX[i]+1)*wPixel, (originY+m_fontHeight+1)*wPixel );
			glVertex2f(ma_advanceX[i]+1.0f, m_fontHeight+1);
			glTexCoord2f( originX*wPixel, (originY+m_fontHeight+1)*wPixel );
			glVertex2f(0.0f, m_fontHeight+1);
			glEnd();
		}
		if(ma_advanceX[i]) ma_advanceX[i]+=wCaret+1; else ma_advanceX[i] = wSpace; // add letter spacing to advanceX
		glTranslatef((float)(ma_advanceX[i]), 0.0f, 0.0f);
		glEndList();
	}
#endif
}

FontMap::~FontMap() {
#ifdef _HAVE_GL
        glDeleteLists(m_listId, NUM_CHARS);
#endif
}

Font* FontMap::create(const std::string & fname, float ) {
	Image* pImg = TextureMgr::singleton().load(fname);
	// check if texture is already loaded:
	unsigned int id, width, height, depth;
	if(!TextureMgr::singleton().properties(fname, id, width, height, depth)) id=0;
	return pImg ? new FontMap(*pImg, id) : 0;
}

void FontMap::print(float x, float y, const char *text, unsigned int nChar) const {
    if(!m_listId || !m_texId || !text ) return;	
 #ifdef _HAVE_GL
	glPushMatrix();
	glTranslatef(x,y,0.0f);
	glBindTexture(GL_TEXTURE_2D, m_texId);
	glListBase(m_listId);
	nChar = (nChar==0) ? strlen(text) : min(nChar, strlen(text));
	glCallLists(nChar, GL_UNSIGNED_BYTE, text);
	glPopMatrix();
#endif
}

float FontMap::width(const char * text, unsigned int nChar) const {
	float w = 0.0f;
	if(nChar) for(const char * c=text; (*c!=0)&&(c!=text+nChar); ++c) 
		w+=ma_advanceX[static_cast<unsigned char>(*c)];
	else for(const char * c=text; *c!=0; ++c) 
		w+=ma_advanceX[static_cast<unsigned char>(*c)];
	return w;
}
