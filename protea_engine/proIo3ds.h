#ifndef _IO_3DS_H
#define _IO_3DS_H

/** @file proIo3ds.h
 \brief Contains a 3DS input/output class

 \author  gf
 $Revision: 1.0 $
 License notice (zlib license):

 (c) 2008 by Gerald Franz, www.viremo.de

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

class proNode;

//--- class io3ds ---------------------------------------------
/// a class for 3ds (old 3ds Max format) input/output
class io3ds {
public:
    /// loads file into internal proNode objects
    /** \param filename the file to be loaded,
         \return pointer to a glTransform object in case of success or 0. */
    static proNode * load(const std::string & filename);
};

#endif //  _IO_3DS_H
