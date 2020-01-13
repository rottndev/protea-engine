// mathematical auxilliary functions (c) 1999 by Gerald Franz www.viremo.de

#include <sys/timeb.h>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <vector>

#include "proMath.h"

using namespace std;

//--- functions and templates --------------------------------------

double NaN() {
#ifdef _MSC_VER
	const unsigned long nan[2]={0xffffffff, 0x7fffffff};
	return *( double* )nan;
#else
	return 0.0/0.0;
#endif
}

float angleXY(float x, float y) {
    if(!x) return (y>0.0f) ? 90.0f : 270.0f;
    return (x<0.0f) ? (atan(y/x)/PI_180)+180.0f : (y<0.0f) ? (atan(y/x)/PI_180)+360.0f : (atan(y/x)/PI_180);
}

float angleXY(float x1, float y1, float x2, float y2) {
    if(x2==x1) return (y2>y1) ? 90.0f : 270.0f;
    if(x2<x1) return (atan((y2-y1)/(x2-x1))/PI_180)+180.0f;
    if(y2<y1) return (atan((y2-y1)/(x2-x1))/PI_180)+360.0f;
    return (atan((y2-y1)/(x2-x1))/PI_180);
}

float angleXY(float x1, float y1, float x2, float y2, float x3, float y3) {
    float ret=angleXY(x1,y1,x3,y3)-angleXY(x1,y1,x2,y2);
    if(ret<0) ret+=360.0f;
    return ret;
}

float angleXY(const vec3f & p0, const vec3f & p1, const vec3f & p2) {
    float ret= angleXY(p0[X],p0[Y],p2[X],p2[Y])-angleXY(p0[X],p0[Y],p1[X],p1[Y]);
    if(ret<0) ret+=360.0f;
    return ret;
}

float distPointSeg(float x1, float y1, float x2, float y2, float x3, float y3) {
    if(x1==x2) {
        if(((y3>=y1)&&(y3<=y2))||((y3<=y1)&&(y3>=y2)))
            return fabs(x1-x3);
        else return minAbs(dist(x1,y1,x3,y3),dist(x2,y2,x3,y3));
    }
    else {
        float m=(y2-y1)/(x2-x1); // Gerade P1P2 y=mx+b
        float b=y1-m*x1;
        float xs;
        if(m!=0.0f) {
            //float c=y3+x3/m;  // Senkrechte durch P3 y=(-1/m)x+c, inline
            xs=((y3+x3/m)-b)/(m+1/m);//Schnittpunkt Gerade-Senkrechte S(xs;ys)
        }
        else xs=x3;
        float ys=m*xs+b;

        if(((xs>=x1)&&(xs<=x2))||((xs<=x1)&&(xs>=x2)))
            return dist(x3, y3, xs, ys);
        else return minAbs(dist(x1,y1,x3,y3),dist(x2,y2,x3,y3));
    }
}

vec3f * intersection(const vec3f & origin, const vec3f & dir, const plane & pl, bool isRay) {
    // is dir parallel to tr?:
    float dot=dir*pl.normal();
    if (dot > -EPSILONF && dot < EPSILONF) return 0; // line and plane are parallel
    // compute intersection distance from pt0:
    float dist = -(pl.normal()*origin+pl.D())/dot;
    if((dist < 0.0f)||(!isRay&&(dist>1.0f))) return 0; // is ray direction wrong or distance longer than line?
    // compute intersection vertex:
    return new vec3f(origin+(dir*dist));
}

//--- class vec2f --------------------------------------------------

const vec2f & vec2f::operator=(const vec2f & source) {
    coord[X]=source[X];
    coord[Y]=source[Y];
    return *this;
}

bool vec2f::operator==(const vec2f &vt) const {
    if(fabs(coord[X]-vt[X])>EPSILONF) return false;
    if(fabs(coord[Y]-vt[Y])>EPSILONF) return false;
    return true;
}

void vec2f::translate(float * x, float dx, float dy, size_t n) {
    for(float *p=x; p<x+(2*n); p+=2) {
        p[X]+=dx;
        p[Y]+=dy;
    }
}


std::ostream & operator<<(std::ostream & os, const vec2f & v) {
    return os << v[X] << ' ' << v[Y];
}



//--- class vec3f --------------------------------------------------

vec3f::vec3f(const vec4f & v) { 
    coord[X]=v[X]; coord[Y]=v[Y]; coord[Z]=v[Z]; 
}

const vec3f & vec3f::operator=(const vec3f & source) {
    coord[X]=source[X];
    coord[Y]=source[Y];
    coord[Z]=source[Z];
    return *this;
}

bool vec3f::operator==(const vec3f &vt) const {
    if(fabs(coord[X]-vt[X])>EPSILONF) return false;
    if(fabs(coord[Y]-vt[Y])>EPSILONF) return false;
    if(fabs(coord[Z]-vt[Z])>EPSILONF) return false;
    return true;
}

bool vec3f::operator<(const vec3f &v) const {
    if(coord[X]!=v.coord[X]) return coord[X]<v.coord[X] ? true : false;
    if(coord[Y]!=v.coord[Y]) return coord[Y]<v.coord[Y] ? true : false;
    return coord[Z]<v.coord[Z] ? true : false;
}

void vec3f::translate(float * x, float dx, float dy, float dz, size_t n) {
    for(float *p=x; p<x+(3*n); p+=3) {
        p[X]+=dx;
        p[Y]+=dy;
        p[Z]+=dz;
    }
}

void vec3f::rotate(float & x, float & y, float & z, float angle, float ax, float ay, float az) {
    float c = static_cast<float>(dcos(angle));
    float s = static_cast<float>(dsin(angle));
    float newX=x*(c+ax*ax*(1-c))    + y*(ax*ay*(1-c)-az*s) + z*(ax*az*(1-c)+ay*s);
    float newY=x*(ax*ay*(1-c)+az*s) + y*(c+ay*ay*(1-c))    + z*(ay*az*(1-c)-ax*s);
    float newZ=x*(ax*az*(1-c)-ay*s) + y*(ay*az*(1-c)+ax*s) + z*(c+az*az*(1-c));
    x=newX;
    y=newY;
    z=newZ;
}

void vec3f::rotate(float * x, float angle, float ax, float ay, float az, unsigned int n) {
    float c = static_cast<float>(dcos(angle));
    float s = static_cast<float>(dsin(angle));
    float mat[]={ c+ax*ax*(1-c),    ax*ay*(1-c)-az*s, ax*az*(1-c)+ay*s,
    		  ax*ay*(1-c)+az*s, c+ay*ay*(1-c),    ay*az*(1-c)-ax*s,
                  ax*az*(1-c)-ay*s, ay*az*(1-c)+ax*s, c+az*az*(1-c) };

    for(float *i=x; i<x+3*n; i+=3) {
        float newX=*i*mat[0] + *(i+1)*mat[1] + *(i+2)*mat[2];
        float newY=*i*mat[3] + *(i+1)*mat[4] + *(i+2)*mat[5];
        float newZ=*i*mat[6] + *(i+1)*mat[7] + *(i+2)*mat[8];
        *i    =newX;
        *(i+1)=newY;
        *(i+2)=newZ;
    }
}

void vec3f::rotate(float angle, vec3f p) {
    p.normalize();
    rotate(coord[X],coord[Y],coord[Z],angle,p[X],p[Y],p[Z]);
}

void vec3f::rotate(float h, float p, float r) {
    float sH = static_cast<float>(dsin(h)); float cH = static_cast<float>(dcos(h));
    float sP = static_cast<float>(dsin(p)); float cP = static_cast<float>(dcos(p));
    float sR = static_cast<float>(dsin(r)); float cR = static_cast<float>(dcos(r));
    float x =coord[X]*(cH*cR - sH*sP*sR) + coord[Y]*-sH*cP + coord[Z]*(cH*sR + sH*sP*cR);
    float y =coord[X]*(sH*cR + cH*sP*sR) + coord[Y]* cH*cP + coord[Z]*(sH*sR - cH*sP*cR);
    coord[Z]=coord[X]*cP*-sR             + coord[Y]*sP     + coord[Z]*cP*cR;
    coord[X]=x;
    coord[Y]=y;
}

void vec3f::scale(float sx, float sy, float sz) {
    coord[X]*=sx;
    coord[Y]*=sy;
    coord[Z]*=sz;
}

void vec3f::transform(const mat4f & m) {
    float x =m[0]*coord[X] + m[4]*coord[Y] + m[8]*coord[Z] + m[12];
    float y =m[1]*coord[X] + m[5]*coord[Y] + m[9]*coord[Z] + m[13];
    coord[Z]=m[2]*coord[X] + m[6]*coord[Y] + m[10]*coord[Z]+ m[14];
    coord[X]=x;
    coord[Y]=y;
}

void vec3f::transform(const vec6f & sdof) {
    float sH = static_cast<float>(dsin(sdof[H])); float cH = static_cast<float>(dcos(sdof[H]));
    float sP = static_cast<float>(dsin(sdof[P])); float cP = static_cast<float>(dcos(sdof[P]));
    float sR = static_cast<float>(dsin(sdof[R])); float cR = static_cast<float>(dcos(sdof[R]));
    float x =coord[X]*(cH*cR - sH*sP*sR) + coord[Y]*-sH*cP + coord[Z]*(cH*sR + sH*sP*cR) + sdof[X];
    float y =coord[X]*(sH*cR + cH*sP*sR) + coord[Y]* cH*cP + coord[Z]*(sH*sR - cH*sP*cR) + sdof[Y];
    coord[Z]=coord[X]*cP*-sR             + coord[Y]*sP     + coord[Z]*cP*cR              + sdof[Z];
    coord[X]=x;
    coord[Y]=y;
}

vec3f vec3f::crossProduct(const vec3f & v2) const {
    float x=coord[Y]*v2[Z]-coord[Z]*v2[Y];
    float y=coord[Z]*v2[X]-coord[X]*v2[Z];
    float z=coord[X]*v2[Y]-coord[Y]*v2[X];
    return vec3f(x,y,z);
}


float vec3f::angleTo(vec3f v) const {
    vec3f v0(*this);
    v0.normalize();
    v.normalize();
    return static_cast<float>(dacos(v0*v));
}


bool vec3f::linearDependent(const vec3f & v, float tolerance) const {
	float factor=0.0f;
	if(fabs(coord[X])<=tolerance) { 
		if(fabs(v[X])>tolerance) return false;
	}
	else if(fabs(v[X])<=tolerance) return false;
	else factor = v[X]/coord[X];

	if(fabs(coord[Y])<=tolerance) { 
		if(fabs(v[Y])>tolerance) return false;
	}
	else if(fabs(v[Y])<=tolerance) return false;
	else if(factor&&(fabs(v[Y]/coord[Y]-factor)>tolerance)) return false;
	else factor = v[Y]/coord[Y];

	if(fabs(coord[Z])<=tolerance) { 
		if(fabs(v[Z])>tolerance) return false;
	}
	else if(fabs(v[Z])<=tolerance) return false;
	else if(factor&&(fabs(v[Z]/coord[Z]-factor)>tolerance)) return false;
	return true;
}

std::ostream & operator<<(std::ostream & os, const vec3f & v) {
    return os << v[X] << ' ' << v[Y] << ' ' << v[Z];
}


//--- class vec4f --------------------------------------------------

const vec4f & vec4f::operator=(const vec4f & source) {
    coord[X]=source[X];
    coord[Y]=source[Y];
    coord[Z]=source[Z];
    coord[W]=source[W];
    return *this;
}

bool vec4f::operator==(const vec4f &vt) const {
    if(fabs(coord[X]-vt[X])>EPSILONF) return false;
    if(fabs(coord[Y]-vt[Y])>EPSILONF) return false;
    if(fabs(coord[Z]-vt[Z])>EPSILONF) return false;
    if(fabs(coord[W]-vt[W])>EPSILONF) return false;
    return true;
}

std::ostream & operator<<(std::ostream & os, const vec4f & v) {
    return os << v[X] << ' ' << v[Y] << ' ' << v[Z] << ' ' << v[W];
}


//--- class vec4d --------------------------------------------------

const vec4d & vec4d::operator=(const vec4d & source) {
    coord[X]=source[X];
    coord[Y]=source[Y];
    coord[Z]=source[Z];
    coord[W]=source[W];
    return *this;
}

bool vec4d::operator==(const vec4d &vt) const {
    if(fabs(coord[X]-vt[X])>EPSILON) return false;
    if(fabs(coord[Y]-vt[Y])>EPSILON) return false;
    if(fabs(coord[Z]-vt[Z])>EPSILON) return false;
    if(fabs(coord[W]-vt[W])>EPSILON) return false;
    return true;
}


//--- class vec6f --------------------------------------------------

void vec6f::set(float x, float y, float z, float h, float pitch, float r) {
    coord[X]=x;
    coord[Y]=y;
    coord[Z]=z;
    ang[0]=h;
    ang[1]=pitch;
    ang[2]=r;
}

void vec6f::set(const mat4f & m) {
    coord[X]=m[12]; coord[Y]=m[13]; coord[Z]=m[14];
    float angleP=asin(m[6]);
    ang[1]=angleP*RAD2DEG;
    float cP=cos(angleP);
    if(fabs(cP)>0.005) { // no Gimbal lock, pitch!=90deg
        float cH= m[5]/cP;
        float sH=-m[4]/cP;
        ang[0]=static_cast<float>(atan2(sH,cH)*RAD2DEG);
        float cR=m[10]/cP; // E
        float sR=-m[2]/cP; // F
        ang[2]=atan2(sR,cR)*RAD2DEG;
    }
    else { // Gimbal lock, pitch==90deg
        ang[0]=static_cast<float>(atan2(m[8],m[0])*RAD2DEG);
        ang[2]=0.0f;
    }
}

void vec6f::get(float * f) const {
    f[0]=coord[X];
    f[1]=coord[Y];
    f[2]=coord[Z];
    f[3]=ang[0];
    f[4]=ang[1];
    f[5]=ang[2];
}

void vec6f::translate(const vec6f & v, float linearscale, float angularscale) {
	vec3f::translate(v[X]*linearscale, v[Y]*linearscale, v[Z]*linearscale);
	ang[0]+=v[H]*angularscale;
	ang[1]+=v[P]*angularscale;
	ang[2]+=v[R]*angularscale;
}

vec6f & vec6f::operator=(const vec6f & source) {
    if(this==&source) return *this;
    set(source.coord[X], source.coord[Y], source.coord[Z],
        source[H], source[P], source[R]);
    return *this;
}

void vec6f::operator+=(const vec6f & summand) {
    coord[X]  +=summand[X];
    coord[Y]  +=summand[Y];
    coord[Z]  +=summand[Z];
    ang[0]+=summand[H];
    ang[1]+=summand[P];
    ang[2]+=summand[R];
}

std::ostream & operator<<(std::ostream & os, const vec6f & sdof) {
    return os << sdof[X] << ' ' << sdof[Y] << ' ' << sdof[Z]
              << ' ' << sdof[H] << ' ' << sdof[P] << ' ' << sdof[R];
}

bool vec6f::operator==(const vec6f &sd) const {
    if((coord[X]!=sd[X])||(coord[Y]!=sd[Y])||(coord[Z]!=sd[Z])) return false;
    if((ang[0]!=sd[H])||(ang[1]!=sd[P])||(ang[2]!=sd[R])) return false;
    return true;
}


//--- class line ---------------------------------------------------

const line & line::operator=(const line& source) {
    if(&source==this) return *this;
    set(source.pt0,source.pt1);
    return *this;
}

float line::distTo(const vec3f & p) const {
    // project p on line:
    vec3f vpt0pt1(pt0,pt1);
    vec3f vpt0p(pt0,p);
    float vpt0pt1_sqrlength=vpt0pt1.sqrLength();
    float proj_factor = vpt0pt1_sqrlength ? (vpt0p*vpt0pt1)/vpt0pt1_sqrlength : 0.0f;
    vec3f vpt0pproj=vpt0pt1*proj_factor;
    vec3f pproj=pt0+vpt0pproj;
    // which of the 3 points is next to p?
    vec3f vpt1pproj(pt1,pproj);
    if ((vpt0pproj.sqrLength()<vpt0pt1.sqrLength()) && (vpt1pproj.sqrLength()<vpt0pt1.sqrLength()))
        return pproj.distTo(p);
    else return minAbs(pt0.distTo(p),pt1.distTo(p));
}

void line::transform(const mat4f & m) { 
    m.transform(&pt0[X],2); 
}

float line::intersect2d(const line &l) const {
    vec2f dir(pt1[X]-pt0[X],pt1[Y]-pt0[Y]);
    vec2f normal(-dir[Y], dir[X]);

    float minSqrDist=FLT_MAX;
    // translate position and line to origin:
    vec2f w1(l[0][X]-pt0[X],l[0][Y]-pt0[Y]);
    vec2f w2(l[1][X]-pt0[X],l[1][Y]-pt0[Y]);
    // test for position of endpoints:
    float nScalar1=normal*w1;
    float nScalar2=normal*w2;
    // check whether w1 or w2 are element of dir:
    if(!nScalar1) minSqrDist=w1.sqrLength();
    if(!nScalar2) minSqrDist=min(minSqrDist,w2.sqrLength());
    if(minSqrDist<FLT_MAX) return sqrt(minSqrDist);
    // test whether w1 and w2 are on different sides of dir:
    if(sgn(nScalar1)==sgn(nScalar2)) return -1;
    // test whether intersection is in positive direction:
    float scalar1=dir*w1;
    float scalar2=dir*w2;
    if((scalar1<0.0f)&&(scalar2<0.0f)) return -1;
    // compute intersection point:
    float n=(dir[X]*w1[Y]-dir[Y]*w1[X]) / (dir[Y]*(w2[X]-w1[X])-dir[X]*(w2[Y]-w1[Y]));
    vec2f w1_to_w2(w2[X]-w1[X],w2[Y]-w1[Y]);
    w1_to_w2*=n;
    vec2f intersection=w1+w1_to_w2;
    return intersection.length();
}

vec3f * line::intersection(const vec3f & tr0, const vec3f & tr1,
                           const vec3f & tr2, bool isRay) const {
    vec3f dir(pt0,pt1);
    // is dir parallel to tr?:
    vec3f edge1(tr0,tr1);
    vec3f edge2(tr0,tr2);
    vec3f pvec(dir.crossProduct(edge2));
    float det = edge1*pvec;
    if(det > -EPSILONF && det < EPSILONF) return 0;
    float invDet = 1.0f/det;

    // is intersection within triangle?:
    vec3f tvec(tr0,pt0);
    float u = (tvec*pvec) * invDet;
    if(u < 0.0f || u > 1.0f) return 0;
    vec3f qvec(tvec.crossProduct(edge1));
    float v = (dir*qvec) * invDet;
    if(v < 0.0f || (u + v) > 1.0f) return 0;

    // compute intersection distance from pt[0]:
    float dist = (edge2*qvec) * invDet;
    if(dist < 0.0f) return 0; // is ray direction wrong?

    if(!isRay&&(dist>1.0f)) return 0; // is distance longer than line?
    // compute intersection vertex:
    vec3f * pResult=new vec3f(pt0);
    pResult->translate(dir,dist);
    return pResult;
}

bool line::intersects(const vec3f & tr0, const vec3f & tr1,
                      const vec3f & tr2, bool isRay) const {
    vec3f dir(pt0,pt1);
    // is dir parallel to tr?:
    vec3f edge1(tr0,tr1);
    vec3f edge2(tr0,tr2);
    vec3f pvec(dir.crossProduct(edge2));
    float det = edge1*pvec;
    if(det > -EPSILONF && det < EPSILONF) return false;
    float invDet = 1.0f/det;

    // is intersection within triangle?:
    vec3f tvec(tr0,pt0);
    float u = (tvec*pvec) * invDet;
    if(u < 0.0f || u > 1.0f) return false;
    vec3f qvec(tvec.crossProduct(edge1));
    float v = (dir*qvec) * invDet;
    if(v < 0.0f || (u + v) > 1.0f) return false;

    // compute intersection distance from pt[0]:
    float dist = (edge2*qvec) * invDet;
    if(dist < 0.0f) return false; // is ray direction wrong?

    if(!isRay&&(dist>1.0f)) return false; // is distance longer than line?
    return true;
}

vec3f * line::intersection(const vec3f & center, float radius, bool isRay) const {
    vec3f rayToSphereCenter(pt0,center);
    vec3f dir(pt0,pt1);
    vec3f rayToClosestAppr(dir);
    rayToClosestAppr.project(rayToSphereCenter);
    if ((rayToClosestAppr*dir) < 0.0f ) return 0; // intersection is behind ray
    vec3f rayCToClAppr(rayToSphereCenter,rayToClosestAppr);
    float distFromClApprPow2=(radius*radius)-rayCToClAppr.sqrLength();
    if(distFromClApprPow2 < 0.0f) return 0; // the ray misses the sphere
    float distFromPt0 = rayToClosestAppr.length()-sqrt(distFromClApprPow2);
    float dirLength=dir.length();
    if(!isRay&&(distFromPt0>dirLength)) return 0; // is distance longer than line?
    // compute intersection vertex:
    vec3f * pResult=new vec3f(pt0);
    pResult->translate(dir,distFromPt0/dirLength);
    return pResult;
}

bool line::intersects(const vec3f & center, float radius, bool isRay) const {
    vec3f rayToSphereCenter(pt0,center);
    vec3f dir(pt0,pt1);
    vec3f rayToClosestAppr(dir);
    rayToClosestAppr.project(rayToSphereCenter);
    if ((rayToClosestAppr*dir) < 0.0f ) return false; // intersection is behind ray
    vec3f rayCToClAppr(rayToSphereCenter,rayToClosestAppr);
    float distFromClApprPow2=(radius*radius)-rayCToClAppr.sqrLength();
    if(distFromClApprPow2 < 0.0f) return false; // the ray misses the sphere
    float distFromPt0 = rayToClosestAppr.length()-sqrt(distFromClApprPow2);
    if(!isRay&&(distFromPt0>dir.length())) return false; // is distance longer than line?
    return true;
}

bool line::intersects(const plane & pl, bool isRay) const {
    vec3f dir(pt0,pt1);
    // is dir parallel to tr?:
    float dirMultNormal=dir*pl.normal();
    if (dirMultNormal > -EPSILONF && dirMultNormal < EPSILONF) return false; // line and plane are parallel
    // compute intersection distance from pt0:
    float dist = -(pl.normal()*pt0+pl.D())/dirMultNormal;
    if(dist < 0.0f) return false; // is ray direction wrong?
    if(!isRay) if(dist>1.0f) return false; // is distance longer than line?
    return true;
}

std::ostream & operator<<(std::ostream & os, const line & l) {
    return os << l[0] << ", " << l[1];
}

//--- class triangle -----------------------------------------------

triangle::triangle(const triangle & source) {
    pt[0]=source.pt[0];
    pt[1]=source.pt[1];
    pt[2]=source.pt[2];
}

triangle::triangle(const float * pCoords) {
    pt[0].set(pCoords[0],pCoords[1],pCoords[2]);
    pt[1].set(pCoords[3],pCoords[4],pCoords[5]);
    pt[2].set(pCoords[6],pCoords[7],pCoords[8]);
}

void triangle::set(float x1,float y1,float z1, float x2,float y2,float z2, float x3,float y3,float z3) {
    pt[0].set(x1,y1,z1);
    pt[1].set(x2,y2,z2);
    pt[2].set(x3,y3,z3);
}

void triangle::scale(float sx, float sy, float sz) {
    pt[0].scale(sx,sy,sz);
    pt[1].scale(sx,sy,sz);
    pt[2].scale(sx,sy,sz);
}

void triangle::transform(const mat4f & m) {
    pt[0].transform(m);
    pt[1].transform(m);
    pt[2].transform(m);
}

void triangle::transform(const vec6f & sdof) {
    pt[0].transform(sdof);
    pt[1].transform(sdof);
    pt[2].transform(sdof);
}

void triangle::getCoords(double * coords) const {
    coords[0]=pt[0][X];
    coords[1]=pt[0][Y];
    coords[2]=pt[0][Z];
    coords[3]=pt[1][X];
    coords[4]=pt[1][Y];
    coords[5]=pt[1][Z];
    coords[6]=pt[2][X];
    coords[7]=pt[2][Y];
    coords[8]=pt[2][Z];
}

static inline float scalarProduct(float x1, float y1, float x2, float y2) {
        return x1*x2 + y1*y2; 
}

bool triangle::isElemXY(const vec3f & p) const {
    if(p[X]<min3(pt[0][X],pt[1][X],pt[2][X])) return false;
    if(p[Y]<min3(pt[0][Y],pt[1][Y],pt[2][Y])) return false;
    if(p[X]>max3(pt[0][X],pt[1][X],pt[2][X])) return false;
    if(p[Y]>max3(pt[0][Y],pt[1][Y],pt[2][Y])) return false;

    if(scalarProduct(pt[0][Y]-pt[1][Y], pt[1][X]-pt[0][X],
                     p[X]-pt[0][X], p[Y]-pt[0][Y])<-EPSILON) return false;
    if(scalarProduct(pt[1][Y]-pt[2][Y], pt[2][X]-pt[1][X],
                     p[X]-pt[1][X], p[Y]-pt[1][Y])<-EPSILON) return false;
    if(scalarProduct(pt[2][Y]-pt[0][Y], pt[0][X]-pt[2][X],
                     p[X]-pt[2][X], p[Y]-pt[2][Y])<-EPSILON) return false;
    return true;
}

float triangle::distZ(const vec3f & p) const {
    vec3f n=normal();
    // coordinate form of plane:
    float & a=n[X];
    float & b=n[Y];
    float & c=n[Z];
    if(c==0.0f) return static_cast<float>(HUGE_VAL);
    float   d=-(a*pt[0][X]+b*pt[0][Y]+c*pt[0][Z]);
    // compute intersection with line l=p+t*(0|0|1):
    return p[Z]+(p[X]*a+p[Y]*b+d)/c;
}

vec3f triangle::normal() const {
    vec3f v1(pt[0],pt[1]);
    vec3f v2(pt[0],pt[2]);
    vec3f vPerpend=v1.crossProduct(v2);
    vPerpend.normalize();
    return vPerpend;
}

double triangle::area() const {
    double a=pt[0].distTo(pt[1]);
    double b=pt[1].distTo(pt[2]);
    double c=pt[2].distTo(pt[0]);
    double s=(a+b+c)*0.5;
    return sqrt(s*(s-a)*(s-b)*(s-c));
}

void triangle::getABCD(float &a, float &b, float &c, float &d) const {
    vec3f n=normal();
    vec3f nProject(n);
    nProject.project(pt[0]);

    a=n[X];
    b=n[Y];
    c=n[Z];
    d=nProject.length();
}

std::ostream & operator<<(std::ostream & os, const triangle & t) {
    return os << t[0] << ", " << t[1] << ", " << t[2];
}



//--- class plane --------------------------------------------------

plane::plane(const vec3f & normal, const vec3f & p) : n(normal) {
    n.normalize();
    d= -(n*p);
}

bool plane::operator==(const plane &pl) const {
    if(fabs(d-pl.d)>EPSILONF) return false;
    return n==pl.n;
}

bool plane::operator<(const plane &pl) const {
    if(n[X]!=pl.n[X]) return n[X]<pl.n[X] ? true : false;
    if(n[Y]!=pl.n[Y]) return n[Y]<pl.n[Y] ? true : false;
    if(n[Z]!=pl.n[Z]) return n[Z]<pl.n[Z] ? true : false;
    return d<pl.d ? true : false;
}

void plane::translate(const vec3f & v) {
    vec3f transl(n);
    transl.project(v);
    d+=((n*v)>0.0f) ? -transl.length() : transl.length();
}

void plane::transform(const vec6f & sdof) {
    n.rotate(sdof[H],sdof[P],sdof[R]);
    translate(sdof);
}

void plane::transform(const mat4f & m) {
	vec3f p(pivot());
	p.transform(m);
	mat4f mTrInv=m.inverse();
	mTrInv.transpose();
	vec3f nrm(normal());
	nrm.transform(mTrInv);
	operator=(plane(nrm,p));
}

vec3f * plane::intersection(const plane & pl2, const plane & pl3) const {
    vec3f t1(n.crossProduct(pl2.n));
    vec3f t4(t1);
    t1*=pl3.d;
    vec3f t2(pl3.n.crossProduct(n)*pl2.d);
    vec3f t3(pl2.n.crossProduct(pl3.n)*d);
    t1+=t2;
    t1+=t3;
    t1.invert();
    float f = t4*pl3.n;
    if(fabs(f)<EPSILONF) return 0;
    t1/=f;
    return new vec3f(t1);
}

std::ostream & operator<<(std::ostream & os, const plane & pl) {
    return os << pl.normal()[X] << ' ' << pl.normal()[Y] << ' ' << pl.normal()[Z] << ' ' << pl.D();
}


//--- random class -------------------------------------------------
unsigned int rnd::defaultSeed=0;

rnd::rnd(unsigned int seed) {
    if(!seed) {
        if(!defaultSeed) {
            struct timeb t;
            ftime(&t);
            seed=static_cast<unsigned int>(t.time*1000+t.millitm%RAND_MAX);
        }
        else seed=defaultSeed;
    }
    srand(seed);
    defaultSeed=rand();
}

int rnd::get() {
    return rand();
}

int rnd::get(int max) {
    return rand()%max;
}

float rnd::getf() {
    return (float)((float)rand()/(float)RAND_MAX);
}

//--- class mat4f ----------------------------------------------

mat4f::mat4f(const mat4f & source) {
    memcpy(m,source.m,16*sizeof(float));
}

mat4f::mat4f(float m0, float m1, float m2, float m3,
									float m4, float m5, float m6, float m7,
									float m8, float m9, float m10,float m11,
									float m12,float m13,float m14,float m15) {
    m[0]=m0; m[1]=m1; m[2]=m2; m[3]=m3;
    m[4]=m4; m[5]=m5; m[6]=m6; m[7]=m7;
    m[8]=m8; m[9]=m9; m[10]=m10; m[11]=m11;
    m[12]=m12; m[13]=m13; m[14]=m14; m[15]=m15;
}

void mat4f::set(float m0, float m1, float m2, float m3,
                float m4, float m5, float m6, float m7,
                float m8, float m9, float m10,float m11,
                float m12,float m13,float m14,float m15) {
    m[0]=m0; m[1]=m1; m[2]=m2; m[3]=m3;
    m[4]=m4; m[5]=m5; m[6]=m6; m[7]=m7;
    m[8]=m8; m[9]=m9; m[10]=m10; m[11]=m11;
    m[12]=m12; m[13]=m13; m[14]=m14; m[15]=m15;
}

void mat4f::set(float *f) {
    memcpy(m,f,16*sizeof(float));
}

void mat4f::set(const vec6f & sdof) {
    float sH = static_cast<float>(dsin(sdof[H])); float cH = static_cast<float>(dcos(sdof[H]));
    float sP = static_cast<float>(dsin(sdof[P])); float cP = static_cast<float>(dcos(sdof[P]));
    float sR = static_cast<float>(dsin(sdof[R])); float cR = static_cast<float>(dcos(sdof[R]));
    m[0] =cH*cR - sH*sP*sR; m[4]=-sH*cP; m[8] =cH*sR + sH*sP*cR; m[12]=sdof[X];
    m[1] =sH*cR + cH*sP*sR; m[5]= cH*cP; m[9] =sH*sR - cH*sP*cR; m[13]=sdof[Y];
    m[2] =cP*-sR          ; m[6]= sP   ; m[10]=cP*cR           ; m[14]=sdof[Z];
    m[3] =0.0f            ; m[7]= 0.0f ; m[11]=0.0f            ; m[15]=1.0f;
}

const mat4f & mat4f::operator=(const mat4f & source) {
    if(&source==this) return *this;
    memcpy(m,source.m,16*sizeof(float));
    return *this;
}

const mat4f mat4f::operator*(const mat4f & m) const {
    mat4f res;
    res[0] =(*this)[0]*m[0] + (*this)[4]*m[1] + (*this)[8]*m[2]  + (*this)[12]*m[3];
    res[4] =(*this)[0]*m[4] + (*this)[4]*m[5] + (*this)[8]*m[6]  + (*this)[12]*m[7];
    res[8] =(*this)[0]*m[8] + (*this)[4]*m[9] + (*this)[8]*m[10] + (*this)[12]*m[11];
    res[12]=(*this)[0]*m[12]+ (*this)[4]*m[13]+ (*this)[8]*m[14] + (*this)[12]*m[15];

    res[1] =(*this)[1]*m[0] + (*this)[5]*m[1] + (*this)[9]*m[2]  + (*this)[13]*m[3];
    res[5] =(*this)[1]*m[4] + (*this)[5]*m[5] + (*this)[9]*m[6]  + (*this)[13]*m[7];
    res[9] =(*this)[1]*m[8] + (*this)[5]*m[9] + (*this)[9]*m[10] + (*this)[13]*m[11];
    res[13]=(*this)[1]*m[12]+ (*this)[5]*m[13]+ (*this)[9]*m[14] + (*this)[13]*m[15];

    res[2] =(*this)[2]*m[0] + (*this)[6]*m[1] + (*this)[10]*m[2]  + (*this)[14]*m[3];
    res[6] =(*this)[2]*m[4] + (*this)[6]*m[5] + (*this)[10]*m[6]  + (*this)[14]*m[7];
    res[10]=(*this)[2]*m[8] + (*this)[6]*m[9] + (*this)[10]*m[10] + (*this)[14]*m[11];
    res[14]=(*this)[2]*m[12]+ (*this)[6]*m[13]+ (*this)[10]*m[14] + (*this)[14]*m[15];

    res[3] =(*this)[3]*m[0] + (*this)[7]*m[1] + (*this)[11]*m[2]  + (*this)[15]*m[3];
    res[7] =(*this)[3]*m[4] + (*this)[7]*m[5] + (*this)[11]*m[6]  + (*this)[15]*m[7];
    res[11]=(*this)[3]*m[8] + (*this)[7]*m[9] + (*this)[11]*m[10] + (*this)[15]*m[11];
    res[15]=(*this)[3]*m[12]+ (*this)[7]*m[13]+ (*this)[11]*m[14] + (*this)[15]*m[15];
    return res;
}

vec3f mat4f::operator*(vec3f& vV) const {
	float x=m[0]*vV[X] + m[4]*vV[Y] + m[8] *vV[Z] + m[12];
	float y=m[1]*vV[X] + m[5]*vV[Y] + m[9] *vV[Z] + m[13];
	float z=m[2]*vV[X] + m[6]*vV[Y] + m[10]*vV[Z] + m[14];
	return vec3f(x,y,z);
}

void mat4f::translate(const vec3f & v) {
    mat4f m;
    m[12]=v[X]; m[13]=v[Y]; m[14]=v[Z];
    operator*=(m);
}

void mat4f::rotateX(float angle){
	float c = static_cast<float>(dcos(angle));
	float s = static_cast<float>(dsin(angle));
	m[0] = 1.0; m[4] = 0.0; m[8] = 0.0; m[12] = 0.0;
	m[1] = 0.0; m[5] = c;	m[9] = -s;	m[13]= 0.0;
	m[2] = 0.0; m[6] = s;	m[10] = c;	m[14] = 0.0;
	m[3] = 0.0; m[7] = 0.0; m[11] = 0.0; m[15] = 1.0;
}


void mat4f::rotateY(float angle){
	float c = static_cast<float>(dcos(angle));
	float s = static_cast<float>(dsin(angle));
	m[0] = c;	m[4] = 0.0; m[8] = s;	m[12]= 0.0;
	m[1] = 0.0; m[5] = 1;	m[9] = 0;	m[13] = 0.0;
	m[2] = -s;	m[6] = 0;	m[10] = c;	m[14] = 0.0;
	m[3] = 0.0; m[7] = 0.0; m[11] = 0.0; m[15] = 1.0;
}

void mat4f::rotateZ(float angle){
	float c = static_cast<float>(dcos(angle));
	float s = static_cast<float>(dsin(angle));
	m[0] = c;	m[4] = -s;	m[8] = 0.0; m[12] = 0.0;
	m[1] = s;	m[5] = c;	m[9] = 0.0;	m[13] = 0.0;
	m[2] = 0.0;	m[6] = 0.0;	m[10] = 1.0; m[14] = 0.0;
	m[3] = 0.0; m[7] = 0.0; m[11] = 0.0; m[15] = 1.0;
}

void mat4f::rotate(float angle, vec3f p) {
    p.normalize();
    mat4f m;
    float c = static_cast<float>(dcos(angle));
    float s = static_cast<float>(dsin(angle));
    m[0]=c+p[X]*p[X]*(1-c);      m[4]=p[X]*p[Y]*(1-c)-p[Z]*s; m[8]=p[X]*p[Z]*(1-c)+p[Y]*s;
    m[1]=p[X]*p[Y]*(1-c)+p[Z]*s; m[5]=c+p[Y]*p[Y]*(1-c);      m[9]=p[Y]*p[Z]*(1-c)-p[X]*s;
    m[2]=p[X]*p[Z]*(1-c)-p[Y]*s; m[6]=p[Y]*p[Z]*(1-c)+p[X]*s; m[10]=c+p[Z]*p[Z]*(1-c);
    (*this)= (*this)*m;
}

void mat4f::scale(float sx, float sy, float sz) {
    mat4f m;
    m[0]=sx; m[5]=sy; m[10]=sz;
    (*this)= (*this)*m;
}

void mat4f::transform(std::vector<vec3f> & vV) const {
    for(unsigned int i=0; i<vV.size(); i++) {
        float x=m[0]*vV[i][X] + m[4]*vV[i][Y] + m[8] *vV[i][Z] + m[12];
        float y=m[1]*vV[i][X] + m[5]*vV[i][Y] + m[9] *vV[i][Z] + m[13];
        float z=m[2]*vV[i][X] + m[6]*vV[i][Y] + m[10]*vV[i][Z] + m[14];
        vV[i].set(x,y,z);
    }
}

void mat4f::transform(std::vector<float> & vF) const {
    for(unsigned int i=0; i<vF.size()-2; i+=3) {
        float x=m[0]*vF[i] + m[4]*vF[i+1] + m[8] *vF[i+2] + m[12];
        float y=m[1]*vF[i] + m[5]*vF[i+1] + m[9] *vF[i+2] + m[13];
        float z=m[2]*vF[i] + m[6]*vF[i+1] + m[10]*vF[i+2] + m[14];
        vF[i]=x; vF[i+1]=y; vF[i+2]=z;
    }
}

void mat4f::transform(float * pF, unsigned int n) const {
    for(unsigned int i=0; i<n*3-2; i+=3) {
        float x=m[0]*pF[i] + m[4]*pF[i+1] + m[8] *pF[i+2] + m[12];
        float y=m[1]*pF[i] + m[5]*pF[i+1] + m[9] *pF[i+2] + m[13];
        float z=m[2]*pF[i] + m[6]*pF[i+1] + m[10]*pF[i+2] + m[14];
        pF[i]=x; pF[i+1]=y; pF[i+2]=z;
    }
}

bool mat4f::isIdentity() const {
    if((m[0]!=1.0f)||(m[5]!=1.0f)||(m[10]!=1.0f)||(m[15]!=1.0f)) return false;
    if(m[1]||m[2]||m[3]) return false;
    if(m[4]||m[6]||m[7]) return false;
    if(m[8]||m[9]||m[11]) return false;
    if(m[12]||m[13]||m[14]) return false;
    return true;
}

bool mat4f::isIdentity(float tolerance) const {
    if((fabs(m[0]-1.0f)>tolerance)||(fabs(m[5]-1.0f)>tolerance)||(fabs(m[10]-1.0f)>tolerance)||(fabs(m[15]-1.0f)>tolerance)) return false;
    if((fabs(m[1])>tolerance)||(fabs(m[2])>tolerance)||(fabs(m[3])>tolerance)) return false;
    if((fabs(m[4])>tolerance)||(fabs(m[6])>tolerance)||(fabs(m[7])>tolerance)) return false;
    if((fabs(m[8])>tolerance)||(fabs(m[9])>tolerance)||(fabs(m[11])>tolerance)) return false;
    if((fabs(m[12])>tolerance)||(fabs(m[13])>tolerance)||(fabs(m[14])>tolerance)) return false;
    return true;
}

void mat4f::identity() {
    m[0]=1.0f;  m[1]=0.0f;  m[2]=0.0f;  m[3]=0.0f;
    m[4]=0.0f;  m[5]=1.0f;  m[6]=0.0f;  m[7]=0.0f;
    m[8]=0.0f;  m[9]=0.0f;  m[10]=1.0f; m[11]=0.0f;
    m[12]=0.0f; m[13]=0.0f; m[14]=0.0f; m[15]=1.0f;
}

void mat4f::nan() { 
	m[0]=static_cast<float>(NaN()); 
}

bool mat4f::isNan() const { 
#ifdef _MSC_VER
	return _isnan(m[0]) ? true : false; 
#else
	return isnan(m[0]) ? true : false; 
#endif
}

void mat4f::transpose() {
    float f[]={ m[1], m[2], m[3], m[6], m[7], m[11] };
    m[1]=m[4];  m[4] =f[0];
    m[2]=m[8];  m[8] =f[1];
    m[3]=m[12]; m[12]=f[2];
    m[6]=m[9];  m[9] =f[3];
    m[7]=m[13]; m[13]=f[4];
    m[11]=m[14];m[14]=f[5];
}

void mat4f::invert() {
    vec4d vRow[]={ row(0), row(1), row(2), row(3) };
    vec4d vRes[]={ vec4d(1,0,0,0), vec4d(0,1,0,0), vec4d(0,0,1,0), vec4d(0,0,0,1) };
    unsigned int i,j;
    const unsigned int sz=4;
    for(j=0; j<sz; ++j) {
        i=j;
        while(i<sz)
            if(!vRow[i][j]) ++i;
            else break;
        if(i==sz) { // singular matrix
            nan();
            return;
        }
        if(i!=j) {
            swap(vRes[j],vRes[i]);
            swap(vRow[j],vRow[i]);
        }
        vRes[j]/=vRow[j][j];
        vRow[j]/=vRow[j][j];
        for(i=0; i<sz; ++i) {
            if(i==j) continue;
            vRes[i]+=(vRes[j]*(-1.0*vRow[i][j]));
            vRow[i]+=(vRow[j]*(-1.0*vRow[i][j]));
        }
    }
    m[0]=vRes[0][0]; m[1]=vRes[1][0]; m[2]=vRes[2][0]; m[3]=vRes[3][0];
    m[4]=vRes[0][1]; m[5]=vRes[1][1]; m[6]=vRes[2][1]; m[7]=vRes[3][1];
    m[8]=vRes[0][2]; m[9]=vRes[1][2]; m[10]=vRes[2][2];m[11]=vRes[3][2];
    m[12]=vRes[0][3];m[13]=vRes[1][3];m[14]=vRes[2][3];m[15]=vRes[3][3];
}

mat4f mat4f::inverse() const {
    mat4f matInv(*this);
    matInv.invert();
    return matInv;
}

std::ostream & operator<<(std::ostream & os, const mat4f & m) {
	return os <<  m[0] << ' ' << m[1] << ' ' << m[2] << ' ' << m[3] << ", "
		<<  m[4] << ' ' << m[5] << ' ' << m[6] << ' ' << m[7] << ", "
		<<  m[8] << ' ' << m[9] << ' ' << m[10] << ' ' << m[11] << ", "
		<<  m[12] << ' ' << m[13] << ' ' << m[14] << ' ' << m[15];
}

//--- class sphere ---------------------------------------------

std::ostream & operator<<(std::ostream & os, const sphere & s) {
    return os << s[X] << ' ' << s[Y] << ' ' << s[Z] << ' ' << s.radius();
}


//--- class frustum ------------------------------------------------
frustum::frustum() {
    set(-1.0f, 1.0f, -0.75f, 0.75f, 0.1f, 1000.0f);
}

frustum::frustum(float clipLeft,float clipRight,float clipBottom,
                 float clipTop,float clipNear,float clipFar) {
    set(clipLeft, clipRight, clipBottom,
        clipTop, clipNear, clipFar);
}

frustum::frustum(const frustum & source) {
    for(unsigned int i=0; i<6; i++) pl[i]=source.pl[i];
}

const frustum & frustum::operator=(const frustum & source) {
    if(&source==this) return *this;
    for(unsigned int i=0; i<6; i++) pl[i]=source.pl[i];
    return *this;
}

void frustum::set(float clipLeft,float clipRight,float clipBottom,
                 float clipTop,float clipNear,float clipFar) {
    pl[0].normal().set(0,0,-1);
    pl[0].D()=clipNear;
    pl[1].normal().set(0,0,1);
    pl[1].D()=clipFar;
    pl[2].normal().set(clipNear,0,clipLeft);
    pl[2].normal().normalize();
    pl[3].normal().set(-clipNear,0,-clipRight);
    pl[3].normal().normalize();
    pl[4].normal().set(0,-clipNear,-clipTop);
    pl[4].normal().normalize();
    pl[5].normal().set(0,clipNear,clipBottom);
    pl[5].normal().normalize();
    pl[2].D()=pl[3].D()=pl[4].D()=pl[5].D()=0.0f;
}

bool frustum::intersects(const sphere & sph) const {
    for(unsigned int i=0; i<6; i++)
        if(pl[i].signedDistTo(sph)<=-sph.radius()) return false;
    return true;
}

bool frustum::intersects(const vec3f & vecMin, const vec3f & vecMax) const {
    for(unsigned int i=0; i<6; i++) {
        if(pl[i].signedDistTo(vecMin[X])>0.0f) continue;
        if(pl[i].signedDistTo(vec3f(vecMin[X],vecMin[Y],vecMax[Z]))>0.0f) continue;
        if(pl[i].signedDistTo(vec3f(vecMin[X],vecMax[Y],vecMin[Z]))>0.0f) continue;
        if(pl[i].signedDistTo(vec3f(vecMin[X],vecMax[Y],vecMax[Z]))>0.0f) continue;
        if(pl[i].signedDistTo(vec3f(vecMax[X],vecMin[Y],vecMin[Z]))>0.0f) continue;
        if(pl[i].signedDistTo(vec3f(vecMax[X],vecMin[Y],vecMax[Z]))>0.0f) continue;
        if(pl[i].signedDistTo(vec3f(vecMax[X],vecMax[Y],vecMin[Z]))>0.0f) continue;
        if(pl[i].signedDistTo(vecMax[X])>0.0f) continue;
        return false;
    }
    return true;
}

void frustum::translate(const vec3f & v) {
    for(unsigned int i=0; i<6; i++) pl[i].translate(v);
}

void frustum::rotate(float angle, const vec3f & v) {
    for(unsigned int i=0; i<6; i++) pl[i].rotate(angle,v);
}

void frustum::rotate(float h, float p, float r) {
    for(unsigned int i=0; i<6; i++) pl[i].rotate(h,p,r);
}

void frustum::transform(const vec6f & sdof) {
    for(unsigned int i=0; i<6; i++) pl[i].transform(sdof);
}
