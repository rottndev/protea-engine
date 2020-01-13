# settings for C++ compiler:
CC     = g++
#CC     = c:/bin/mingw/bin/g++
CFLAGS = -O -Wall -D_HAVE_GL -D_DEBUG #-g
INCDIR = -I. -I/usr/include -Iexternal/include -Imodules

# settings to make the library:
LIBN   = protea
LCC    = ar
LFLAGS = -rcs

# linker settings:
LIB    = -ljpeg -lpng -lz
LIBDIR = -L.
LINLIB = -Lexternal/Linux -lGL -lGLU -lX11 -lm -lpthread -lrt
WINLIB = -Lexternal/win32 -lopengl32 -lglu32 -lwinmm -luser32 -lgdi32 -lkernel32 -mwindows -s

# generic platform specific rules:
ARCH            = $(shell uname -s)
ifeq ($(ARCH),Linux)
  LIBS          = -L/usr/lib -Lexternal/Linux $(LIBDIR) $(LIB) $(LINLIB)
  EXESUFFIX     =
else  # windows, MinGW
  LIBS          = $(LIBDIR) -lmingw32 $(LIB) $(WINLIB)
  EXESUFFIX     = .exe
endif

# source files:
SRC = proMath.cpp proStr.cpp proResource.cpp proXml.cpp proScene.cpp proIo.cpp proMaterial.cpp proMesh.cpp proRenderer.cpp proDevice.cpp proDeviceLocal.cpp proCallable.cpp
HDR = $(SRC:.cpp=.h) protea.h
OBJ = $(SRC:.cpp=.o)

# make targets and rules:
all: $(LIBN) DeviceInputTest$(EXESUFFIX) proteaViewer$(EXESUFFIX)

$(LIBN): $(OBJ)
	$(LCC) $(LFLAGS) lib$(LIBN).a $(OBJ)

DeviceInputTest$(EXESUFFIX) : DeviceInputTest.o proGlfw.o lib$(LIBN).a
	$(CC) $(CFLAGS) DeviceInputTest.o proGlfw.o $(LIBDIR) -l$(LIBN) -lglfw $(LIBS) -o $@

proteaViewer$(EXESUFFIX) : proteaViewer.o modules/proCanvas.o modules/proGui.o proGlfw.o proIoWrl.o proIoObj.o proIo3ds.o proIoPng.o proIoJpg.o skydome.o lib$(LIBN).a
	$(CC) $(CFLAGS) proteaViewer.o modules/proCanvas.o modules/proGui.o proGlfw.o proIoWrl.o proIoObj.o proIo3ds.o proIoPng.o proIoJpg.o skydome.o $(LIBDIR) -l$(LIBN) -lglfw -llua $(LIBS) -o $@

DeviceInputTest.o: DeviceInputTest.cpp $(HDR) proGlfw.h
proteaViewer.o: proteaViewer.cpp $(HDR) proGlfw.h skydome.h
modules/proCanvas.o: modules/proCanvas.cpp modules/proCanvas.h proDevice.h proResource.h
modules/proGui.o: modules/proGui.cpp modules/proGui.h modules/proCanvas.h
proGlfw.o: proGlfw.cpp proGlfw.h proDevice.h proResource.h defaultFont.xpm
proIoWrl.o: proIoWrl.cpp proIoWrl.h
proIoObj.o: proIoObj.cpp proIoObj.h
proIo3ds.o: proIo3ds.cpp proIo3ds.h
proIoPng.o: proIoPng.cpp proIoPng.h
proIoJpg.o: proIoJpg.cpp proIoJpg.h
skydome.o: skydome.cpp skydome.h

proCallable.o: proCallable.cpp proCallable.h
proMath.o: proMath.cpp proMath.h
proStr.o: proStr.cpp proStr.h
proResource.o: proResource.cpp proResource.h
proXml.o: proXml.cpp proXml.h
proScene.o: proScene.cpp proScene.h proXml.h proMaterial.h
proIo.o: proIo.cpp proIo.h proStr.h
proMaterial.o: proMaterial.cpp proMaterial.h proMath.h proXml.h proStr.h proResource.h
proMesh.o: proMesh.cpp proMesh.h proScene.h
proDevice.o: proDevice.cpp proDevice.h
proDeviceLocal.o: proDeviceLocal.cpp proDeviceLocal.h proDevice.h proIo.h proStr.h
proRenderer.o: proRenderer.cpp proRenderer.h proScene.h proMaterial.h

# generic rules and targets:
.cpp.o:
	$(CC) $(CFLAGS) $(INCDIR) -c $< -o $@

clean:
	rm -f *.o modules/*.o $(EXE) *~

