#include "proIo.h"
#include "proStr.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <fstream>
#include <algorithm>
# include <sys/stat.h>

#if defined __WIN32__ || defined WIN32
# include <windows.h>
# include <direct.h>
#  include <process.h>
#else
#  include <sys/mman.h>
#  include <fcntl.h>
#  include <sys/wait.h>
#endif

#ifndef _MSC_VER
# include <dirent.h>
#endif

using namespace std;

//--- struct SharedMemory ------------------------------------------

void* SharedMemory_open(SharedMemory* shm, const char* name, unsigned long size ) {
	shm->data=0;
	int counter=0;
	const unsigned int extraSize = sizeof(unsigned int);
#if defined __WIN32__ || defined WIN32
	// first try opening an existing file mapping:
	shm->fd = (int)OpenFileMapping( FILE_MAP_ALL_ACCESS, 0, name);
	// then try to create a new one:
	if(!shm->fd) {
		shm->fd = (int)CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size+extraSize, name);
		counter=1;
	}
	if (!shm->fd) {
		fprintf(stderr, "Could not open or create shared memory \"%s\":%u.\n", name, (unsigned int)GetLastError());
		return 0;
	}
	shm->data =  MapViewOfFile((HANDLE)shm->fd, FILE_MAP_ALL_ACCESS, 0, 0, size+extraSize );
	if(!shm->data) {
		fprintf(stderr, "Could not map shared memory \"%s\":%u.\n", name, (unsigned int)GetLastError()); 
		CloseHandle((HANDLE)shm->fd);
		return 0;
	}
	shm->name = (char*)malloc(strlen(name)+1);
	strcpy(shm->name, name);
#else
	int mode = (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	char * n = (char*)malloc(strlen(name)+2);
	n[0]='/';
	n[1]=0;
	strcat(n,name);
	shm->fd = shm_open (n, O_RDWR, mode );
	if (shm->fd<0) {
		shm->fd = shm_open (n, O_RDWR | O_CREAT, mode );
		if (shm->fd<0) {
			fprintf(stderr, "Could not open or create shared memory \"%s\":%u.\n", name, errno);
			free(n);
			return 0;
		}
		counter = 1;
		lseek(shm->fd, size+extraSize, SEEK_SET);
		write(shm->fd, "", 1);
		lseek(shm->fd, 0, SEEK_SET);
	}
	shm->data = mmap( 0, size+extraSize, PROT_READ | PROT_WRITE, 
		MAP_FILE|MAP_SHARED, shm->fd, 0);
	if(shm->data<=0) {
		fprintf(stderr, "Could not map shared memory \"%s\":%u.\n", name, errno); 
		close(shm->fd);
		free(n);
		return 0;
	}
	shm->name = n;	
#endif
	shm->pCounter = (int*)((char*)shm->data+size);
	if(counter) *shm->pCounter =counter;
	else ++(*shm->pCounter);
	shm->size = size;
	return shm->data;
}

int SharedMemory_close(SharedMemory* shm) {
	if(!shm->data) return -1;
	--(*shm->pCounter);
#if defined __WIN32__ || defined WIN32
	if(!UnmapViewOfFile(shm->data)) 
		fprintf(stderr, "Could not unmap shared memory:%u.\n", (unsigned int)GetLastError()); 
	int ret = (CloseHandle((HANDLE)shm->fd)==0) ? -1 : 0;
#else
	int counter = *shm->pCounter;
	if(munmap(shm->data, getpagesize())<0)
		fprintf(stderr, "Could not unmap shared memory:%u.\n", errno); 
	int ret = 0;
	if(counter < 1) 
		ret = shm_unlink(shm->name);
#endif
	free((void*)shm->name);
	shm->name = 0;
	shm->size=0;
	shm->data=0;
	return ret;
}


//--- class io -----------------------------------------------------

string io::load(const string & filename) {
    ifstream file(filename.c_str(), std::ios::in);
    if (!file.good()) {
        cerr << "io::load(string) ERROR: " << filename << " file error or file not found!"<< endl;
        return string();
    }
    string s, line;
    while(!file.eof()) {
        std::getline(file,line);
        s+=line+'\n';
    }
    file.close();
    return s;
}

bool io::save(const string & s, const string & filename) {
    ofstream file(filename.c_str(), std::ios::out);
    if (!file.good()) return false;
    file << s << flush;
    file.close();
    return true;
}

string io::exec(string cmd) {
#ifdef _MSC_VER
    cerr << "This function is not yet implemented for _MSC_VER-compilers";
    return "";
#else
#if defined __WIN32__ || defined WIN32
    cmd=replaceAll(cmd,"/","\\");
#endif
    FILE *myPipe=NULL;
    myPipe=popen(cmd.c_str(),"r");
    if(!myPipe) return "";

    string returnStr;
    char buffer[256];
    while(fgets(buffer,256,myPipe)) returnStr+=buffer;

    pclose(myPipe);
    return returnStr;
#endif //_MSC_VER
}

int io::spawn(const char * path,  const char **argv, int wait) {
	 char * p = (char*)path;
#if !defined __WIN32__ && !defined WIN32	
	if(p[0]!='.'&&p[0]!='/') { // add relative path prefix if necessary
		p=(char*)malloc(strlen(path)+3);
		strcpy(p,"./");
		strcat(p,path);
	}
#endif
	// duplicate path as first arg:
	unsigned int argc = 0;
	while(argv[argc]) argc++;
	const char ** arg = (const char**)malloc(sizeof(char*)*(argc+2));
	arg[0]=p;
	argc = 0;
	while(argv[argc]) {
		arg[argc+1] =argv[argc];
		argc++;
	}
	arg[argc+1] =0;
#if defined __WIN32__ || defined WIN32
	int ret = _spawnv( wait ? _P_WAIT : _P_NOWAIT, p, arg);
	free(arg);
#else
	int ret = fork ();
	switch (ret) {
	case -1:
		return -1;
	case 0: // child process
		exit( execvp (p, (char* const*)arg) );
	default: // parent process
		if(wait) {
			int status = 0;
			if(waitpid (ret, &status, 0) != ret)  {
				ret= -1;
				break;
			}
			if (WIFSIGNALED (status))  return WTERMSIG(status);
			if(WIFSTOPPED (status))  return WSTOPSIG(status);
			ret = WEXITSTATUS(status);
		}
	}
	// free(arg); // dumps for unknown reasons
#endif
	if(p!=path) free(p);
	return ret;	
}

vector<std::string> io::dir(const std::string & path, const std::string & filter) {
	vector<std::string> target;
#ifdef _MSC_VER
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;

    string tmp = path;
    tmp.append("\\*.*");
    hFind = FindFirstFile(tmp.c_str(), &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        cerr << "io::dir() ERROR: Invalid File Handle.\n";
        return target;
    }

    while(FindNextFile(hFind, &findFileData)) {
        string s(findFileData.cFileName);
        if((findFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)&&((s==".")||(s=="..")))
			continue;
        if(wildcardMatch(s,filter)) target.push_back(s);
    }
    bool bClosed;
    if(bClosed = FindClose(hFind) == false)
		cerr << "io::dir() ERROR: Unable to close file handle.\n";
#else
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir(path.c_str())) == NULL) {
        cerr << "ERROR io::dir: unable to open directory " << path << endl;
        return target;
    }
    while ((ent = readdir(dir)) != NULL) {
        string s(ent->d_name);
        if((s==".")||(s=="..")) continue;
        if(!filter.size()||wildcardMatch(s,filter)) target.push_back(s);
    }
    if (closedir(dir) != 0)
        cerr << "ERROR io::dir: unable to close directory " << path << endl;
#endif
    sort(target.begin(),target.end());
    return target;
}

bool io::isDir(const std::string & path) {
#ifdef _MSC_VER
    struct _stat buf;
    if ( _stat(path.c_str(),&buf) == 0 )
    {
      if(buf.st_mode & _S_IFDIR)
        return true;
      else
        return false;
    }
#else
    struct stat buf ;
    if ( stat(path.c_str(),&buf) == 0 )
        return buf.st_mode & S_IFDIR;
#endif
    return false ;
}

string io::drives() {
#if defined __WIN32__ || defined WIN32
    int drive, curdrive;
    string s;
    curdrive = _getdrive(); // save current drive
    for( drive = 0; drive < 26; ++drive )
        if( !_chdrive( drive+1 ) ) s+= (drive + 'A');
    _chdrive( curdrive ); // restore original drive
    return s;
#else
    return "";
#endif
}

size_t io::getline(FILE* fp, std::string & s) {
    s.erase();
    const size_t bufSz=4096;
    char chbuf[bufSz];
    chbuf[0]=0;
    fgets(chbuf,bufSz,fp);
    if(strlen(chbuf)) {
        s+=chbuf;
        while((s[s.size()-1]!='\n')&&!feof(fp)) {
            fgets(chbuf,bufSz,fp);
            if(strlen(chbuf)) s+=chbuf;
        }
        if(s[s.size()-1]=='\n') s.erase(s.size()-1,1);
    }
    return s.size();
}

bool io::fileExist(const string & filename) {
    ifstream file(filename.c_str(), ios::in);
    return file.good();
}

unsigned int io::fileSize(FILE* fp) {
    long currPos=ftell(fp);
    fseek( fp, 0, SEEK_END);
    unsigned int size=ftell(fp);
    fseek( fp, currPos, SEEK_SET);
    return size;
}

string io::unifyPath(const string & source) {
  string s(source);
  //turn all slashes into unix notation because windows doesn't care...
  s = replaceAll(s, "\\", "/");
#if defined __WIN32__ || defined WIN32
  //...except if you want to access files over the net like \\uni\home\... the first two
  //slashes than need to be "\\" instead of "//".
  s = replaceAll(s, "//", "\\\\");
#endif
  // make sure there are no two slashes next to each other
  s = replaceAll(s, "//", "/");
  return s;
}

string io::cwd() {
    char buf[1024];
#ifdef _MSC_VER
    return unifyPath(_getcwd ( buf, 1023 ));
#else
    return unifyPath(getcwd ( buf, 1023 ));
#endif
}

int io::chdir(const string & path) {
#if defined __WIN32__ || defined WIN32
    if(path.size()>1 && path[1]==':' && toupper(path[0])+1-'A'!=_getdrive())
        _chdrive(toupper(path[0])+1-'A');
    return _chdir(replaceAll(unifyPath(path),"/","\\").c_str());
#else
    return ::chdir(unifyPath(path).c_str());
#endif
}

void io::openURL(const std::string & url) {
#ifdef WIN32
    ShellExecute(GetActiveWindow(),
         "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif defined(__APPLE__)
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "open %s", url.c_str());
    system(buffer);
#else
    const char *apps[] = {"x-www-browser",
                    "firefox", // iceweasel has an alias on debian
                    "opera",
                    "mozilla",
                    "galeon",
                    "konqueror", NULL};

    char buffer[256];
    int i = 0;

    while (apps[i]) {
        snprintf(buffer, sizeof(buffer), "which %s >/dev/null", apps[i]);
        if (system(buffer) == 0) {
            snprintf(buffer, sizeof(buffer), "%s %s", apps[i], url.c_str());
            system(buffer);
            return;
        }
        ++i;
    }
#endif
}

//--- class cmdLine ------------------------------------------------

vector<string> cmdLine::vArg;
vector<string> cmdLine::vOpt;
string cmdLine::ownDir="";
string cmdLine::ownCmd="";
bool cmdLine::isParsed=false;

string cmdLine::name_="";
string cmdLine::author_="";
string cmdLine::version_="";
string cmdLine::date_="";
string cmdLine::shortDescr_="";
string cmdLine::descr_="";
string cmdLine::usage_="";

void cmdLine::interpret( int argc, char **argv, bool optsAsArgs ) {
    if(isParsed) return;
    isParsed=true;
#if defined __WIN32__ || defined WIN32
    char delimiter='\\';
#else
    char delimiter='/';
#endif
    string arg0=argv[0];
    ownDir=arg0.substr(0,arg0.rfind(delimiter)+1);
    ownCmd=arg0.substr(arg0.rfind(delimiter)+1);

    for(int i=1; i<argc; i++) { // parse args and options
        if(optsAsArgs) {
            vArg.push_back(argv[i]);
            continue;
        }
        if((strcmp(argv[i],"--help")==0)||(strcmp(argv[i],"-h")==0)) {
            if(usage_.size()) {
                cerr << help() << endl;
                exit(1);
            }
        }
        if(argv[i][0]=='-') vOpt.push_back(&argv[i][1]);
        else vArg.push_back(argv[i]);
    }
    if(name_=="") name_=ownCmd;
}

bool cmdLine::opt(char c) {
    for(unsigned int i=0; i<vOpt.size(); i++)
        if(vOpt[i][0]==c) return true;
    return false;
}

string cmdLine::optArg(char c) {
    for(unsigned int i=0; i<vOpt.size(); i++)
        if(vOpt[i][0]==c) return vOpt[i].substr(1);
    return "";
}

const string cmdLine::help() {
    string s="\n"+name_ +" - " + shortDescr_
        +"\n\nversion "+version_+" (c) "+date_+" by "+author_
        +"\n\nusage: "+cmd()+" [-h(elp)] "+usage_+"\n";
    if(descr_!="") s+="\n"+descr_+"\n";
    return s;
}


