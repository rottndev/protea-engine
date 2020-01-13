#ifndef _PRO_IO_JPG_H
#define _PRO_IO_JPG_H

/** @file proIoJpg.h
 \brief Contains an image loder class for JPEG files

 \author  gf
 $Revision: 1.0 $
 License notice (zlib license):

 (c) 2008 by Gerald Franz, gf@viremo.de

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

#include <string>

class Image;

//--- class ioJpg ---------------------------------------------
/// an image loader class for JPEG files
class ioJpg {
public:
    /// loads a JPEG file into an Image object
    /** \param filename the file to be loaded,
         \return pointer to an Image object in case of success or 0. */
    static Image * load(const std::string & filename);
	/// saves an Image to a JPG file using the default settings
    /**  \param img reference of Image to be saved
	 \param filename path and name of file to be saved to
         \return true case of success, otherwise false. */
	static bool save(const Image& img, const std::string & filename);
};

#endif //  _PRO_IO_JPG_H
