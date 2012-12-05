/********************************************************************

	Quaternion.cpp	Source  File

	Quaternion Object, Methods, and Procedures
	Donald H. House  November 9, 2000
		Visualization Laboratory
		Texas A&M University

	Q to M and M to Q code from Baraff & Witkin 
        SIGGRAPH 97 Course Notes, Course 19, Physically Based Modeling

*********************************************************************/

#include "Quaternion.h"
#include <iostream>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

using namespace std;

//
// Constructors
//
Quaternion::Quaternion(const Quaternion &quat){
  set(quat);
}

Quaternion::Quaternion(const Vector3d &vect){
  set(vect);
}

Quaternion::Quaternion(double angle, double x, double y, double z){
  set(angle, x, y, z);
}

Quaternion::Quaternion(double yaw, double pitch, double roll){
  set(yaw, pitch, roll);
}

Quaternion::Quaternion(double angle, const Vector3d &axis){
  set(angle, axis);
}

Quaternion::Quaternion(const Matrix3x3 &rot){
  set(rot);
}

Quaternion::Quaternion(const Matrix4x4 &rot){
  set(rot);
}

//
// Destructor, nothing to do
//
Quaternion::~Quaternion()
{
}

//
// convert the vector part of quaternion to a vector
//
Quaternion::operator struct Vector3d(){
  Vector3d vect;
  vect.x = q.x;
  vect.y = q.y;
  vect.z = q.z;
  return vect;
}

void Quaternion::print(){
  Vector3d axis;
  axis.x = q.x; axis.y = q.y; axis.z = q.z;
  cout << "{" << q.w << " ";
  axis.print();
  cout << "}";
}

void Quaternion::print(int w, int p){
  Vector3d axis;
  axis.x = q.x; axis.y = q.y; axis.z = q.z;
  cout << "{" << setw(w) << setprecision(p) << Round(q.w, p) << " ";
  axis.print(w, p);
  cout << "}";
}

Quaternion Quaternion::normalize(){
  double magnitude;
  Quaternion newq;

  magnitude = norm();
  if (abs(q.x) > magnitude * HUGENUMBER ||
      abs(q.y) > magnitude * HUGENUMBER ||
      abs(q.z) > magnitude * HUGENUMBER ||
      abs(q.w) > magnitude * HUGENUMBER){
    cerr << "Attempting to take the norm of a zer3o quaternion." << endl;
  }

  newq.q.x = q.x / magnitude;
  newq.q.y = q.y / magnitude;
  newq.q.z = q.z / magnitude;
  newq.q.w = q.w / magnitude;

  return newq;
}

//
// Set quaternion to the identity quaternion:
//   vector = (0, 0, 0)
//   scalar = 1
//
void Quaternion::identity()
{
  q.x = q.y = q.z = 0.0;	
  q.w = 1.0;
}

//
// Set quaternion to an initial value 
//
void Quaternion::set(const Quaternion &quat){
  q = quat.q;
}

void Quaternion::set(const Vector3d &vect){
  q.x = vect.x;
  q.y = vect.y;
  q.z = vect.z;
  q.w = 0;
}

void Quaternion::set(double yaw, double pitch, double roll){
  set(roll, 0, 0, 1);
  rotate(pitch, 1, 0, 0);
  rotate(yaw, 0, 1, 0);
}

void Quaternion::set(double angle, double x, double y, double z){
  Vector3d axis(x, y, z);

  set(angle, axis);
}

void Quaternion::set(double angle, const Vector3d &axis){
  Vector3d normaxis = axis.normalize();
  double halfangle = PI * angle / 360;

  normaxis = sin(halfangle) * normaxis;
  q.x = normaxis.x;
  q.y = normaxis.y;
  q.z = normaxis.z;

  q.w = cos(halfangle);
}

void Quaternion::set(const Matrix3x3 &rot){
  Matrix3x3 myrot = rot;
  double trace, s;

  trace = myrot[0][0] + myrot[1][1] + myrot[2][2];

  if(trace >= 0){
    s = sqrt(trace + 1);
    q.w = 0.5 * s;
    q.x = s * (myrot[2][1] - myrot[1][2]);
    q.y = s * (myrot[0][2] - myrot[2][0]);
    q.z = s * (myrot[1][0] - myrot[0][1]);
  }
  else{
    int i;
    i = (myrot[1][1] > myrot[0][0])? 1 : 0;
    i = (myrot[2][2] > myrot[i][i])? 2 : i;

    switch(i){
    case 0:
      s = sqrt((myrot[0][0] - (myrot[1][1] + myrot[2][2])) + 1);
      q.x = 0.5 * s;
      s = 0.5 / s;
      q.y = s * (myrot[0][1] + myrot[1][0]);
      q.z = s * (myrot[2][0] + myrot[0][2]);
      q.w = s * (myrot[2][1] - myrot[1][2]);
      break;

    case 1:
      s = sqrt((myrot[1][1] - (myrot[2][2] + myrot[0][0])) + 1);
      q.y = 0.5 * s;
      s = 0.5 / s;
      q.z = s * (myrot[1][2] + myrot[2][1]);
      q.x = s * (myrot[0][1] + myrot[1][0]);
      q.w = s * (myrot[0][2] - myrot[2][0]);

      break;
    case 2:
      s = sqrt((myrot[2][2] - (myrot[0][0] + myrot[1][1])) + 1);
      q.z = 0.5 * s;
      s = 0.5 / s;
      q.x = s * (myrot[2][0] + myrot[0][2]);
      q.y = s * (myrot[1][2] + myrot[2][1]);
      q.w = s * (myrot[1][0] - myrot[0][1]);
    }
  }
}

void Quaternion::set(const Matrix4x4 &rot){
  Matrix3x3 r;

  for(int row = 0; row < 3; row++)
    for(int col = 0; col < 3; col++)
      r[row][col] = rot[row][col];

  set(r);
}

//
// Append a rotation onto the current quaternion, via quaternion multiply
// in order such that last applied is first done when rotating
//
void Quaternion::rotate(const Quaternion &rotq)
{
  Vector4d orig = q;

  q.x = orig.w * rotq.q.x + rotq.q.w * orig.x + 
          (orig.y * rotq.q.z - orig.z * rotq.q.y);
  q.y = orig.w * rotq.q.y + rotq.q.w * orig.y + 
          (orig.z * rotq.q.x - orig.x * rotq.q.z);
  q.z = orig.w * rotq.q.z + rotq.q.w * orig.z + 
          (orig.x * rotq.q.y - orig.y * rotq.q.x);
  q.w = orig.w * rotq.q.w - 
          (orig.x * rotq.q.x + orig.y * rotq.q.y + orig.z * rotq.q.z);
}

void Quaternion::rotate(double yaw, double pitch, double roll){
  rotate(roll, 0, 0, 1);
  rotate(pitch, 1, 0 , 0);
  rotate(yaw, 0, 1, 0);
}

void Quaternion::rotate(double angle, double x, double y, double z){
  Vector3d axis(x, y, z);

  rotate(angle, axis);
}

void Quaternion::rotate(double angle, const Vector3d &axis){
  Quaternion  newq(angle, axis);

  rotate(newq);
}

void Quaternion::rotate(const Matrix3x3 &rot){
  Quaternion newq(rot);

  rotate(newq);
}

void Quaternion:: rotate(const Matrix4x4 &rot){
  Quaternion newq(rot);

  rotate(newq);
}

//
// construct and return a general rotation transformation matrix from the 
// normalized (unit length) quaternion
//
Matrix3x3 Quaternion::rotation()
{
  Matrix3x3 rot;
  /* in the following variable names, tx means 2x, and x2 means x^2 */
  double tx,ty,tz,tx2,ty2,tz2,txy,txz,txw,tyz,tyw,tzw;
    
  /* construct the rotation matrix from the Quaternion, which
     is assumed to already be of unit length */
  tx = q.x + q.x;
  ty = q.y + q.y;
  tz = q.z + q.z;
  tx2 = tx * q.x;
  txy = tx * q.y;
  txz = tx * q.z;
  txw = tx * q.w;
  ty2 = ty * q.y;
  tyz = ty * q.z;
  tyw = ty * q.w;
  tz2 = tz * q.z;
  tzw = tz * q.w;

  /* make matrix row by row from above subelements */
  rot[0][0] = 1 - ty2 - tz2;
  rot[0][1] = txy - tzw;
  rot[0][2] = txz + tyw;
  rot[1][0] = txy + tzw;
  rot[1][1] = 1 - tx2 - tz2;
  rot[1][2] = tyz - txw;
  rot[2][0] = txz - tyw;
  rot[2][1] = tyz + txw;
  rot[2][2] = 1 - tx2 - ty2;
  
  return(rot);
}

double Quaternion::angle(){	// rotation angle, assuming rotation quaternion
  return 360 * acos(q.w) / PI;
}

Vector3d Quaternion::axis(){	// rotation axis, assuming rotation quaternion
  Vector3d ax;

  ax.x = q.x; ax.y = q.y; ax.z = q.z;
  return ax.normalize();
}

float *Quaternion::GLrotation(){	// return rotation matrix in form
  Matrix4x4 rot = rotation();		// needed by OpenGL
  float *glrot = new float[16];		// caution -- delete when done

  for(int row = 0; row < 4; row++)
    for(int col = 0; col < 4; col++)
      glrot[col * 4 + row] = rot[row][col];

  return glrot;
}

void Quaternion::GLrotate(){		// do the OpenGL rotation
  Vector3d v = axis();

  glRotatef(angle(), v.x, v.y, v.z);
}

double Quaternion::norm(){
  return sqrt(normsqr());
}

double Quaternion::normsqr(){
  return Sqr(q.x) + Sqr(q.y) + Sqr(q.z) + Sqr(q.w);
}

Quaternion Quaternion::inv(){
  Quaternion newq;
  double magsq = normsqr();
  
  newq.q.w = q.w / magsq;
  newq.q.x = -q.x / magsq;
  newq.q.y = -q.y / magsq;
  newq.q.z = -q.z / magsq;

  return newq;
}

// unary negation, negate the vector part to reverse direction of rotation
Quaternion operator-(const Quaternion &q){
  Quaternion newq = q;

  newq.q.x = -newq.q.x;
  newq.q.y = -newq.q.y;
  newq.q.z = -newq.q.z;
  return newq;
}

Quaternion operator+(const Quaternion &q1, const Quaternion &q2){
  Quaternion newq;

  newq.q = q1.q + q2.q;
  return newq;
}

Quaternion operator-(const Quaternion &q1, const Quaternion &q2){
  Quaternion newq;

  newq.q = q1.q - q2.q;
  return newq;
}

Quaternion operator*(const Quaternion &q, double s){
  Quaternion newq;

  newq.q = s * q.q;
  return newq;
}

Quaternion operator*(double s, const Quaternion &q){
  Quaternion newq;

  newq.q = s * q.q;
  return newq;
}

Quaternion operator*(const Quaternion &q1, const Quaternion &q2){
  Vector3d v1(q1.q.x, q1.q.y, q1.q.z);
  Vector3d v2(q2.q.x, q2.q.y, q2.q.z);;
  double s1 = q1.q.w, s2 = q2.q.w;

  Vector3d newv = s1 * v2 + s2 * v1 + v1 % v2;
  Quaternion newq(newv);
  newq.q.w = s1 * s2 - v1 * v2;
  
  return newq;
}

Quaternion operator*(const Vector3d &v, const Quaternion &q){
  Quaternion vq(v);
  Quaternion newq;

  newq = vq * q;
  return newq;
}

Quaternion operator*(const Quaternion &q, const Vector3d &v){
  Quaternion vq(v);
  Quaternion newq;

  newq = q * vq;
  return newq;
}

Quaternion operator/(const Quaternion &q, double s){
  Quaternion newq;

  newq.q = q.q / s;
  return newq;
}

short operator==(const Quaternion &q1, const Quaternion &q2){
  return (q1.q == q2.q);
}

const Quaternion& Quaternion::operator=(const Quaternion& qright){	// assignment

  q = qright.q;

  return *this;
}

ostream& operator<< (ostream& os, const Quaternion& q){
  Vector3d axis;
  axis.x = q.q.x; axis.y = q.q.y; axis.z = q.q.z;
  os << "{" << q.q.w << " " << axis << "}";
  return os;
}

