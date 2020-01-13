/** @file proMath.h
 \brief mathematical utility classes and functions 
 \version 2008-10-08

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

#ifndef _PRO_MATH_H
#define _PRO_MATH_H

#include <cmath>
#include <iostream>
#include <vector>

using namespace std;

class vec3f;
class vec6f;
class plane;

//--- constants and enums ------------------------------------------
/// defines PI
const float PI=3.14159265358979323846f;
/// defines PI/180, for angle conversions from deg to rad.
const float PI_180=PI/180.0f;
/// defines PI/180, for angle conversions from deg to rad.
const float DEG2RAD=PI_180;
/// defines 180/PI, for angle conversions from rad to deg.
const float RAD2DEG=180.0f/PI;
/// defines meaningful names for vec2f, vec3f, vec4f, and vec6f ordinates.
typedef enum { X=0, Y=1, Z=2, W=3, H=3, P=4, R=5 } axis;

///share the normal math.h definition
#ifndef M_PI
#  define M_PI PI
#endif

/// defines upper limit of regular float values
#if !defined _MSC_VER && !defined FLT_MAX
#  define FLT_MAX 3.40282347e+38F
#endif
/// defines lower limit of regular float values
#if !defined _MSC_VER && !defined FLT_MIN
#  define FLT_MIN 1.17549435e-38F
#endif
/// defines C99 HUGE_VALF constant if necessary
#ifndef HUGE_VALF
#ifdef _MSC_VER
#include <float.h>
#define HUGE_VALF FLT_MAX
#else
#  define HUGE_VALF 1.0f/0.0f
#endif
#endif


/// just a small double value
const double EPSILON = 0.00000001;
/// just a small float value
const double EPSILONF = 0.000001;

/// this array defines names for all 32 bits of an unsigned int
const unsigned int BIT[]= {
    0x1,0x2,0x4,0x8, 0x10,0x20,0x40,0x80, 0x100,0x200,0x400,0x800,
    0x1000,0x2000,0x4000,0x8000, 0x10000,0x20000,0x40000,0x80000,
    0x100000,0x200000,0x400000,0x800000, 0x1000000,0x2000000,
    0x4000000,0x8000000, 0x10000000,0x20000000,0x40000000,0x80000000 };

/// this array defines names for all 4 bytes of an unsigned int
const unsigned int BYTE_ID[]= { 0xFF, 0xFF00, 0xFF0000, 0xFF000000 };


// --- templates ---------------------------------------------------
/// sinus function for degrees
template <class T> double dsin(T x) { return sin((double(x))*PI_180); }
/// arcus sinus function for degrees
template <class T> double dasin(T x) { return asin(double(x))/PI_180; }
/// cosinus function for degrees
template <class T> double dcos(T x) { return cos((double(x))*PI_180); }
/// arcus cosinus function for degrees
template <class T> double dacos(T x) { return acos(double(x))/PI_180; }
/// tangens function for degrees
template <class T> double dtan(T x) { return tan((double(x))*PI_180); }
/// arcustangens function for degrees
template <class T> double datan(T x) { return atan(double(x))/PI_180; }

//--- math functions & templates -----------------------------------
/// returns a not a number value
double NaN();
#if defined _MSC_VER
/// tests for NaN in a platform-independent manner
#define isnan _isnan
#endif

    /// simple signum function for floats
    /**   Macro for extracting the sign of float or double values. */
#define fsign(fnum) ((fnum)<0.0?-1:(fnum)>0.0?1:0)
    /// returns maximum of 2 values
#ifndef maximum
#  define maximum(a,b)            (((a) > (b)) ? (a) : (b))
#endif
    /// returns minimum of 2 values
#ifndef minimum
#  define minimum(a,b)            (((a) < (b)) ? (a) : (b))
#endif
/// clamps a value to the range [valMin,valMax]
template <class T> T clamp(const T & value, const T & valMin, const T & valMax) {
    return (value>valMax) ? valMax : (value<valMin) ? valMin : value; }

/// returns distance between P1(x1|y1|z1) and P2(x2|y2|z2)
inline float dist(float x1, float y1,float z1, float x2,float y2,float z2) {
    return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2)); };
/// returns distance between P1(x1|y1) and P2(x2|y2)
inline float dist(float x1, float y1, float x2, float y2) {
    return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)); };
/// returns minimum absolute value
inline float minAbs(float f1, float f2) {
    return (fabs(f1)<fabs(f2) ? fabs(f1) : fabs(f2)); };
/// returns minimum distance between P(x1|y1) and line segment((x2|y2)|(x3|y3))
float distPointSeg(float x1, float y1, float x2, float y2,
                   float x3, float y3);
/// returns intersection point between ray(origin|dir) and plane pl
vec3f * intersection(const vec3f & origin, const vec3f & dir, const plane & pl, bool isRay=true);
/// returns angle between line p0|p1 and line p0|p2
float angleXY(const vec3f & p0, const vec3f & p1, const vec3f & p2);
/// returns angle between point xy1 and point xy2
float angleXY(float x1, float y1, float x2, float y2);
/// returns angle between origin and point xy
float angleXY(float x, float y);
/// returns angle between line xy1|xy2 and line xy1|xy3
float angleXY(float x1, float y1, float x2, float y2, float x3, float y3);
/// generic signum function template
template <class T> int sgn(T x) { return x>0 ? 1 : (x<0 ? -1 : 0); };
/// returns minimum of 3 values
template <class T> T min3(T value0, T value1, T value2) { return min(min(value0,value1),value2); };
/// returns maximum of 3 values
template <class T> T max3(T value0, T value1, T value2) { return max(max(value0,value1),value2); };

//--- random-Class -------------------------------------------------
/// an extension of c random number functions.
class rnd {
public:
    /// default constructor.
    rnd(unsigned int seed=0);
    /// returns a random integer value.
    int get();
    /// returns an int number between 0 < max.
    int get(int max);
    /// returns a float number between 0.0 and 1.0.
    float getf();
    /// returns a float number between 0.0 and f.
    float getf(float f) { return getf()*f; };
protected:
    /// stores a seed for the case no seed is provided by the constructor call.
    /** The default seed will be generated from system time or
     from previously instanciated rnd object constructors. */
    static unsigned int defaultSeed;
};

//--- class vec2f --------------------------------------------------
/// a class for 2d vector and vertex geometry operations.
/** The vec2f class is intentionally free of virtual methods and takes exactly
 8 bytes of memory. It is not meant as generic vector class (use vec3f instead),
 but only for efficient storing of 2D mass coordinate information. */
class vec2f {
public:
    /// default constructor
    vec2f(float x=0.0f, float y=0.0f) { coord[X]=x; coord[Y]=y; };
    /// copy constructor
    vec2f(const vec2f & v) { coord[X]=v[X]; coord[Y]=v[Y]; };
    /// constructor for vector between two given vertices
    vec2f(const vec2f & p1, const vec2f & p2) {
        coord[X]=p2[X]-p1[X]; coord[Y]=p2[Y]-p1[Y]; };

    /// copy operator
    const vec2f & operator=(const vec2f &source);
    /// sets vector ordinates to new values (x_|y_).
    void set(float x_, float y_) { coord[X]=x_; coord[Y]=y_; };
    /// sets vector ordinates to new values (x_[0]|x_[1]).
    void set(float * x_) { coord[X]=x_[0]; coord[Y]=x_[1]; };

    /// comparison operator equality
    bool operator==(const vec2f &vt) const;
    /// comparison operator inequality
    bool operator!=(const vec2f & vt) const { return !((*this)==vt); };
    /// returns reference to single ordinate
    float & operator[](unsigned int i) { return coord[i%2]; };
    /// returns single ordinate value
    float operator[](unsigned int i) const { return coord[i%2]; };
    /// returns coordinate array
    const float * coords() const { return coord; };

    /// translates object by dx,dy
    void translate(float dx, float dy) { coord[X]+=dx; coord[Y]+=dy; };
    /// translates object by vector v, optionally scaled by factor n
    void translate(const vec2f & v, float n=1.0f) { translate(v[X]*n, v[Y]*n); };
    /// translates n points starting at ordinate *x by  dx,dy
    /** This is an optimized low level translation function for large numbers of coherent
     coordinate data. */
    static void translate(float * x, float dx, float dy, size_t n=1);
    /// += operator. Sums correspondend ordinates. Identical to translate(v).
    void operator+=(const vec2f & v) { coord[X]+=v[X]; coord[Y]+=v[Y]; };
    /// scales this vector by f.
    const vec2f & operator*=(float f) { coord[X]*=f; coord[Y]*=f; return *this; };
    /// devides this vector by f.
    const vec2f & operator/=(float f) { coord[X]/=f; coord[Y]/=f; return *this; };
    /// vector scalar product operator
    inline float operator* (const vec2f & v) const {
        return v.coord[X]*coord[X] + v.coord[Y]*coord[Y]; }
    /// scales this vector to unit vector of length 1.0.
    const vec2f & normalize() { return *this*=(1.0f/length()); }

    /// returns the squared length.
    float sqrLength() const { return coord[X]*coord[X]+coord[Y]*coord[Y]; }
    /// returns absolute length.
    float length() const { return sqrt(sqrLength()); }
    
    /// vector addition operator
    inline const vec2f operator+(const vec2f & v) const { return vec2f(coord[X]+v[X], coord[Y]+v[Y]); }
    /// vector subtraction operator
    inline const vec2f operator-(const vec2f & v) const { return vec2f(coord[X]-v[X], coord[Y]-v[Y]); }
    /// operator multiplying a vector v with a scalar f
    inline const vec2f operator*(float f) const { return vec2f(coord[X]*f, coord[Y]*f); }

    /// operator for output of vec2f objects in streams
    friend std::ostream & operator<<(std::ostream & os, const vec2f & v);
protected:
    /// stores coordinate values.
    float coord[2];
};

//--- class vec3f --------------------------------------------------
class vec4f;
class vec6f;
class mat4f;
/// a class for vector and 3D vertex geometry operations.
/** The vec3f class is intentionally free of virtual methods and takes exactly
 twelve bytes of memory. This allows for using a pointer to a vec3f array instead
 of using pointers to floats, e.g., for OpenGL vertex arrays. */
class vec3f {
public:
    /// default constructor
    vec3f(float x=0.0f, float y=0.0f, float z=0.0f) { coord[X]=x; coord[Y]=y; coord[Z]=z; };
    /// copy constructor
    vec3f(const vec3f & v) { coord[X]=v[X]; coord[Y]=v[Y]; coord[Z]=v[Z]; }
    /// constructor from a vec4f
    vec3f(const vec2f & v) { coord[X]=v[X]; coord[Y]=v[Y]; coord[Z]=0.0f; }
    /// constructor from a vec4f
    vec3f(const vec4f & v);
    /// constructor for vector between two given vertices
    vec3f(const vec3f & p1, const vec3f & p2) {
        coord[X]=p2[X]-p1[X]; coord[Y]=p2[Y]-p1[Y]; coord[Z]=p2[Z]-p1[Z]; }
    /// copy operator
    const vec3f & operator=(const vec3f &source);
    /// sets vector ordinates to new values (x|y).
    void set(float x, float y) { coord[X]=x; coord[Y]=y; }
    /// sets vector ordinates to new values (x|y|z).
    void set(float x, float y, float z) {
        coord[X]=x; coord[Y]=y; coord[Z]=z; }
    /// sets vector ordinates to new values (p[0]|p[1]|p[2]).
    void set(const float * p) {
        coord[X]=p[0]; coord[Y]=p[1]; coord[Z]=p[2]; }
    /// sets vector to difference between two given vertices
    void set(const vec3f & p1, const vec3f & p2) {
        coord[X]=p2[X]-p1[X]; coord[Y]=p2[Y]-p1[Y]; coord[Z]=p2[Z]-p1[Z]; }

    /// comparison operator equality
    bool operator==(const vec3f &vt) const;
    /// comparison operator inequality
    bool operator!=(const vec3f & vt) const { return !((*this)==vt); }
	/// comparison operator less than
	/** mianly for sorting purposes */
	bool operator<(const vec3f &v) const;
    /// returns reference to single ordinate
    float & operator[](unsigned int i) { return coord[i%3]; }
    /// returns single ordinate value
    const float & operator[](unsigned int i) const { return coord[i%3]; }
    /// returns coordinate array
    const float * coords() const { return coord; }
    /// unary negation operator
    const vec3f operator- () const { return vec3f(-coord[X], -coord[Y], -coord[Z]); }

    /// translates object by dx,dy
    void translate(float dx, float dy) { coord[X]+=dx; coord[Y]+=dy; }
    /// translates object by dx,dy,dz
    void translate(float dx, float dy, float dz) {
        coord[X]+=dx; coord[Y]+=dy; coord[Z]+=dz; }
    /// translates object by vector v, optionally scaled by factor n
    void translate(const vec3f & v, float n=1.0f) { translate(v[X]*n, v[Y]*n, v[Z]*n); }
    /// translates n points starting at ordinate *x by  dx,dy,dz.
    /** This is an optimized low level translation function for large numbers of coherent
     coordinate data. */
    static void translate(float * x, float dx, float dy, float dz, size_t n=1);
    /// += operator. Sums correspondent ordinates. Identical to translate(v).
    void operator+=(const vec3f & v) { coord[X]+=v[X]; coord[Y]+=v[Y]; coord[Z]+=v[Z]; }
    /// -= operator. Subtracts correspondent ordinates. Identical to translate(-v).
    void operator-=(const vec3f & v) { coord[X]-=v[X]; coord[Y]-=v[Y]; coord[Z]-=v[Z]; }
    /// rotates object around arbitrary axis from origin to p by angle.
    void rotate(float angle, vec3f p);
    /// rotates vector according to heading, pitch, and roll.
    void rotate(float h, float p, float r);
    /// rotates point (x|y|z) by angle around axis (origin|(ax|ay|az)).
    /** Vertex (ax|ay|az) has to be normalized before applying this static method.
     Otherwise an uncontrolled scaling occurs.*/
    static void rotate(float & x, float & y, float & z, float angle, float ax, float ay, float az);
    /// rotates n vertices starting at ordinate *x  by angle around axis (origin|(ax|ay|az)).
    /** This is an optimized low level rotation function for large numbers of coherent
     coordinate data. Vertex (ax|ay|az) has to be normalized before applying this static
     method. Otherwise an uncontrolled scaling occurs.*/
    static void rotate(float * x, float angle, float ax, float ay, float az, unsigned int n=1);
    /// scales object by sx,sy and optionally sz
    void scale(float sx, float sy, float sz=1);
    /// scales this vector by f.
    const vec3f & operator*=(float f) { coord[X]*=f; coord[Y]*=f; coord[Z]*=f; return *this; }
    /// divides this vector by f.
    const vec3f & operator/=(float f) { coord[X]/=f; coord[Y]/=f; coord[Z]/=f; return *this; }
    /// vector addition operator
    inline const vec3f operator+(const vec3f &v) const { return vec3f(coord[X]+v[X],coord[Y]+v[Y],coord[Z]+v[Z]); }
    /// vector subtraction operator
    inline const vec3f operator-(const vec3f &v) const { return vec3f(coord[X]-v[X],coord[Y]-v[Y],coord[Z]-v[Z]); }
    /// operator multiplying a vector v with a scalar f
    inline const vec3f operator*(float f) const { return vec3f(coord[X]*f,coord[Y]*f,coord[Z]*f); }
    /// operator dividing a vector v by a scalar f
    inline const vec3f operator/(float f) const { return vec3f(coord[X]/f,coord[Y]/f,coord[Z]/f); }
    /// vector scalar / dot product operator
    inline float operator* (const vec3f & v) const { 
        return v.coord[X]*coord[X] + v.coord[Y]*coord[Y] + v.coord[Z]*coord[Z]; }
    /// scales this vector to unit vector of length 1.0.
    const vec3f & normalize() { return *this*=(1.0f/length()); }
    /// inverts this vector, multiplies by -1.0
    void invert() { coord[X]=-coord[X];  coord[Y]=-coord[Y]; coord[Z]=-coord[Z]; }
    /// transforms this vector to an orthogonal projection of v2.
    void project(const vec3f & v2) { (*this)*=((*this) * v2)/sqrLength(); }
    /// transforms this vec3f by multiplying it with matrix m
    void transform(const mat4f & m);
    /// transforms this vec3f by applying the provided sixdof transformation.
    /** The vector is rotated first according to r,p,h, afterwards translated
     by x,y,z. */
    void transform(const vec6f & sdof);

    /// returns the squared length.
    float sqrLength() const { return coord[X]*coord[X]+coord[Y]*coord[Y]+coord[Z]*coord[Z]; }
    /// returns absolute length.
    float length() const { return sqrt(sqrLength()); }
    /// returns cross product vector with vec3f v2.
    vec3f crossProduct(const vec3f & v2) const;
    /// returns squared distance to coordinate x|y|(z).
    float sqrDistTo(float x, float y, float z=0.0f) const
    { return (coord[X]-x)*(coord[X]-x)+(coord[Y]-y)*(coord[Y]-y)+(coord[Z]-z)*(coord[Z]-z); }
    /// returns squared distance to vertex v.
    float sqrDistTo(const vec3f & v) const { return sqrDistTo(v[X],v[Y],v[Z]); }
    /// returns distance to vertex v
    float distTo(const vec3f & v) const { return sqrt(sqrDistTo(v)); }
    /// returns distance to vertex coordinate x|y|(z).
    float distTo(float x, float y, float z=0.0f) const { return sqrt(sqrDistTo(x,y,z)); }
    /// returns angle in xy plane to vertex v
    float angleToXY(const vec3f & v) const { return angleXY(coord[X],coord[Y],v[X],v[Y]); }
    /// returns angle in xy plane to vertex x|y
    float angleToXY(float x, float y) const { return angleXY(coord[X],coord[Y],x,y); }
    /// returns angle to vector v in space
    float angleTo(vec3f v) const;
    /// test for linear dependency with vector v
    bool linearDependent(const vec3f & v, float tolerance = EPSILONF) const;
    /// sets vec3f to the specified polar coordinates
    void polar(float angleXYpl, float angleZ=0.0f, float r=1.0f) {
        set(static_cast<float>(dcos(angleXYpl)) * static_cast<float>(dcos(angleZ)) * r,
          static_cast<float>(dsin(angleXYpl)) * static_cast<float>(dcos(angleZ)) * r,
          static_cast<float>(dsin(angleZ)) * r); }
	/// returns this vec3f in polar coordinates (heading,pitch,length)
	vec3f polar() const {
		return vec3f(angleXY(coord[X],coord[Y]), angleXY(sqrt(coord[X]*coord[X]+coord[Y]*coord[Y]),coord[Z]), length()); }
    /// operator for output in streams
    friend std::ostream & operator<<(std::ostream & os, const vec3f & v);
protected:
    /// stores coordinate values.
    float coord[3];
};

/// operator for output of vec3f objects in streams
std::ostream & operator<<(std::ostream & os, const vec3f & v);


//--- class vec4f ----------------------------------------------

/// a class for 4D vector geometry operations.
/** The vec4f class is intentionally free of virtual methods and takes exactly
 sixteen bytes of memory. This allows for using a pointer to a vec4f array instead
 of using pointers to floats, e.g., for OpenGL vertex arrays.*/
class vec4f {
public:
    /// default constructor
    vec4f(float x=0.0f, float y=0.0f, float z=0.0f, float w=1.0f) {
        coord[X]=x; coord[Y]=y; coord[Z]=z; coord[W]=w; };
    /// copy constructor
    vec4f(const vec4f & v) { coord[X]=v[X]; coord[Y]=v[Y];
                             coord[Z]=v[Z]; coord[W]=v[W]; };
    /// constructor from a float pointer
    vec4f(const float * f) { coord[X]=f[X]; coord[Y]=f[Y];
        	             coord[Z]=f[Z]; coord[W]=f[W]; };
    /// copy operator
    const vec4f & operator=(const vec4f &source);
    /// sets vector ordinates to new values (x|y|z|w).
    void set(float x, float y, float z, float w) {
        coord[X]=x; coord[Y]=y; coord[Z]=z; coord[W]=w; };

    /// comparison operator equality
    bool operator==(const vec4f &vt) const;
    /// comparison operator inequality
    bool operator!=(const vec4f & vt) const { return !((*this)==vt); };
    /// returns reference to single ordinate
    float & operator[](unsigned int i) { return coord[i%4]; };
    /// returns single ordinate value
    const float & operator[](unsigned int i) const { return coord[i%4]; };
    /// returns coordinate array
    const float * coords() const { return coord; };

    /// translates object by dx,dy,dz,dw
    void translate(float dx, float dy, float dz, float dw) {
        coord[X]+=dx; coord[Y]+=dy; coord[Z]+=dz; coord[W]+=dw; };
    /// translates object by vector v, optionally scaled by factor n
    void translate(const vec4f & v, float n=1.0f) {
        translate(v[X]*n, v[Y]*n, v[Z]*n, v[W]*n); };
    /// += operator. Sums correspondent ordinates. Identical to translate(v).
    void operator+=(const vec4f & v) {
        coord[X]+=v[X]; coord[Y]+=v[Y]; coord[Z]+=v[Z]; coord[W]+=v[W]; };
    /// scales object by sx,sy,sz,sw
    void scale(float sx, float sy, float sz, float sw) {
        coord[X]*=sx; coord[Y]*=sy; coord[Z]*=sz; coord[W]*=sw; };
    /// scales this vector by f.
    const vec4f & operator*=(float f) { coord[X]*=f; coord[Y]*=f;
        coord[Z]*=f; coord[W]*=f; return *this; };
    /// divides this vector by f.
    const vec4f & operator/=(float f) { coord[X]/=f; coord[Y]/=f;
        coord[Z]/=f; coord[W]/=f; return *this; };
    /// scales this vector to unit vector of length 1.0.
    const vec4f & normalize() { return *this*=(1.0f/length()); };
    /// returns the squared length.
    float sqrLength() const { return coord[X]*coord[X] + coord[Y]*coord[Y] + coord[Z]*coord[Z] + coord[W]*coord[W]; };
    /// returns absolute length.
    float length() const { return sqrt(sqrLength()); };
    /// computes scalar product between this vector and vec4f v.
    float scalarProduct(const vec4f & v) const {
        return v.coord[X]*coord[X] + v.coord[Y]*coord[Y] + v.coord[Z]*coord[Z] + v.coord[W]*coord[W]; };

    /// operator for output in streams
    friend std::ostream & operator<<(std::ostream & os, const vec4f & v);
protected:
    /// stores coordinate values.
    float coord[4];
};

/// operator for output of vec4f objects in streams
std::ostream & operator<<(std::ostream & os, const vec4f & v);
/// operator multiplying a vec4f v with a scalar f
inline const vec4f operator*(const vec4f & v, float f) {
    return vec4f(v[X]*f,v[Y]*f,v[Z]*f,v[W]*f); };

//--- class vec4d ----------------------------------------------

/// a class for 4D vector geometry operations, double precision
class vec4d {
public:
    /// default constructor
    vec4d(double x=0.0, double y=0.0, double z=0.0, double w=0.0) {
        coord[X]=x; coord[Y]=y; coord[Z]=z; coord[W]=w; };
    /// copy constructor
    vec4d(const vec4d & v) { coord[X]=v[X]; coord[Y]=v[Y];
                             coord[Z]=v[Z]; coord[W]=v[W]; };
    /// copy constructor from a vec4f
    vec4d(const vec4f & v) { coord[X]=v[X]; coord[Y]=v[Y];
                             coord[Z]=v[Z]; coord[W]=v[W]; };
    /// copy operator
    const vec4d & operator=(const vec4d &source);

    /// comparison operator equality
    bool operator==(const vec4d &vt) const;
    /// comparison operator inequality
    bool operator!=(const vec4d & vt) const { return !((*this)==vt); };
    /// returns reference to single ordinate
    double & operator[](unsigned int i) { return coord[i%4]; };
    /// returns single ordinate value
    const double & operator[](unsigned int i) const { return coord[i%4]; };

    /// += operator. Sums correspondent ordinates. Identical to translate(v).
    void operator+=(const vec4d & v) {
        coord[X]+=v[X]; coord[Y]+=v[Y]; coord[Z]+=v[Z]; coord[W]+=v[W]; };
    /// scales this vector by f.
    const vec4d & operator*=(double f) { coord[X]*=f; coord[Y]*=f;
        coord[Z]*=f; coord[W]*=f; return *this; };
    /// divides this vector by f.
    const vec4d & operator/=(double f) { coord[X]/=f; coord[Y]/=f;
        coord[Z]/=f; coord[W]/=f; return *this; };
    /// scales this vector to unit vector of length 1.0.
    const vec4d & normalize() { return *this*=(1.0/length()); };
    /// returns the squared length.
    double sqrLength() const { return coord[X]*coord[X] + coord[Y]*coord[Y] + coord[Z]*coord[Z] + coord[W]*coord[W]; };
    /// returns absolute length.
    double length() const { return sqrt(sqrLength()); };
    /// computes scalar product between this vector and vec4d v.
    double scalarProduct(const vec4d & v) const {
        return v.coord[X]*coord[X] + v.coord[Y]*coord[Y] + v.coord[Z]*coord[Z] + v.coord[W]*coord[W]; };
protected:
    /// stores coordinate values.
    double coord[4];
};

/// operator multiplying a vec4f v with a scalar f
inline const vec4d operator*(const vec4d & v, double f) {
    return vec4d(v[X]*f,v[Y]*f,v[Z]*f,v[W]*f); };


//--- class vec6f --------------------------------------------------

/// a class representing a six degree of freedom coordinate.
/** Note that unfortunately there is no generally accepted standard for
 sixdofs rotations. Since the veLib normally defines forward along the +Y
 axis and upward along +Z axis, roll means a rotation around the Y axis, pitch
 around the X axis, and heading (i.e. yaw) around the Z axis. Euler rotations
 shall be preformed in exactly this order. */
class vec6f : public vec3f {
public:
    /// default constructor
    vec6f(float x=0, float y=0, float z=0, float h=0, float p=0, float r=0) : vec3f(x,y,z) {
        ang[0]=h; ang[1]=p; ang[2]=r; }
    /// copy constructor
    vec6f(const vec6f & source) : vec3f(source) {
        ang[0]=source.ang[0]; ang[1]=source.ang[1]; ang[2]=source.ang[2]; }
    /// constructor from a vec3f
    vec6f(const vec3f & source) : vec3f(source) {
        ang[0]=ang[1]=ang[2]=0; }
    /// constructor from a transformation matrix
    vec6f(const mat4f & m) : vec3f() { set(m); }
    /// copy operator
    vec6f & operator=(const vec6f & source);
    /// += operator. Sums correspondend ordinates.
    void operator+=(const vec6f & summand);
    /// comparison operator equality
    bool operator==(const vec6f & sd) const;
    /// comparison operator inequality
    bool operator!=(const vec6f & sd) const { return !((*this)==sd); };
    /// translates object by dx,dy
    void translate(float dx, float dy) { coord[X]+=dx; coord[Y]+=dy; };
    /// translates object by dx,dy,dz
    void translate(float dx, float dy, float dz) {
        coord[X]+=dx; coord[Y]+=dy; coord[Z]+=dz; };
    /// translates the vec6f in the direction of v (basically the same as in class vec3f). It also rotates with respective scaling factor.
    void translate(const vec6f & v, float linearscale=1.0f, float angularscale=1.0f);
    /// sets all ordinates
    void set(float x=0, float y=0, float z=0, float h=0, float p=0, float r=0);
    /// sets sixdof as far as possible to an equivalent transformation as in matrix m
    void set(const mat4f & m);
    /// copies values into a float pointer
    void get(float * f) const;
    /// returns single ordinate reference
    float & operator[](unsigned int i) {
        return (i%6<3) ? coord[i%6] : ang[i%6-3]; }
    /// returns single ordinate value, const
    float operator[](unsigned int i) const {
        return (i%6<3) ? coord[i%6] : ang[i%6-3]; }
    /// addition operator vec6f+vec6f
    inline const vec6f operator+(const vec6f & v) const {
        return vec6f(coord[X]+v[X],coord[Y]+v[Y],coord[Z]+v[Z],
            coord[H]+v[H],coord[P]+v[P],coord[R]+v[R]); }
    /// addition operator vec6f+vec3f
    inline const vec6f operator+(const vec3f & v) const {
        return vec6f(coord[X]+v[X],coord[Y]+v[Y],coord[Z]+v[Z],
            coord[H],coord[P],coord[R]); }
    /// subtraction operator
    inline const vec6f operator-(const vec6f & v) const {
        return vec6f(coord[X]-v[X],coord[Y]-v[Y],coord[Z]-v[Z],
            coord[H]-v[H],coord[P]-v[P],coord[R]-v[R]); }
    /// product operator. All ordinates of the return value are scaled by f.
    inline vec6f operator*(float f) const {
        return vec6f(coord[X]*f,coord[Y]*f,coord[Z]*f,coord[H]*f,coord[P]*f,coord[R]*f); }

    /// sets all ordinates to 0.0
    void reset() { coord[X]=coord[Y]=coord[Z]=ang[0]=ang[1]=ang[2]=0.0f; };
    /// returns vec6f size (number of ordinates), mainly for "for" statements
    static unsigned int size() { return 6; };

    /// operator for output in streams
    friend std::ostream & operator<<(std::ostream & os, const vec6f & sdof);
protected:
    /// stores angle ordinates
    float ang[3];
};


/// operator for output of sixdofs in streams
std::ostream & operator<<(std::ostream & os, const vec6f & sdof);

/// addition operator vec3f+vec6f
inline const vec6f operator+(const vec3f &v1, const vec6f &v2) {
    return vec6f(v1[X]+v2[X],v1[Y]+v2[Y],v1[Z]+v2[Z], v2[H],v2[P],v2[R]); }

//--- class sphere ---------------------------------------------

/// a class representing a sphere.
class sphere : public vec3f {
public:
    /// default constructor
    sphere(float x=0, float y=0, float z=0, float rd=0) : vec3f(x,y,z), r(rd) { }
    /// constructor from a vec
    sphere(const vec3f & cnt, float rd=0) : vec3f(cnt), r(rd) { }
    /// copy constructor
    sphere(const sphere & source) : vec3f(source), r(source.r) { }
    /// sets data
    void set(float x_, float y_, float z_, float r_) { vec3f::set(x_,y_,z_); r=r_; }
    /// returns center
    vec3f & center() { return *this; }
    /// returns center, const
    const vec3f & center() const { return *this; }
    /// returns radius
    float radius() const { return r; }
    /// sets radius
    void radius(float newRadius) { r=newRadius; }
protected:
    /// stores radius
    float r;
};

/// operator for output of sphere objects in streams
std::ostream & operator<<(std::ostream & os, const sphere & s);

//--- line class ---------------------------------------------------
/// a class for line mathematics.
/** Depending on the method, the line is treated as a
 -# line segment (limited by the 2 control vertices): distTo, length, squaredLength, intersect2d, intersects(isRay=false)
 -# ray with pt0 as origin and pt0pt1 as direction: intersects(isRay=true)
 -# infinite line.
 */
class line {
public:
    /// default constructor
    line() : pt1(1.0f,0.0f,0.0f) { }
    /// 3d constructor
    line(float x1, float y1, float z1, float x2, float y2, float z2) : pt0(x1,y1,z1), pt1(x2,y2,z2) { }
    ///  2d constructor
    line(float x1, float y1, float x2, float y2) : pt0(x1,y1), pt1(x2,y2) { }
    ///constructor taking vertices as argument
    line(const vec3f & p1, const vec3f & p2) : pt0(p1), pt1(p2) { }
    /// copy constructor
    line(const line& source) : pt0(source.pt0), pt1(source.pt1) { }
    /// copy operator
    const line & operator=(const line& source);

    /// sets the coordinates of the two control vertices
    void set(float x1, float y1, float z1, float x2, float y2, float z2) {
        pt0.set(x1,y1,z1); pt1.set(x2,y2,z2); }
    /// sets the two control vertices to p1 & p2
    void set(const vec3f & p1, const vec3f & p2) { pt0=p1; pt1=p2; }
    /// translates object by dx,dy,dz
    void translate(float dx, float dy, float dz=0) {
        pt0.translate(dx,dy,dz); pt1.translate(dx,dy,dz); }
    /// translates object by vector v, optionally scaled by factor n
    void translate(const vec3f & v, float n=1.0f) { translate(v[X]*n,v[Y]*n,v[Z]*n); };
    /// rotates object around arbitrary axis from origin to p by angle.
    void rotate(float angle, float x, float y, float z) {
        vec3f::rotate(&pt0[X],angle,x,y,z,2); };
    /// scales object by sx,sy and optionally sz
    void scale(float sx, float sy, float sz=1.0f) {
        pt0.scale(sx,sy,sz); pt1.scale(sx,sy,sz); }
    /// transforms object by matrix m
    void transform(const mat4f & m);

    /// returns control vec3f n
    vec3f & operator[](unsigned int n) { return n ? pt1 : pt0; }
    /// returns control vec3f n, const
    const vec3f & operator[](unsigned int n) const { return n ? pt1 : pt0; }
    /// returns distance to vertex p
    float distTo(const vec3f & p) const;
    /// returns length
    float length() const { return sqrt( (pt0[X]-pt1[X])*(pt0[X]-pt1[X]) + (pt0[Y]-pt1[Y])*(pt0[Y]-pt1[Y])+(pt0[Z]-pt1[Z])*(pt0[Z]-pt1[Z])); }
    /// returns the squared length.
    /** This method is provided mainly for efficiency reasons for avoiding square root computations. */
    float sqrLength() const { return (pt0[X]-pt1[X])*(pt0[X]-pt1[X]) + (pt0[Y]-pt1[Y])*(pt0[Y]-pt1[Y])+(pt0[Z]-pt1[Z])*(pt0[Z]-pt1[Z]); }
	/// calculates and returns normalized direction vector
	vec3f direction() const { vec3f dir(pt0,pt1); dir.normalize(); return dir; }
    /// tests for intersection, projected in xy plane.
    /** \param l the line segment to be tested
     \return distance to intersection point or -1 if no intersection has been found. */
    float intersect2d(const line &l) const;

    /// calculates intersection with triangle (tr0|tr1|tr2).
    /**
     \param tr0 is a reference to the first vertex of the triangle that is tested
     \param tr1 is a reference to the second vertex of the triangle that is tested
     \param tr2 is a reference to the third vertex of the triangle that is tested
     \param isRay (optional) defines whether the line is
     treated as as infinite ray starting from pt[0].
     \return NULL or intersection point. The memory of this vec
     has to be deallocated by the user! */
    vec3f * intersection(const vec3f & tr0, const vec3f & tr1,
                         const vec3f & tr2, bool isRay=true ) const;
    /// tests for intersection between a line (segment) and triangle (tr0|tr1|tr2).
    /** This method is much faster than the corresponding intersection() method,
     because it does not compute the exact intersection.
     \param tr0 is a reference to the first vertex of the triangle that is tested
     \param tr1 is a reference to the second vertex of the triangle that is tested
     \param tr2 is a reference to the third vertex of the triangle that is tested
     \param isRay (optional) defines whether the line is
     treated as as infinite ray starting from pt[0].
     \return false or true */
    bool intersects(const vec3f & tr0, const vec3f & tr1,
                    const vec3f & tr2, bool isRay=true ) const;
    /// calculates intersection with sphere sph.
    /**
     \param sph is a reference to the sphere that is tested
     \param isRay (optional) defines whether the line is
     treated as as infinite ray starting from pt[0].
     \return NULL or intersection point. The memory of this vec
     has to be deallocated by the user! */
    vec3f * intersection(const sphere & sph, bool isRay=true ) const {
        return intersection(sph,sph.radius(),isRay); }
    /// calculates intersection with sphere (center|radius).
    /**
     \param center is a reference to the center of the sphere that is tested
     \param radius defines the sphere's radius
     \param isRay (optional) defines whether the line is
     treated as as infinite ray starting from pt[0].
     \return NULL or intersection point. The memory of this vec
     has to be deallocated by the user! */
    vec3f * intersection(const vec3f & center, float radius, bool isRay=true ) const;
    /// tests for intersection with sphere sph.
    /**
     \param sph is a reference to the sphere that is tested
     \param isRay (optional) defines whether the line is
     treated as as infinite ray starting from pt[0].
     \return true in case of intersection */
    bool intersects(const sphere & sph, bool isRay=true ) const {
        return intersects(sph,sph.radius(),isRay); }
    /// tests for intersection with sphere (center|radius).
    /**
     \param center is a reference to the center of the sphere that is tested
     \param radius defines the sphere's radius
     \param isRay (optional) defines whether the line is
     treated as as infinite ray starting from pt[0].
     \return true in case of intersection */
    bool intersects(const vec3f & center, float radius, bool isRay=true) const;
        
    /// calculates intersection with plane pl.
    /**
     \param pl is a reference to the plane that is tested
     \param isRay (optional) defines whether the line is
     treated as as infinite ray starting from pt[0].
     \return NULL or intersection point. The memory of this vec
     has to be deallocated by the user! */
    inline vec3f * intersection(const plane & pl, bool isRay=true ) const {
		return ::intersection(pt0, vec3f(pt0,pt1), pl, isRay); }
    /// tests for intersection between a line (segment) and a plane.
    /** This method is much faster than the corresponding intersection() method,
     because it does not compute the exact intersection.
     \param pl is a reference to the plane that is tested
     \param isRay (optional) defines whether the line is
     treated as as infinite ray starting from pt[0].
     \return false or true */
    bool intersects(const plane & pl, bool isRay=true ) const;


    /// operator for output in streams
    friend std::ostream & operator<<(std::ostream & os, const line & l);
protected:
    /// vertices
    vec3f pt0, pt1;
};

/// operator for output of lines in streams
std::ostream & operator<<(std::ostream & os, const line & l);


//--- class triangle -----------------------------------------------

/// a class for triangle geometry
class triangle {
public:
    /// 2d constructor
    triangle(float x1,float y1, float x2,float y2, float x3,float y3) { set(x1,y1,0, x2,y2,0, x3,y3,0); };
    /// 3d constructor
    triangle(float x1,float y1,float z1, float x2,float y2,float z2,  float x3,float y3,float z3) {
        set(x1,y1,z1, x2,y2,z2, x3,y3,z3); };
    /// constructor taking vec3f references
    triangle(const vec3f & p0, const vec3f & p1, const vec3f & p2) { set(p0, p1, p2); };
    /// low level constructor taking binary data of 9 float values, no range checks, beware of segfaults!
    triangle(const float * pCoords);
    /// copy constructor
    triangle(const triangle & source);
    /// sets triangle to new 2D vec3f values
    void set(float x1,float y1, float x2,float y2, float x3,float y3) {
        set(x1,y1,0, x2,y2,0, x3,y3,0); };
    /// sets triangle to new 3D vec3f values
    void set(float x1,float y1,float z1, float x2,float y2,float z2,  float x3,float y3,float z3);
    /// sets triangle to new vec3f values
    void set(const vec3f & v0, const vec3f & v1, const vec3f & v2){
        pt[0]=v0; pt[1]=v1; pt[2]=v2; };
    /// translates object by dx,dy,dz
    void translate(float dx, float dy, float dz=0) {
        vec3f::translate(&pt[0][X],dx,dy,dz,3); };
    /// translates object by vector v, optionally scaled by factor n
    void translate(const vec3f & v, float n=1.0f) { translate(v[X]*n,v[Y]*n,v[Z]*n); };
    /// rotates object around arbitrary axis from origin to p by angle.
    void rotate(float angle, float x, float y, float z) {
        vec3f::rotate(&pt[0][X],angle,x,y,z,3); };
    /// scales object by sx,sy and optionally sz
    void scale(float sx, float sy, float sz=1);
    /// transforms this triangle by multiplying it with matrix m
    void transform(const mat4f & m);
    /// transforms this triangle by applying the provided sixdof transformation.
    /** The vector is rotated first according to r,p,h, afterwards translated
     by x,y,z. */
    void transform(const vec6f & sdof);
    /// returns control point n
    vec3f & operator[](unsigned int n) { return pt[n%3]; };
    /// returns control point n, const
    const vec3f & operator[](unsigned int n) const { return pt[n%3]; };
    /// fills coords with coordinate values of all 3 control points, no memory allocation.
    void getCoords(double * coords) const;
    /// tests v for being in the same xy area than the triangle
    bool isElemXY(const vec3f & p) const;
    /// returns the z distance from v to the triangle plane, positive if v is above, otherwise negative
    float distZ(const vec3f & p) const;
    /// returns normal vector of the triangle plane
    vec3f normal() const;
    /// returns triangle center
    inline vec3f center() const {
        return vec3f((pt[0][X]+pt[1][X]+pt[2][X])/3.0f,(pt[0][Y]+pt[1][Y]+pt[2][Y])/3.0f,(pt[0][Z]+pt[1][Z]+pt[2][Z])/3.0f); }
    /// returns area of triangle.
    double area() const;
    /// returns plane equation factors A,B,C,D.
    void getABCD(float &a, float &b, float &c, float &d) const;

    /// operator for output in streams
    friend std::ostream & operator<<(std::ostream & os, const triangle & t);
protected:
    /// vertices
    vec3f pt[3];
};

/// operator for output of triangles in streams
std::ostream & operator<<(std::ostream & os, const triangle & t);


//--- class plane ----------------------------------------------

/// a class representing a plane
class plane {
public:
    /// default constructor
    plane() : n(0.0f,0.0f,1.0f), d(0.0f) { }
    /// constructor taking a normal vector's coordinates and an optional distance component
    plane(float x, float y, float z, float distance=0.0f)  : n(x,y,z), d(distance) { n.normalize(); }
    /// constructor from a normal vector and optional distance component
    plane(const vec3f & normal, float distance=0.0f) : n(normal), d(distance) { n.normalize(); }
    /// constructor from a normal vector and a point p.
    plane(const vec3f & normal, const vec3f & p);
    /// copy constructor
    plane(const plane & source) : n(source.n), d(source.d) { }
    /// copy operator
    const plane & operator=(const plane & source) { n=source.n; d=source.d; return *this; }
    /// comparison operator equality
    bool operator==(const plane & pl) const;
	/// comparison operator less than, mainly for sorting purposes
	bool operator<(const plane & pl) const;
    /// returns signed distance to point p
    float signedDistTo(const vec3f & p) const { return (n*p)+d; }
    /// returns absolute distance to point p
    float distTo(const vec3f & p) const { return fabs(signedDistTo(p)); }
    /// returns normal vector
    vec3f & normal() { return n; }
    /// returns normal vector, const
    const vec3f & normal() const { return n; }
    /// returns offset component d, const
    float D() const { return d; }
    /// returns offset component d
    float & D() { return d; }
    /// returns plane's pivot, i.e. the point on plane next to the origin
    vec3f pivot() const { return vec3f(-n[0]*d,-n[1]*d,-n[2]*d); }
    /// translates object by x|y|z.
    void translate(float x, float y, float z=0.0f) { translate(vec3f(x,y,z)); }
    /// translates object by vector v.
    void translate(const vec3f & v);
    /// rotates object around arbitrary axis from origin to p by angle.
    void rotate(float angle, vec3f p) { n.rotate(angle,p); }
    /// rotates object according to heading, pitch, and roll.
    void rotate(float h, float p, float r)  { n.rotate(h,p,r); }
    /// transforms this plane by applying the provided sixdof transformation.
    /** The plane is rotated first according to r,p,h, afterwards translated
     by x,y,z. */
    void transform(const vec6f & sdof);
    /// transforms this plane by multiplying it with matrix m
    void transform(const mat4f & m);
    /// unary negation operator returns a plane pointing in the opposite direction
    const plane operator-() const { return plane(-n,-d); }
    /// inverts plane
    void invert() { n=-n; d=-d; }
    /// computes intersection point between 3 planes
    vec3f * intersection(const plane & pl2, const plane & pl3) const;
protected:
    /// stores normal vector
    vec3f n;
    /// stores offset component d of plane equation.
    float d;
};

/// operator for output of plane objects in streams
std::ostream & operator<<(std::ostream & os, const plane & pl);

//--- class mat4f --------------------------------------------------

/// a class for typical 3D geometry 4x4 matrix operations.
/** The matrix class is intentionally free of virtual methods and takes exactly
 64 bytes memory. This allows for using a pointer to a matrix identically
 to using float pointers, e.g., for OpenGL. Therefore also the order of tranformations
 (e.g., rotations, scale) correspond exactly to OpenGL's factor order.\n
 Also the order of its members is equivalent to OpenGL:\n
 \verbatim
 mat4f M = [ m0  m4 m8  m12
             m1  m5 m9  m13
             m2  m6 m10 m14
             m3  m7 m11 m15 ]
 \endverbatim
 Note that this order has changed between veLib v1.0 and veLib 1.0.1!
 The normal user should not be affected, as long as matrices have not been used
 directly. However, the switch of the order could also be the origin of subtle
 bugs. In order to make the user explicitly aware of this change, the class has
 been renamed from matrix4f to mat4f (which also is equivalent to GLSL).
 */
class mat4f {
public:
    /// default constructor, creating identity matrix
    mat4f() { identity(); }
    /// constructor from direct values analogous to the set function
    mat4f(float m0, float m1, float m2, float m3,
        float m4, float m5, float m6, float m7,
        float m8, float m9, float m10,float m11,
        float m12,float m13,float m14,float m15);
    /// constructor from sixdof
    mat4f(const vec6f & sdof) { set(sdof); }
    /// copy constructor, bitwise copy.
    mat4f(const mat4f & source);
    /// copy operator, bitwise copy.
    const mat4f & operator=(const mat4f & source);
    /// returns reference of single ordinate
    float & operator[](unsigned int i) { return m[i%16]; }
    /// returns single ordinate value
    const float & operator[](unsigned int i) const { return m[i%16]; }
    /// returns column vector i
    const vec4f col(unsigned int i) const {
        return vec4f(&m[4*(i%4)]); }
    /// returns row vector i
    const vec4f row(unsigned int i) const {
        return vec4f(m[i%4], m[4+(i%4)], m[8+(i%4)], m[12+(i%4)]); }

    /// transforms this matrix to an identity matrix
    void identity();
    /// returns true if this matrix is an identity matrix
    bool isIdentity() const;
    /// returns true if this matrix is (almost) an identity matrix
    bool isIdentity(float tolerance) const;
    /// makes this matrix to a NaN matrix
    /** The NaN matrix is used to indicate that no mathematical solution could
     be found in an operation. To test for NaN, use the mat4f::isNan() method. */
    void nan();
    /// returns true if this matrix is a NaN matrix
    bool isNan() const;

    /// transposes this matrix
    void transpose();
    /// transforms this matrix into its inverse if possible, otherwise into a NaN matrix
    void invert();
    /// returns the inverse matrix if possible, otherwise a NaN matrix
    mat4f inverse() const;
    /// sets this matrix to the 16 provided values
    /** the values are assumed to be in the OpenGL order. */
    void set(float m0, float m1, float m2, float m3,
             float m4, float m5, float m6, float m7,
             float m8, float m9, float m10,float m11,
             float m12,float m13,float m14,float m15);
    /// sets this matrix to the values provided in a float array
    /** the values are assumed to be in the OpenGL order. */
    void set(float *f);
    /// sets this matrix to transformation stored in a sixdof
    void set(const vec6f & sdof);
    /// operator multiplying two matrixes in the order (*this) * m
    const mat4f operator*(const mat4f & m) const;
    /// multiplies this matrix by m2, (*this) = (*this) * m2
    void operator*=(const mat4f & m2) { (*this)= (*this)*m2; };
	/// multiplies a vector by this matrix;
	vec3f operator*(vec3f& vV)const;
	/// adds the translation x|y|z to the transformation
    void translate(float x, float y, float z=0.0f) { translate(vec3f(x,y,z)); };
    /// adds the translation v to the transformation
    void translate(const vec3f & v);
    /// creates a rotation matrix around the x axis
    void rotateX(float angle);
    /// creates a rotation matrix around the y axis
    void rotateY(float angle);
    /// creates a rotation matrix around the z axis
    void rotateZ(float angle);
    /// multiplies matrix with a rotation matrix around arbitrary axis from origin to p by angle, (*this) = (*this) * mRot
    void rotate(float angle, vec3f p);
    /// multiplies matrix with a rotation matrix around arbitrary axis from origin to (x|y|z) by angle, (*this) = (*this) * mRot
    void rotate(float angle, float ax, float ay, float az) { rotate(angle,vec3f(ax,ay,az)); }
    /// multiplies matrix with a scale matrix, (*this) = (*this) * mScale
    void scale(float sx, float sy, float sz);
    /// multiplies matrix with a scale matrix, (*this) = (*this) * mScale
    void scale(const vec3f & sc) { scale(sc[X],sc[Y],sc[Z]); }
    /// transforms a vector of vec3f by applying this matrix
    void transform(std::vector<vec3f> & vV) const;
    /// transforms a vector of float coordinates n*(x|y|z) by applying this matrix
    void transform(std::vector<float> & vF) const;
    /// transforms an array of coherent float coordinates n*(x|y|z) starting at pF by applying this matrix
    void transform(float * pF, unsigned int n) const;
protected:
    /// stores values.
    float m[16];
};

/// operator for output in streams
std::ostream & operator<<(std::ostream & os, const mat4f & m);


//--- class frustum --------------------------------------------

/// class for frustum (clipping) operations.
class frustum {
public:
    /// default constructor
    frustum();
    /// constructor
    frustum(float clipLeft,float clipRight,float clipBottom,
            float clipTop,float clipNear,float clipFar);
    /// copy constructor
    frustum(const frustum & source);
    /// copy operator
    const frustum & operator=(const frustum & source);
    /// sets frustum geometry
    void set(float clipLeft,float clipRight,float clipBottom,
            float clipTop,float clipNear,float clipFar);
    /// sets frustum geometry by a vec6f and resets transformation
    void set(const vec6f & dims) { set(dims[0],dims[1],dims[2],dims[3],dims[4],dims[5]); }
    /// tests whether sphere sph at least partially intersects this frustum.
    bool intersects(const sphere & sph) const;
    /// tests whether an axis-aligned box, defined by its min and max coordinates, at least partially intersects this frustum.
    bool intersects(const vec3f & vecMin, const vec3f & vecMax ) const;
    /// translates object by x|y|z.
    void translate(float x, float y, float z=0.0f) { translate(vec3f(x,y,z)); };
    /// translates object by vector v.
    void translate(const vec3f & v);
    /// rotates object around arbitrary axis from origin to p by angle.
    void rotate(float angle, const vec3f & p);
    /// rotates object according to heading, pitch, and roll.
    void rotate(float h, float p, float r);
    /// transforms this frustum by applying the provided sixdof transformation.
    /** The vector is rotated first according to r,p,h, afterwards translated
     by x,y,z. */
    void transform(const vec6f & sdof);
protected:
    /// stores clipping planes
    plane pl[6];
};

#endif        // _PRO_MATH_H

