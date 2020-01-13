# include "proStr.h"
# include "proScene.h"
# include "proIo3ds.h"
#include  "proMaterial.h"

using namespace std;

//--- internal data structures -------------------------------------

/// \internal This class holds the information for a material.
class Material3ds {
public:
    /// default constructor
    Material3ds();
    /// the texture name
    char  matName[255];
    /// the texture file name (If this is set it's a texture map)
    char  fileName[255];
    /// the color of the object (R, G, B)
    unsigned char color[3];
    /// the opacity of the object (A)
    float opacity;
} ;

Material3ds::Material3ds() : opacity(1.0f){
    matName[0]=0;
    fileName[0]=0;
    color[0]=color[1]=color[2]=255;
}

/// \internal this class stores a material reference
class MaterialRef3ds {
public:
    /// default constructor
    MaterialRef3ds() { matName[0]=0; };
    /// The material name of the object
    char matName[255];
    /// stores face indices
    std::vector<unsigned short> vFace;
};

/// \internal This class holds all the information of a part of our model
class Mesh3ds {
public:
    /// default constructor
    Mesh3ds() { objName[0]=0; };
    /// The name of the object
    char objName[255];

    /// The object's vertices
    std::vector<vec3f> vCoords;
    /// The texture's UV coordinates
    std::vector<vec2f> vTexCoords;

    /// stores indices
    std::vector<unsigned int> vIndices;
    /// stores assigned materials
    std::vector<MaterialRef3ds> material;
};

/// this struct holds a 3ds chunk info
struct Chunk3ds {
    /// the chunk's ID
    unsigned short int ID;
    /// the length of a chunk
    unsigned int length;
    /// the amount of bytes read within that chunk
    unsigned int bytesRead;
};


//--- 3dsLoaderClass declaration -----------------------------------

/// this class handles the loading of 3ds files
class Loader3ds {
public:
    /// constructor initializing the data members
    Loader3ds();
    /// destructor initializing the data members
    ~Loader3ds() { close(); }
    /// loads the 3ds geometry into a vector of submeshes
    bool load(const std::string & strFileName);
	/// returns number of submeshes
	size_t size() const { return mv_mesh.size(); }
	/// allows reading the individual submeshes
	const Mesh3ds & operator[](size_t n) const { return mv_mesh[n]; }
	/// allows reading the material information
	const Material3ds & material(size_t n) const { return mv_material[n]; }
	/// returns number of materials
	size_t numMaterials() const { return mv_material.size(); }
protected:
    /// stores children
    std::vector<Mesh3ds> mv_mesh;
    /// stores materials
    std::vector<Material3ds> mv_material;

    /// This reads in a string and saves it in the char array passed in
    int getString(char *);
    /// This reads the next chunk
    void readChunk(Chunk3ds *);
    /// This reads the next large chunk
    void processNextChunk(Chunk3ds *);
    /// This reads the object chunks
    void processNextObjectChunk(Mesh3ds *pObject, Chunk3ds *);
    /// This reads the material chunks
    void processNextMaterialChunk(Chunk3ds *);
    /// This reads the RGB value for the object's color
    void readColorChunk(Material3ds *pMaterial, Chunk3ds *pChunk);
    ///  this method reads transparency data
    void readTranspChunk(Material3ds *pMaterial, Chunk3ds *pChunk);
    /// This reads the objects vertices
    void readVertices(Mesh3ds *pObject, Chunk3ds *);
    /// This reads the objects face information
    void readVertexIndices(Mesh3ds *pObject, Chunk3ds *);
    /// This reads the texture coodinates of the object
    void readUVCoordinates(Mesh3ds *pObject, Chunk3ds *);
    /// This reads in the material name assigned to the object and sets the materialID
    void readObjectMaterial(Mesh3ds *pObject, Chunk3ds *pPreviousChunk);
    /// This frees memory and closes the file
    void close();

    /// The file pointer
    FILE *m_FilePointer;
    /// this pointer is used through the loading process to hold the chunk information
    Chunk3ds *m_CurrentChunk;
    /// this pointer is used through the loading process to hold the chunk information
    Chunk3ds *m_TempChunk;
    /// this buffer is used to skip unwanted data when reading
    int buffer[50000];
};


//--- externally visible 3ds loader function -----------------------
proNode * io3ds::load(const std::string & filename) {
    Loader3ds loader;
    if(!loader.load(filename)||!loader.size()) return 0;

	// transfer material information:
	for(size_t j = 0; j < loader.numMaterials(); ++j) {
		proMaterial mat(loader.material(j).matName);
		if(loader.material(j).fileName[0])
			mat.texName(loader.material(j).fileName);
		mat.color(float(loader.material(j).color[0])/255.0f, float(loader.material(j).color[1])/255.0f, float(loader.material(j).color[2])/255.0f,loader.material(j).opacity);
		MaterialMgr::singleton().add(mat);
	}
	
    // transfer geometry:
    proTransform * parent=new proTransform(filename);
    for(size_t i=0; i<loader.size(); i++) {
        if(loader[i].material.size()<2) {
            proMesh * pMesh=new proMesh(loader[i].objName);
            pMesh->coords()=loader[i].vCoords;
            pMesh->texCoords()=loader[i].vTexCoords;
            pMesh->indices()=loader[i].vIndices;
			if(loader[i].material.size())
				pMesh->material(MaterialMgr::singleton()[loader[i].material[0].matName]);
            parent->append(pMesh,false);
        }
        else for(size_t k=0; k<loader[i].material.size(); k++) {
            proMesh * pMesh=new proMesh(loader[i].objName+i2s(k));
            pMesh->coords()=loader[i].vCoords;
            pMesh->texCoords()=loader[i].vTexCoords;
            size_t j;
            for(j = 0; j < loader[i].material[k].vFace.size(); j++) {
                pMesh->indices().push_back(loader[i].vIndices[loader[i].material[k].vFace[j]*3]);
                pMesh->indices().push_back(loader[i].vIndices[loader[i].material[k].vFace[j]*3+1]);
                pMesh->indices().push_back(loader[i].vIndices[loader[i].material[k].vFace[j]*3+2]);
            }
			pMesh->material(MaterialMgr::singleton()[loader[i].material[k].matName]);
            parent->append(pMesh,false);
        }
    }
	
    if(parent->size()) return parent;
    else {
        delete parent;
        return 0;
    }
}

//--- 3ds chunk ids and names --------------------------------------
//>------ Primary Chunk, at the beginning of each file
#define PRIMARY       0x4D4D

#define CHUNK_PERCENT_INT   0x0030
#define CHUNK_PERCENT_FLOAT 0x0031

//>------ Main Chunks
#define OBJECTINFO    0x3D3D                // This gives the version of the mesh and is found right before the material and object information
#define VERSION       0x0002                // This gives the version of the .3ds file
#define EDITKEYFRAME  0xB000                // This is the header for all of the key frame info

//>------ sub defines of OBJECTINFO
#define MATERIAL      0xAFFF                // This stored the texture info
#define OBJECT        0x4000                // This stores the faces, vertices, etc...

//>------ sub defines of MATERIAL
#define MATNAME       0xA000                // This holds the material name
#define MATDIFFUSE    0xA020                // This holds the color of the object/material
#define MATTRANSP     0xA050                // This holds the transparency of the material
#define MATMAP        0xA200                // This is a header for a new material
#define MATMAPFILE    0xA300                // This holds the file name of the texture

#define OBJECT_MESH   0x4100                // This lets us know that we are reading a new object

//>------ sub defines of OBJECT_MESH
#define OBJECT_VERTICES     0x4110          // The objects vertices
#define OBJECT_FACES        0x4120          // The objects faces
#define OBJECT_MATERIAL     0x4130          // This is found if the object has a material, either texture map or color
#define OBJECT_UV           0x4140          // The UV texture coordinates


//--- 3dsLoaderClass definition ------------------------------------
// This class includes all the code needed to load a .3DS file.
// It is heavily based on a tutorial by
//
// Ben Humphrey (DigiBen)
// DigiBen@GameTutorials.com


///   This constructor initializes the Chunk3ds data
Loader3ds::Loader3ds() {
    m_CurrentChunk = 0;
    m_TempChunk = 0;
}

///   this method is called to open the .3ds file, read it, then clean up
bool Loader3ds::load(const std::string & strFileName) {
#ifdef _MSC_VER
    if (fopen_s(&m_FilePointer, strFileName.c_str(), "rb")!=0) { // open the file
#else
    if ((m_FilePointer = fopen(strFileName.c_str(), "rb")) == NULL) { // open the file
#endif
        cerr << "ERROR: Unable to find the file: " << strFileName << endl;
        return false;
    }

    m_CurrentChunk = new Chunk3ds;
    m_TempChunk = new Chunk3ds;
    mv_mesh.clear();
    mv_material.clear();

    // read the first chuck of the file to see if it's a 3DS file
    readChunk(m_CurrentChunk);
    if (m_CurrentChunk->ID != PRIMARY) { // make sure this is a 3DS file
        cerr << "Unable to load PRIMARY chuck from file: %s!" << strFileName << endl;
        close();
        return false;
    }

    // begin loading objects, by calling this recursive function
    processNextChunk(m_CurrentChunk);
    close();
    return true;
}

///   This function cleans up our allocated memory and closes the file
void Loader3ds::close() {
    if(m_FilePointer) fclose(m_FilePointer);
	m_FilePointer = 0;
    delete m_CurrentChunk;
	m_CurrentChunk = 0;
    delete m_TempChunk;
	m_TempChunk = 0;
}


///   This function reads the main sections of the .3DS file, then dives deeper with recursion
void Loader3ds::processNextChunk(Chunk3ds *pPreviousChunk) {
    Mesh3ds newObject;                  // This is used to add to our object list
    Material3ds newTexture;             // This is used to add to our material list
    unsigned int version = 0;                   // This will hold the file version

    m_CurrentChunk = new Chunk3ds;                // Allocate a new chunk

    while (pPreviousChunk->bytesRead < pPreviousChunk->length) {
        // read next Chunk
        readChunk(m_CurrentChunk);

        // Check the chunk ID
        switch (m_CurrentChunk->ID) {
        case VERSION:                           // This holds the version of the file
            // read the file version and add the bytes read to our bytesRead variable
            m_CurrentChunk->bytesRead += static_cast<unsigned int>(fread(&version, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer));
            // If the file version is over 3, give a warning that there could be a problem
            if (version > 0x03)
                cerr << "This 3DS file is over version 3 so it may load incorrectly" << endl;
            break;

        case OBJECTINFO:                        // This holds the version of the mesh
            // This chunk holds the version of the mesh.  It is also the head of the MATERIAL
            // and OBJECT chunks.  From here on we start reading in the material and object info.
            // read the next chunk
            readChunk(m_TempChunk);
            // Get the version of the mesh
            m_TempChunk->bytesRead += static_cast<unsigned int>(fread(&version, 1, m_TempChunk->length - m_TempChunk->bytesRead, m_FilePointer));
            // Increase the bytesRead by the bytes read from the last chunk
            m_CurrentChunk->bytesRead += m_TempChunk->bytesRead;
            // Go to the next chunk, which is the object has a texture, it should be MATERIAL, then OBJECT.
            processNextChunk(m_CurrentChunk);
            break;

        case MATERIAL:                          // This holds the material information
            // This chunk is the header for the material info chunks
            // Add a empty texture structure to our texture list.
            mv_material.push_back(newTexture);
            // Proceed to the material loading function
            processNextMaterialChunk(m_CurrentChunk);
            break;

        case OBJECT:                            // This holds the name of the object being read
            // This chunk is the header for the object info chunks.  It also
            // holds the name of the object.
            // Add a new tObject node to our list of objects (like a link list)
            mv_mesh.push_back(newObject);
            // Get the name of the object and store it, then add the read bytes to our byte counter.
            m_CurrentChunk->bytesRead += getString(mv_mesh[mv_mesh.size() - 1].objName);
            // Now proceed to read in the rest of the object information
            processNextObjectChunk(&mv_mesh.back(), m_CurrentChunk);
            break;

        case EDITKEYFRAME:
        default:
            // If we didn't care about a chunk, then we get here.  We still need
            // to read past the unknown or ignored chunk and add the bytes read to the byte counter.
            m_CurrentChunk->bytesRead += static_cast<unsigned int>(fread(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer));
            break;
        }
        // Add the bytes read from the last chunk to the previous chunk passed in.
        pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
    }
    // Free the current chunk and set it back to the previous chunk (since it started that way)
    delete m_CurrentChunk;
    m_CurrentChunk = pPreviousChunk;
}


///   This function handles all the information about the objects in the file
void Loader3ds::processNextObjectChunk(Mesh3ds *pObject, Chunk3ds *pPreviousChunk) {
    // Allocate a new chunk to work with
    m_CurrentChunk = new Chunk3ds;

    // Continue to read these chunks until we read the end of this sub chunk
    while (pPreviousChunk->bytesRead < pPreviousChunk->length) {
        // Read the next chunk
        readChunk(m_CurrentChunk);

        // Check which chunk we just read
        switch (m_CurrentChunk->ID) {
        case OBJECT_MESH:
            // We found a new object, so let's read in it's info using recursion
            processNextObjectChunk(pObject, m_CurrentChunk);
            break;
        case OBJECT_VERTICES:
            readVertices(pObject, m_CurrentChunk);
            break;
        case OBJECT_FACES:
            readVertexIndices(pObject, m_CurrentChunk);
            break;
        case OBJECT_MATERIAL:
            // We now will read the name of the material assigned to this object
            readObjectMaterial(pObject, m_CurrentChunk);
            break;
        case OBJECT_UV:
            // This chunk holds all of the UV coordinates for our object.  Let's read them in.
            readUVCoordinates(pObject, m_CurrentChunk);
            break;
        default:
            // read past the ignored or unknown chunks
            m_CurrentChunk->bytesRead += static_cast<unsigned int>(fread(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer));
            break;
        }
        // Add the bytes read from the last chunk to the previous chunk passed in.
        pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
    }

    // Free the current chunk and set it back to the previous chunk (since it started that way)
    delete m_CurrentChunk;
    m_CurrentChunk = pPreviousChunk;
}


///   This function handles all the information about the material (Texture)
void Loader3ds::processNextMaterialChunk(Chunk3ds *pPreviousChunk) {
    // Allocate a new chunk to work with
    m_CurrentChunk = new Chunk3ds;

    // Continue to read these chunks until we read the end of this sub chunk
    while (pPreviousChunk->bytesRead < pPreviousChunk->length) {
        // read the next chunk
        readChunk(m_CurrentChunk);

        // Check which chunk we just read in
        switch (m_CurrentChunk->ID) {
        case MATNAME:                           // This chunk holds the name of the material
            // Here we read in the material name
            m_CurrentChunk->bytesRead += static_cast<unsigned int>(fread(mv_material[mv_material.size() - 1].matName, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer));
            break;
        case MATDIFFUSE:                        // This holds the R G B color of our object
            readColorChunk(&(mv_material[mv_material.size() - 1]), m_CurrentChunk);
            break;
        case MATMAP:                            // This is the header for the texture info
            // Proceed to read in the material information
            processNextMaterialChunk(m_CurrentChunk);
            break;
        case MATMAPFILE:                        // This stores the file name of the material
            // Here we read in the material's file name
            m_CurrentChunk->bytesRead += static_cast<unsigned int>(fread(mv_material[mv_material.size() - 1].fileName, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer));
            break;
        case MATTRANSP:
            readTranspChunk(&(mv_material[mv_material.size() - 1]), m_CurrentChunk);
            break;
        default:
            // Read past the ignored or unknown chunks
            m_CurrentChunk->bytesRead += static_cast<unsigned int>(fread(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer));
            break;
        }

        // Add the bytes read from the last chunk to the previous chunk passed in.
        pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
    }

    // Free the current chunk and set it back to the previous chunk (since it started that way)
    delete m_CurrentChunk;
    m_CurrentChunk = pPreviousChunk;
}

///   This function reads in a chunk ID and it's length in bytes
void Loader3ds::readChunk(Chunk3ds *pChunk) {
    // This reads the chunk ID which is 2 bytes.
    // The chunk ID is like OBJECT or MATERIAL.  It tells what data is
    // able to be read in within the chunks section.
    pChunk->bytesRead = static_cast<unsigned int>(fread(&pChunk->ID, 1, 2, m_FilePointer));
    // Then, we read the length of the chunk which is 4 bytes.
    // This is how we know how much to read in, or read past.
    pChunk->bytesRead += static_cast<unsigned int>(fread(&pChunk->length, 1, 4, m_FilePointer));
}

///   This function reads in a string of characters
int Loader3ds::getString(char *pBuffer) {
    int index = 0;
    // Read 1 byte of data which is the first letter of the string
    fread(pBuffer, 1, 1, m_FilePointer);
    // Loop until we get NULL
    while (*(pBuffer + index++) != 0)
        // Read in a character at a time until we hit NULL.
        fread(pBuffer + index, 1, 1, m_FilePointer);

    // Return the string length, which is how many bytes we read in (including the NULL)
    return static_cast<int>(strlen(pBuffer)) + 1;
}


///   This function reads in the RGB color data
void Loader3ds::readColorChunk(Material3ds *pMaterial, Chunk3ds *pChunk) {
    // read the color chunk info
    readChunk(m_TempChunk);
    // read in the R G B color (3 bytes - 0 through 255)
    m_TempChunk->bytesRead += static_cast<unsigned int>(fread(pMaterial->color, 1, m_TempChunk->length - m_TempChunk->bytesRead, m_FilePointer));
    // Add the bytes read to our chunk
    pChunk->bytesRead += m_TempChunk->bytesRead;
}


///  this method reads transparency data
void Loader3ds::readTranspChunk(Material3ds *pMaterial, Chunk3ds *pChunk) {
    readChunk(m_TempChunk);
    float percent=0;
    switch (m_TempChunk->ID) {
    case CHUNK_PERCENT_INT: {// int format
        int i=0;
        m_TempChunk->bytesRead += static_cast<unsigned int>(fread(&i, 1, 2, m_FilePointer));
        percent = (float)i;
        break;
    }
    case CHUNK_PERCENT_FLOAT: // float format
        m_TempChunk->bytesRead += static_cast<unsigned int>(fread(&percent, 1, 4, m_FilePointer));
        break;
    default:
        break;
    }
    pMaterial->opacity=1.0f-(percent/100.0f);
    // Add the bytes read to our chunk
    pChunk->bytesRead += m_TempChunk->bytesRead;
}


///   This function reads in the indices for the vertex array
void Loader3ds::readVertexIndices(Mesh3ds *pObject, Chunk3ds *pCurrChunk) {
    // read in the number of faces that are in this object (int)
    int numFaces=0;
    pCurrChunk->bytesRead += static_cast<unsigned int>(fread(&numFaces, 1, 2, m_FilePointer));

    // Alloc enough memory for the faces and initialize the structure
    pObject->vIndices.clear();
    pObject->vIndices.assign((unsigned int)numFaces*3,0);

    unsigned short index=0;
    // Go through all of the faces in this object
    for(unsigned int i = 0; i < (unsigned int)numFaces; i++) {
        // Next, we read in the A then B then C index for the face, but ignore the 4th value.
        // The fourth value is a visibility flag for 3D Studio Max, we don't care about this.
        for(int j = 0; j < 4; j++) {
            // read the first vertex index for the current face
            pCurrChunk->bytesRead += static_cast<unsigned int>(fread(&index, 1, sizeof(index), m_FilePointer));
            if(j < 3)  // Store the index in our face structure.
                pObject->vIndices[i*3+j]=index;
        }
    }
}


void Loader3ds::readUVCoordinates(Mesh3ds *pObject, Chunk3ds *pCurrChunk) {
    // read in the number of UV coordinates there are (int)
    int numTexVertex=0;
    pCurrChunk->bytesRead += static_cast<unsigned int>(fread(&numTexVertex, 1, 2, m_FilePointer));
    // Allocate memory to hold the UV coordinates
    pObject->vTexCoords.clear();
    pObject->vTexCoords.assign((unsigned int)numTexVertex,vec2f(0,0));
    // read in the texture coodinates (an array 2 float)
    pCurrChunk->bytesRead += static_cast<unsigned int>(fread(&pObject->vTexCoords[0], 1, pCurrChunk->length - pCurrChunk->bytesRead, m_FilePointer));
}


void Loader3ds::readVertices(Mesh3ds *pObject, Chunk3ds *pCurrChunk) {
    // read in the number of vertices (int)
    int numVerts=0;
    pCurrChunk->bytesRead += static_cast<unsigned int>(fread(&numVerts, 1, 2, m_FilePointer));
    // Allocate the memory for the verts and initialize the structure
    pObject->vCoords.clear();
    pObject->vCoords.assign(static_cast<unsigned int>(numVerts),vec3f(0,0,0));
    // read in the array of vertices (an array of 3 floats)
    pCurrChunk->bytesRead += static_cast<unsigned int>(fread(&pObject->vCoords[0], 1, pCurrChunk->length - pCurrChunk->bytesRead, m_FilePointer));
}


///   This function reads in the material name assigned to the object and sets the materialID
void Loader3ds::readObjectMaterial(Mesh3ds *pObject, Chunk3ds *pCurrChunk) {
    MaterialRef3ds currMaterial;
    // here we read the material name
    pCurrChunk->bytesRead += getString(currMaterial.matName);
    // read face references:
    int numEntries=0;
    pCurrChunk->bytesRead += static_cast<unsigned int>(fread(&numEntries, 1, 2, m_FilePointer));
    currMaterial.vFace.assign(static_cast<unsigned int>(numEntries),0);
    pCurrChunk->bytesRead += static_cast<unsigned int>(fread(&currMaterial.vFace[0], 1, numEntries*sizeof(unsigned short), m_FilePointer));
    pObject->material.push_back(currMaterial);
}

