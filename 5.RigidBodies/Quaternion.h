/********************************************************************

	Quaternion.h	Header  File

	Quaternion Object, Methods, and Procedures
	Donald H. House  November 9, 2000
		Visualization Laboratory
		Texas A&M University

	Q to M and M to Q code from Baraff & Witkin 
        SIGGRAPH 97 Course Notes, Course 19, Physically Based Modeling

*********************************************************************/

#ifndef _QUATERNION_H_
#define _QUATERNION_H_

#include "Matrix.h"

struct Quaternion{
public:
  Vector4d q;		// quaternion is stored as a 4D vector, q, where:
  			// (x, y, z) is vector part and w is scalar part

  Quaternion(const Quaternion &quat);	// copy constructor
  Quaternion(const Vector3d &vect);	// copy, setting scalar part = 0

  // the following constructors assume a rotation quaternion, where angle
  // will be the rotation angle in degrees, and the vector is the rotation axis
  // scalar part will be sin(angle/2), vector part will be cos(angle/2)u,
  // where u is the unit vector in the direction of the vector parameter
  Quaternion(double yaw = 0, double pitch = 0, double roll = 0); // first yaw, then pitch, then roll
  Quaternion(double angle, double x, double y, double z);
  Quaternion(double angle, const Vector3d &axis);
  Quaternion(const Matrix3x3 &rot);
  Quaternion(const Matrix4x4 &rot);

  ~Quaternion();	// quaternion destructor

  operator struct Vector3d();

  void print();
  void print(int w, int p);	// print with width and precision

  Quaternion normalize();

  void identity();	// set quaternion to the identity quaternion

  void set(const Quaternion &quat);	// set quaternion to another
  void set(const Vector3d &vect);	// set scalar part to 0
  // the following set quaternion to a rotation quaternion. The matrices
  // must be valid rotations for this to work correctly
  void set(double yaw = 0, double pitch = 0, double roll = 0);
  void set(double angle, double x, double y, double z);
  void set(double angle, const Vector3d & axis);
  void set(const Matrix3x3 &rot);
  void set(const Matrix4x4 &rot);

  void rotate(const Quaternion &rotq); // rotate quaternion by another
  void rotate(double yaw, double pitch, double roll);
  void rotate(double angle, double x, double y, double z);
  void rotate(double angle, const Vector3d &axis);
  void rotate(const Matrix3x3 &rot);
  void rotate(const Matrix4x4 &rot);

  Matrix3x3 rotation();		// return rotation matrix for quaternion

  double angle();		// rotation angle, assuming rotation quaternion
  Vector3d axis();		// rotation axis, assuming rotation quaternion

  float *GLrotation();		// return rotation matrix for quaternion,
				// stored as 16 floats needed by OpenGL
  void GLrotate();		// do OpenGL rotation according to quaternion

  double norm();		// quaternion magnitude (root square of 4 components)
  double normsqr();		// square of quaternion magnitude
  Quaternion inv();		// inverse of quaternion such that q * q.inv() = [1, (0, 0, 0)]

  /* Quaternion operator prototypes */
  friend Quaternion operator-(const Quaternion &q);	// unary negation
  friend Quaternion operator+(const Quaternion &q1, const Quaternion &q2); // +
  friend Quaternion operator-(const Quaternion &q1, const Quaternion &q2); // -
  friend Quaternion operator*(const Quaternion &q, double s); // scalar * q
  friend Quaternion operator*(double s, const Quaternion &q); // q * scalar
  friend Quaternion operator*(const Quaternion &q1, const Quaternion &q2); // q1 * q2
  friend Quaternion operator*(const Vector3d &v, const Quaternion &q); // vect * q
  friend Quaternion operator*(const Quaternion &q, const Vector3d &v); // q * vect
  friend Quaternion operator/(const Quaternion &q, double s); // q / scalar
  friend short operator==(const Quaternion &q1, const Quaternion &q2); // ==
  const Quaternion& operator=(const Quaternion& qright);  // assignment
  friend ostream& operator<< (ostream& os, const Quaternion& q);   // output to stream
};

#endif
