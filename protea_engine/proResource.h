#ifndef _PRO_RESOURCE_H
#define _PRO_RESOURCE_H

#include <string>
#include <map>
#include <vector>
/** @file proResource.h
 
 \brief Contains resource access classes

 \author  gf
started 2000. \n
 last update 2009-02-08

 License notice (zlib license):

 (c) 2006-2009 by Gerald Franz, www.viremo.de

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the author be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

//--- class Color --------------------------------------------------
/// auxiliary class for color definitions
class Color {
public:
	/// default constructor
	Color(): r(1.0f), g(1.0f), b(1.0f), a(1.0f) { }
	/// constructor
	Color(float rIn, float gIn, float bIn, float aIn=1.0f) : r(rIn), g(gIn), b(bIn), a(aIn) { }
	/// returns color as float[4]
	const float* value() const { return &r; }
	/// allows manipulating color as float[4]
	float* value() { return &r; }
	/// sets color value
	void value(float rIn, float gIn, float bIn, float aIn=1.0f) { r=rIn; g=gIn; b=bIn; a=aIn; }
	/// equality comparison operator
	bool operator==(const Color & c) const { 
		return (r!=c.r) ? false : (g!=c.g) ? false : (b!=c.b) ? false : (a!=c.a) ? false : true; }
	/// inequality comparison operator
	bool operator!=(const Color & c) const { return !operator==(c); }

	/// red color component
	float r;
	/// green color component
	float g;
	/// blue color component
	float b;
	/// alpha color component, opacity
	float a;
		
	/// static Color instance to indicate an invalid color
	static Color null;
	/// static Color instance to indicate a completely transparent color
	static Color transparent;
};

//--- class Image --------------------------------------------------
/// a class for handling pixel image data
class Image {
public:
	/// constructor passing image data and properties
	/** passed data is deleted up by destructor later on */
	Image(unsigned char *data, unsigned int width, unsigned int height, unsigned int depth) : 
		mp_data(data), m_width(width), m_height(height), m_depth(depth) { }
	/// destructor
	~Image() { delete[] mp_data; }

	/// allows access to raw image data
	unsigned char *data() { return mp_data; };
	/// allows read access to raw image data
	const unsigned char *data() const { return mp_data; };
	/// returns image width
	unsigned int width() const { return m_width; }
	/// returns image height
	unsigned int height() const { return m_height; }
	/// returns byte depth, bytes per pixel
	unsigned int depth() const { return m_depth; }	
	
	/// flips image vertically
	void flipV();
	/// returns an upsampled versio of this image
	unsigned char * upsample(int w2, int h2) const;

	/// creates image from XPM memory data
	static Image* createFromXPM(char **xpm);
	/// loads an image defined in a Truevision tga file
	static Image* loadTGA(const std::string & name);
	/// saves image to an uncompressed TGA file
	bool saveTGA(const std::string & filename) const;
protected:
	/// pointer to image data
	unsigned char *mp_data;
	/// image width
	unsigned int m_width;
	/// image height
	unsigned int m_height;
	/// bytes per pixel
	unsigned char m_depth;
};


//--- class TextureMgr ---------------------------------------------

/// a singleton class managing texture resources as well as loaders and savers
class TextureMgr {
public:
	/// returns singleton instance
	static TextureMgr & singleton() {
		if(!sp_instance) sp_instance=new TextureMgr; return *sp_instance; }
	/// returns pointer to singleton instance
	static TextureMgr * singletonPtr() { return &singleton(); }

	/// looks up whether a texture is already loaded or loads up an image file to an OpenGL texture in one step. 
	/** \return texture id or 0 in case of error */
	unsigned int getTextureId(const std::string & name, bool repeatX=true, bool repeatY=true, bool reload=false);
	/// uploads an Image object as texture to OpenGL */
	/**  \param img Image object to be uploaded
	 \param repeatX (optional) stores whether texture will be repeated or clamped in X direction
	 \param repeatY (optional) stores whether texture will be repeated or clamped in Y direction
	\return texture id or 0 in case of error */
	unsigned int genTexture(const Image & img, bool repeatX=true, bool repeatY=true);
	/// returns properties of a previously loaded texture by name
	bool properties(const std::string & name, unsigned int & id, unsigned int & width, unsigned int & height,  unsigned int & depth) const;

	/// registers a loader function handling a certain suffix
	/** The function has to be of type Image * loadXYZ(const std::string & filename).*/
	void loaderRegister(Image *(*loadFunc)(const std::string &), const std::string & suffix);
	/// returns true in case a loader for this kind of model file is available
	bool loaderAvailable(const std::string & suffix) const;
	/// loads an image
	Image* load(const std::string & name);
	/// makes a screenshot
	Image* grabScreen(unsigned int screenW, unsigned int screenH, bool frontBuffer=true);
	/// appends a path to the texture search path
	void searchPathAppend(const std::string & path);
protected:    
	/// default constructor registering built-in loaders and savers
	TextureMgr();

	/// structure holding basic texture attributes
	struct TexData {
		/// constructor
		TexData(const std::string & name, unsigned int id, unsigned int w, unsigned int h, unsigned int d) : texName(name), texId(id), width(w), height(h), depth(d) { }
		/// texture name
		std::string texName;
		/// texture ID
		unsigned int texId;
		/// texture width in pixels
		unsigned int width;
		/// texture height in pixels
		unsigned int height;
		/// texture depth in bytes
		unsigned int depth;
	};
	/// associative array between texture names and texture data
	std::map<std::string, TexData*> mm_texDataName;
	/// map associating suffixes to loader functions
	std::map<std::string, Image* (*)(const std::string &)> mm_loader;
	/// vector of texture search paths
	std::vector<std::string> mv_searchPath;

	/// pointer to singleton instance
	static TextureMgr* sp_instance;
};

//--- class Font ---------------------------------------------------
/// abstract base class for fonts
class Font{
public:
	/// prints single-line text at position x|y,
	/** \param x X coordinate of the text 
	 \param y Y coordinate of the text
	 \param text pointer to the first character of the text to be measured
	 \param nChar (optional) restricts the number of characters to be measured, 0 means no restriction */
	virtual void print(float x, float y, const char *text, unsigned int nChar=0) const=0;
	/// prints multi-line text within the box defined by x|y w|h
	void print(float x, float y, float w, float h, const char* text, float alignHor=1.0f, float lineHeight=1.33f) const;
	/// returns width of text
	/** \param text pointer to the first character of the text to be measured
	 \param nChar (optional) restricts the number of characters to be measured, 0 means no restriction
	 \return text width in pixels */
	virtual float width(const char * text, unsigned int nChar=0) const=0;
	/// returns width of string text
	/** \param text string to be measured
	 \return text width in pixels */
	float width(const std::string & text) const { return width(text.c_str(), text.size()); }
	/// returns height of font
	float height(const char * text=0) const { return m_fontHeight; }
	/// returns height of font
	float height(const std::string & ) const { return m_fontHeight; }
	/// returns height of multi-line text
	float height(const char* text, float w, float lineHeight=1.33f) const;
	/// destructor
	virtual ~Font() { }
	
	/// auxiliary method that allows the output of glyphs to the console
	static void glyphPrint(unsigned char c, int width, int height, const unsigned char* const data, int stride=0);
	/// auxiliary method that determines the metrics of a glyph from a 8bit grayscale bitmap
	static void glyphMetrics(int width, int height, const unsigned char* const data, int stride, int metrics[4]);
	/// maximum number of characters contained in the font
	static const unsigned int NUM_CHARS=256;
protected:
	/// constructor
	Font(float fontHeight) : m_fontHeight(fontHeight) { }
	/// stores font height
	float m_fontHeight;
};

//--- class FontMap ------------------------------------------------
// 2008-12-09 by Gerald Franz, www.viremo.de
///  minimal OpenGL text class that draws text as texture-mapped quads based on a single texture
/** Usage:\n
proFont* pFont=proFontMap::create(fontName);\n
 glColor4f(R, G, B, A);\n
 pFont->print(x,y, text, ...);\n
 \n
 Note: Requires a square texture containing 16x16 font glyphs.\n

 Acknowlegments:\n
   This class has originally been inspired by open-source code by various authors:
   Martin G. Bell, Marco Monster, Giuseppe D'Agata, Jeff Molofee.
   The current version is a complete rewrite (December '08) and provides superior performance 
   and quality as compared to its before state.
*/

class FontMap : public Font {
public:
	/// creates a font resource from an 8bit grayscale texture file
	static Font* create(const std::string & fname, float fontHeight=0.0f);
	/// creates a font resource from an 8bit grayscale Image object
	static Font* create(const Image & img, float fontHeight=0.0f) {
		return new FontMap(img); }
	/// prints single-line text at position x|y,
	virtual void print(float x, float y, const char *text, unsigned int nChar=0) const;
	/// returns width of text
	virtual float width(const char * text, unsigned int nChar=0) const;
protected:
	/// constructor
	FontMap(const Image & img, unsigned int fontTextureId=0);
	/// destructor
	virtual ~FontMap();
	/// start list id
	unsigned int m_listId;
	/// GL font texture id
	unsigned int m_texId;
	/// horizontal advances of the font glyphs
	unsigned int ma_advanceX[256];
};
#endif // _PRO_RESOURCE_H
