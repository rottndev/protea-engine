<p align="center">Uploaded and archived due to source is no longer available. (www.viremo.de)<b /><br />

<img src="protea_128.png /></p>


## Introduction
Protea is a 3D graphics engine layered on top of OpenGL which lends itself very well
for fast prototyping purposes. Due to its simplicity, it can be
easily learned and used to rapidly test programming ideas that require the ability to load
3D scenes from files and display and manipulate them in realtime. Furthermore, its minimalistic and non-intrusive design
makes protea a perfect basis to test rendering algorithms and design ideas, or to build more specialized or complete solutions
on top of it. Despite its tininess, protea is mature and complete enough to be productively used in mid-sized
projects, its liberal licensing conditions (zlib style) allow the library to be used both in commercial and non-commercial, open source and closed
source projects.

## Main features
- clean minimal C++ code base, portable, extendable class design
- makes optimized use of fast hardware-accelerated industry standard OpenGL (1.2+) 3D graphics API
- regularly tested on MS Windows and Linux (MinGW/gcc compiler)
- capable of loading and writing 3D mesh models based on a subset of ISO standard xml-encoded X3D, supporting all basic nodes
- extensible framework for the support of further 3D scene formats, various mesh loaders and writers available (OBJ, VRML, 3DS, OGRE3D)
- basic hierarchical scene graph and frustum culling
- loading all flavors (8, 16, 24, 32 bit, paletted, RLE, uncompressed) of Targa images as textures
- optionally loadng and saving of PNG and JPEG images via libpng (http://www.libpng.org/pub/png/) and libjpeg (http://www.ijg.org/) [ new ]
- text rendering support using simple texture fonts
- optional experimental cast shadows using shadow volumes both for point lights and distant lights
- both flat face shading and smooth Gouraud shading possible within the same object, depending on definable crease angle
- complete set of generic linear algebra / vector mathematics classes
- ray casting and scene query functions
- optional Lua based scripting framework for the flexible integration of modules at runtime [ new ]
- generic input device access abstraction layer, allowing for flexible remapping at runtime [ new ]
- optional connection to external input devices via adapter applications without direct linking (see class DeviceLocal) [ new ]
- deployment of executables amde easy, because protea and all its (optional) dependencies legally and technically allow for static linking

## Download
- <a href="http://www.viremo.de/protea/protea_090912.zip">protea source code release 2009-09-12</a>

## Status
Protea is a slowly yet constantly evolving spare-time project. Due to that, it is unlikely that there
will ever be something like a true final release. However, from time to time self-contained and fairly stable
states will be published and archived at the protea website (http://www.viremo.de/protea).

## Requirements
Protea has been designed to introduce only a minimum of external dependencies:

- A C++ compiler including an implementation of the Standard Template
  Library.  Protea is mainly developed and maintained using
   the GNU C++ compiler 3.2+ under the development platforms Linux and
  Windows. The provided Makefiles work both for Linux and MinGW under Windows. 
  Exemplary project files for MS VisualStudio 2008 are included as well.

- Currently an OpenGL (1.2+) implementation is mandatory. Since Protea deliberately maintains 
  the possibility to mix it with low level graphic access, it is rather improbable
  that further graphic library interfaces such as DirectX will ever be added.
  
- For flexible integration of C++ objects at runtime, the proCallable base class is available.
  It requires the Lua virtual machine available at http://www.lua.org . Apart from the demo
  applications, protea does not make use of the Lua VM internally at the moment, hence this
  dependency can be seen as optional.

- In order to open a window and to get user input, an additional input library is required.
  While protea itself can work with any framework, the provided example is based on
  GLFW (http://glfw.sf.net) which shares the same liberal license conditions (see below). Alternatively,
  libSDL (http://www.libsdl.org) may be used to get platform independent access to high
  performance multimedia and input programming interfaces. libSDL has a slightly more restrictive
  license (LGPL), but offers additional features such as audio functions.
  
- While not a requirement, conv3d (http://www.viremo.de/conv3d) is a perfect complementary application that allows
  3D geometry models in a wide variety of file format to be converted into protea-compatible X3D files. In addition,
  the convert tool from the ImageMagick package lends itself very well to convert from other image formats to the
  TGA format natively required by protea.

- In order to load texture from PNG and JPEG image files, libpng (http://www.libpng.org/pub/png/), zlib 
  (http://www.zlib.net/), and libjpeg (http://www.ijg.org/) are required.

## License notice (zlib license):

 (c) 2006-2009 by Gerald Franz, www.viremo.de

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the author be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  - The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  - Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  - This notice may not be removed or altered from any source distribution.
 