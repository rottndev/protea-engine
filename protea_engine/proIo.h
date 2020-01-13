#ifndef _PRO_IO
#define _PRO_IO
/** @file proIo.h
 \brief A collection of file input/output utility functions and classes.

 \author Gerald Franz, www.viremo.de

current version 2009-02-05

 $Revision: 1.4 $ 

  License notice (zlib license):

 (c) 2007-2009 by Gerald Franz, www.viremo.de

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
#include <vector>

//--- struct SharedMemory ------------------------------------------

/// sinternal tructure for handling a shared memory area
typedef struct {
	/// name handle
	char* name;
	/// shared memory size in bytes
	unsigned long size;
	/// pointer to data
	void* data;
	/// connection counter 
	int* pCounter;
	/// file descriptor
	int fd;
} SharedMemory;

/// opens a named shared memory area
void* SharedMemory_open(SharedMemory* shm, const char* name,unsigned long size );
/// closes a named shared memory area
int SharedMemory_close(SharedMemory* shm);


//--- class io -----------------------------------------------------
/// class facilitating file input/output operations.
class io {
public:
    /// loads a string from a file
    static std::string load(const std::string & filename);
    /// saves a string to a file
    static bool save(const std::string & s, const std::string & filename);

    /// opens a pipe from an executed external command and returns its standard output.
    static std::string exec(std::string cmd);
	/// platform independent asynchronous subprocess exec/spawn method
	static int spawn(const char * path,  const char **argv, int wait);
    /// puts all directory entries of path in target string vector.
    /**
     \param path (optional) defines directory path
     \param filter (optional) sets filter with * wildcards
     \return a string vector containing the file names */
    static std::vector<std::string> dir(const std::string & path=".", const std::string & filter="*");
    /// determines whether path points to a directory
    static bool isDir(const std::string & path);
    /// reads a line from a FILE* into a c++ string
    /** \return the number of read chars. */
    static size_t getline(FILE* fp, std::string & s);
    /// tests whether file filename exists and returns TRUE in case of existence.
    static bool fileExist(const std::string & filename);
    /// returns the file size in bytes.
    static unsigned int fileSize(FILE* fp);
    /// returns currently available drives
    /** Under Windows, a string consisting of individual drive letters is returned. Under Unix,
        always an empty string is returned, because the concept of explicit dirves does not exist here. */
    static std::string drives();
    /// removes OS dependencies from a filepath string
    static std::string unifyPath(const std::string & source);
    /// returns current working directory
    static std::string cwd();
    /// changes the current working directory
    static int chdir(const std::string & path);
	/// opens an URL in the system's default browser
	void openURL(const std::string & url);
};

//--- class cmdLine --------------------------------------------
/// a simple static class for preparsing command line arguments and options
class cmdLine {
public:
    /// parses command line and command line options.
    /** \param argc standard C main() number of arguments.
     \param argv standard C main() pointer to arguments char array.
     \param optsAsArgs optional parameter that causes all command line tokens to be interpreted as arguments.
     */
    static void interpret( int argc, char **argv, bool optsAsArgs=false );
    /// returns whether command line has already been parsed
    static bool parsed() { return isParsed; };
    /// returns command line option number i
    static char opt(unsigned int i) { return vOpt[i][0]; }
    /// returns true if a single char command line option exists
    static bool opt(const char c);
    /// returns optional argument of a single char command line option, or ""
    static std::string optArg(const char c);
    /// returns a reference of command line argument i
    static const std::string & arg(unsigned int i) { return vArg[i]; }
    /// returns number of command line arguments
    static size_t nArg() { return vArg.size(); }
    /// returns number of command line options
    static size_t nOpt() { return vOpt.size(); }
    /// returns path to main's directory
    static std::string dir() { return ownDir; }
    /// returns main's command name
    static std::string cmd() { return ownCmd; }
    /// sets program's name.
    static void name(const std::string & s) { name_=s; }
    /// returns program's name.
    static const std::string & name() { return name_; }
    /// sets program's author.
    static void author(const std::string & s) { author_=s; }
    /// returns program's author.
    static const std::string & author() { return author_; }
    /// sets program's version.
    static void version(const std::string & s) { version_=s; }
    /// returns program's version.
    static const std::string & version() { return version_; }
    /// sets program's date.
    static void date(const std::string & s) { date_=s; }
    /// returns program's date.
    static const std::string & date() { return date_; }
    /// sets program's short description.
    static void shortDescr(const std::string & s) { shortDescr_=s; }
    /// returns program's short description.
    static const std::string & shortDescr() { return shortDescr_; }
    /// sets program's long description.
    static void descr(const std::string & s) { descr_=s; }
    /// returns program's long description.
    static const std::string & descr() { return descr_; }
    /// sets program's long description.
    static void usage(const std::string & s) { usage_=s; }
    /// returns program's long description.
    static const std::string & usage() { return usage_; }

    /// returns a help string containing previously setted data.
    static const std::string help();
protected:
    /// stores command line arguments
    static std::vector<std::string> vArg;
    /// stores command line options
    static std::vector<std::string> vOpt;
    /// stores path to main's directory
    static std::string ownDir;
    /// stores main's command name
    static std::string ownCmd;
    /// stores whether class is already initialized
    static bool isParsed;

    /// stores program's name.
    /** If no name is provided by the user, the program's command line
     call is taken as default. */
    static std::string name_;
    /// stores program's author.
    static std::string author_;
    /// stores program's version.
    static std::string version_;
    /// stores program's date.
    static std::string date_;
    /// stores program's short description.
    static std::string shortDescr_;
    /// stores program's long description.
    static std::string descr_;
    /// stores program's usage.
    static std::string usage_;
};

#endif // _PRO_IO
