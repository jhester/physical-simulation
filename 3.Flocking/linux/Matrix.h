/********************************************************************

	Matrix.h	Header File

	Matrix Algebra Objects, Methods, and Procedures
		Donald H. House  April 17, 1997
		Visualization Laboratory
		Texas A&M University

	Copyright (C) - Donald H. House. 2005

*********************************************************************/

#ifndef _H_Matrix
#define _H_Matrix

#include "Vector.h"

/* Matrix Descriptions and Operations */

class Matrix2x2;
class Matrix3x3;
class Matrix4x4;
class Matrix;

//
// The internal storage form of transforms.  The matrices in row-major
// order (ie  mat[row][column] )
//

class Matrix2x2 {
protected:
  Vector2d row[2];

public:
  Matrix2x2(double a11 = 0, double a12 = 0,
	    double a21 = 0, double a22 = 0);

  Vector2d& operator[](int i);
  const Vector2d& operator[](int i) const;

  operator Matrix3x3();
  operator Matrix4x4();
  operator Matrix();

  void print(int w = 7, int p = 3) const; // print with width and precision

  void set(double a11 = 0, double a12 = 0,
	   double a21 = 0, double a22 = 0);
  void identity();

  Matrix2x2 transpose() const;
  Matrix2x2 inv() const;

  friend Matrix2x2 operator+(const Matrix2x2& m1, const Matrix2x2& m2);
  friend Matrix2x2 operator-(const Matrix2x2& m1, const Matrix2x2& m2);
  friend Matrix2x2 operator*(const Matrix2x2& m1, const Matrix2x2& m2);
  friend Matrix2x2 operator*(double a, const Matrix2x2& m);
  Matrix2x2 operator*(double a) const;
  friend ostream& operator<< (ostream& os, const Matrix2x2& m);

  // mat times vector
  friend Vector2d operator*(const Matrix2x2& m, const Vector2d& v);
  // vector times mat
  friend Vector2d operator*(const Vector2d& v, const Matrix2x2& m);
  // outer product
  friend Matrix2x2 operator&(const Vector2d& v1, const Vector2d& v2);
};

class Matrix3x3 {
protected:
  Vector3d row[3];

public:
  Matrix3x3(double a11 = 0, double a12 = 0,
	    double a21 = 0, double a22 = 0);
  Matrix3x3(double a11, double a12, double a13,
	    double a21, double a22, double a23,
	    double a31, double a32, double a33);

  Vector3d& operator[](int i);
  const Vector3d& operator[](int i) const;

  operator Matrix4x4();
  operator Matrix();

  void print(int w = 7, int p = 3) const;  // print with width and precision

  void set(double a11 = 0, double a12 = 0,
	   double a21 = 0, double a22 = 0);
  void set(double a11, double a12, double a13,
	   double a21, double a22, double a23,
	   double a31, double a32, double a33);
  void identity();

  Matrix3x3 transpose() const;
  Matrix3x3 inv() const;

  friend Matrix3x3 operator+(const Matrix3x3& m1, const Matrix3x3& m2);
  friend Matrix3x3 operator-(const Matrix3x3& m1, const Matrix3x3& m2);
  friend Matrix3x3 operator*(const Matrix3x3& m1, const Matrix3x3& m2);
  friend Matrix3x3 operator*(double a, const Matrix3x3& m);
  Matrix3x3 operator*(double a) const;
  friend ostream& operator<< (ostream& os, const Matrix3x3& m);

  // mat times vector
  friend Vector3d operator*(const Matrix3x3& m, const Vector3d& v);
  // vector times mat
  friend Vector3d operator*(const Vector3d& v, const Matrix3x3& m);
  // outer product
  friend Matrix3x3 operator&(const Vector3d& v1, const Vector3d& v2);
};

class Matrix4x4 {
protected:
  Vector4d row[4];

public:
  Matrix4x4(double a11 = 0, double a12 = 0, double a13 = 0,
	    double a21 = 0, double a22 = 0, double a23 = 0,
	    double a31 = 0, double a32 = 0, double a33 = 0);
  Matrix4x4(double a11, double a12, double a13, double a14,
	    double a21, double a22, double a23, double a24,
	    double a31, double a32, double a33, double a34,
	    double a41, double a42, double a43, double a44);

  operator Matrix();

  Vector4d& operator[](int i);
  const Vector4d& operator[](int i) const;

  void print(int w = 7, int p = 3) const;  // print with width and precision

  void set(double a11 = 0, double a12 = 0, double a13 = 0,
	   double a21 = 0, double a22 = 0, double a23 = 0,
	   double a31 = 0, double a32 = 0, double a33 = 0);
  void set(double a11, double a12, double a13, double a14,
	   double a21, double a22, double a23, double a24,
	   double a31, double a32, double a33, double a34,
	   double a41, double a42, double a43, double a44);
  void identity();

  Matrix4x4 transpose() const;
  Matrix4x4 inv() const;

  friend Matrix4x4 operator+(const Matrix4x4& m1, const Matrix4x4& m2);
  friend Matrix4x4 operator-(const Matrix4x4& m1, const Matrix4x4& m2);
  friend Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2);
  friend Matrix4x4 operator*(double a, const Matrix4x4& m);
  Matrix4x4 operator*(double a) const;
  friend ostream& operator<< (ostream& os, const Matrix4x4& m);

  friend Matrix4x4 LU_Decompose(const Matrix4x4& M, int *indx);
  friend void LU_back_substitution(const Matrix4x4& M, int *indx, double col[]);
  // mat times vector
  friend Vector4d operator*(const Matrix4x4& m, const Vector4d& v);
  // vector times mat
  friend Vector4d operator*(const Vector4d& v, const Matrix4x4& m);
  // outer product
  friend Matrix4x4 operator&(const Vector4d& v1, const Vector4d& v2);
};

class Matrix {
protected:
  Vector *row;
  int Nrows, Ncols;

public:
  Matrix(int rows = 0, int cols = 0, const double *M = NULL);
  Matrix(const Matrix& M);
  Matrix(double a11, double a12,
	 double a21, double a22);
  Matrix(double a11, double a12, double a13,
	 double a21, double a22, double a23,
	 double a31, double a32, double a33);
  Matrix(double a11, double a12, double a13, double a14,
	 double a21, double a22, double a23, double a24,
	 double a31, double a32, double a33, double a34,
	 double a41, double a42, double a43, double a44);

  ~Matrix();

  void setsize(int rows, int cols);
  int nrows() const;
  int ncols() const;

  operator Matrix2x2();
  operator Matrix3x3();
  operator Matrix4x4();

  Vector& operator[](int i);
  const Vector& operator[](int i) const;

  void print(int w = 7, int p = 3) const;  // print with width and precision

  void set(double *M);
  void set(double a11, double a12,
	   double a21, double a22);
  void set(double a11, double a12, double a13,
	   double a21, double a22, double a23,
	   double a31, double a32, double a33);
  void set(double a11, double a12, double a13, double a14,
	   double a21, double a22, double a23, double a24,
	   double a31, double a32, double a33, double a34,
	   double a41, double a42, double a43, double a44);
  void identity();

  Matrix transpose() const;
  Matrix inv() const;
  void svd(Matrix &U, Vector &W, Matrix &V) const;
 
  const Matrix& operator=(const Matrix& m2);
  friend Matrix operator+(const Matrix& m1, const Matrix& m2);
  friend Matrix operator-(const Matrix& m1, const Matrix& m2);
  friend Matrix operator*(const Matrix& m1, const Matrix& m2);
  friend Matrix operator*(double a, const Matrix& m);
  Matrix operator*(double a) const;
  friend ostream& operator<< (ostream& os, const Matrix& m);

  friend Matrix LU_Decompose(const Matrix& M, int *indx);
  friend void LU_back_substitution(const Matrix& M, int *indx, double col[]);
  // mat times vector
  friend Vector operator*(const Matrix& m, const Vector& v); 
  // vector times mat
  friend Vector operator*(const Vector& v, const Matrix& m);
  // outer product
  friend Matrix operator&(const Vector& v1, const Vector& v2);
};

Matrix diag(const Vector &V);

#endif
