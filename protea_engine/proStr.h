/** @file proStr.h
 \brief A collection of string utility functions

 \author Gerald Franz, www.viremo.de
started as utils_str.h  2000-02-03\n
current version 2008-05-22

 $Revision: 1.6 $ 

  License notice (zlib style):

 (c) 2000-2009 by Gerald Franz, www.viremo.de

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

#ifndef _PRO_STR_H
#define _PRO_STR_H

#include <vector>
#include <string>
#include <sstream>

#ifdef _DEBUG
/// debugging macro: prints a variable name and value
  #define P(A) cout << #A << ": " << (A) << endl;
/// debugging macro: prints an STL container
  #define PV(A) { cout << #A << ": [ "; for (size_t __i__=0; __i__<A.size(); ++__i__) std::cout << A[__i__] << ' '; cout << "]" << endl; }
/// debugging macro: prints an argument
  #define dout(A) cout << (A) << flush;
#else
/// debugging macro: prints a variable name and value
  #define P(A)
/// debugging macro: prints an STL container
  #define PV(A)
/// debugging macro: prints an argument
  #define dout(A)
#endif


//--- string utilities ---------------------------------------------

/// general tool for splitting strings into pieces.
size_t split(const std::string & input, std::vector<std::string> & output, const std::string & separators=" \t\n\015");
/// tool for splitting strings into pieces preserving quotations (").
size_t qsplit(const std::string & input, std::vector<std::string> & output, const std::string & separators=" \t\n\015");
/// splits char array into tokens
/** the input string is changed by adding additional 0 characters after all tokens */
char** tokenize(char* input);
/// general tool for stripping definable characters from both ends
std::string trim(const std::string& s, const std::string & pattern=", \t\n\015");
/// replaces all occurences of search with repl in s
std::string replaceAll(std::string s, const std::string & search, const std::string & repl);
/// replaces occurences of pattern in string s by a single ch
std::string replaceChars(const std::string & s, const std::string & pattern=" \t\n\015", char ch=' ');
/// trims n characters from the end of string s
void operator-=(std::string & s, unsigned int n);
/// searches a pair of matching opening and closing characters
size_t match(const std::string & s, size_t pos, char chOpen, char chClose);
/// tests whether a string corresponds to a wildcard (*) pattern
/** the pattern may contain asterisks (*) and semicolons to separate multiple alternative patterns. The method is case-insensitive. */
bool wildcardMatch(std::string s, const std::string & pattern);
/// concatenates all elments of a vector<class T> into a single string
template <class T> std::string join(const std::vector<T> & v, const std::string sep =", "){
	if(v.begin()==v.end()) return std::string();
	typename std::vector<T>::const_iterator it = v.begin();
	std::stringstream sstr;
	sstr << *it;
	std::string s(sstr.str());
	for(++it; it!=v.end(); ++it) {
		sstr.str("");
		sstr.clear();
		sstr << *it;
		s+=sep+sstr.str();
	}
	return s;
}


/// converts integer value to string
std::string i2s(long i);
/// converts float value to string
std::string f2s(double f);
/// converts float value to string, specify number of digits
std::string f2s(double f,unsigned short nDigits);
/// converts bool value b to string, optionally specify mode ( 0 1, true false)
std::string b2s(bool b, bool asText=false);
/// converts character to string
std::string c2s(char ch);
/// converts string to integer value
int s2i(const std::string & s);
/// converts string to unsigned integer value
unsigned int s2ui(const std::string & s);
/// converts string to float value
float s2f(const std::string & s);
/// converts string to bool value
bool s2b(const std::string & s);
/// converts string to float vector.
/** The string is splitted according to optional argument separators.
 \return the number of generated floats.*/
size_t s2f(const std::string & s, std::vector<float> & vFloat, const std::string & separators=", \t\n\015");
/// converts string to int vector.
/** The string is splitted according to optional argument separators.
 \return the number of generated ints.*/
size_t s2i(const std::string & s, std::vector<int> & vInt, const std::string & separators=", \t\n\015");

/// converts a string to upper case, if possible.
std::string toUpper(const std::string & s);
/// converts a string to lower case, if possible.
std::string toLower(const std::string & s);
/// encodes a string to its string representation: special characters are protected by a backslash
std::string repr(const std::string & s, bool parentheses=false);
/// decodes a string representation into a bare string: escape sequences within quotes are translated to their ASCII counterparts
std::string derepr(const std::string & s);
/// converts a hexadecimal string into an unsigned int
unsigned int hex2ui(const std::string & s);
/// tests whether a string defines a (float=2 or integer=1) number
int isNumber(const std::string & s);

#ifdef _WIN32
/// windows-specific wide string to string conversion function
std::string ws2s(const std::wstring &wstr);
/// windows-specific string to wide string conversion function
std::wstring s2ws(const std::string &str);
#endif // _WIN32


#endif //_PRO_STR_H
