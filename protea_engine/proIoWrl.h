#ifndef _IO_WRL_H
#define _IO_WRL_H

/** @file proIoWrl.h
 \brief contains a class for VRML97 input
*/
#include <string>

class proNode;

///  a class for VRML97 input
class ioWrl {
public:
    /// loads obj files into geoNodes
    /**
         \param filename the file to be loaded,
         \return pointer to a node in case of success or 0. */
    static proNode * load(const std::string & filename);
};

#endif //_IO_WRL_H
