/********************************************************************
 
 Matrix.cpp	Source File
 
 Matrix Algebra Objects, Methods, and Procedures
 Donald H. House  April 17, 1997
 Visualization Laboratory
 Texas A&M University
 
 Matrix Inversion Code from Numerical Recipes
 
 Copyright (C) - Donald H. House. 2005
 
 *********************************************************************/

using namespace std;

#include "Matrix.h"

/* Matrix Descriptions and Operations */

// Matrix constructors
Matrix2x2::Matrix2x2(double a11, double a12,
					 double a21, double a22)
{
	set(a11, a12, a21, a22);
}

Matrix3x3::Matrix3x3(double a11, double a12,
					 double a21, double a22)
{
	set(a11, a12, a21, a22);
}

Matrix3x3::Matrix3x3(double a11, double a12, double a13,
					 double a21, double a22, double a23,
					 double a31, double a32, double a33)
{
	set(a11, a12, a13, a21, a22, a23, a31, a32, a33);
}

Matrix4x4::Matrix4x4(double a11, double a12, double a13,
					 double a21, double a22, double a23,
					 double a31, double a32, double a33)
{
	set(a11, a12, a13, a21, a22, a23, a31, a32, a33);
}

Matrix4x4::Matrix4x4(double a11, double a12, double a13, double a14,
					 double a21, double a22, double a23, double a24,
					 double a31, double a32, double a33, double a34,
					 double a41, double a42, double a43, double a44)
{
	set(a11, a12, a13, a14, a21, a22, a23, a24,
		a31, a32, a33, a34, a41, a42, a43, a44);
}

Matrix::Matrix(int rows, int cols, const double *M)
{
	int i, j, k;
	setsize(rows, cols);
	
	if(M != NULL)
		for(i = 0, k = 0; i < rows; i++)
			for(j = 0; j < cols; j++)
				(row[i])[j] = M[k++];
}

Matrix::Matrix(const Matrix& M)
{
	int i, j;
	
	setsize(M.Nrows, M.Ncols);
	for(i = 0; i < Nrows; i++)
		for(j = 0; j < Ncols; j++)
			row[i][j] = M.row[i][j];
}

Matrix::Matrix(double a11, double a12,
			   double a21, double a22)
{
	setsize(2, 2);
	row[0].set(a11, a12);
	row[1].set(a21, a22);
}

Matrix::Matrix(double a11, double a12, double a13,
			   double a21, double a22, double a23,
			   double a31, double a32, double a33)
{
	setsize(3, 3);
	row[0].set(a11, a12, a13);
	row[1].set(a21, a22, a23);
	row[2].set(a31, a32, a33);
}

Matrix::Matrix(double a11, double a12, double a13, double a14,
			   double a21, double a22, double a23, double a24,
			   double a31, double a32, double a33, double a34,
			   double a41, double a42, double a43, double a44)
{
	setsize(4, 4);
	row[0].set(a11, a12, a13, a14);
	row[1].set(a21, a22, a23, a24);
	row[2].set(a31, a32, a33, a34);
	row[3].set(a41, a42, a43, a44);
}

// Destructor
Matrix::~Matrix()
{
	delete []row;
}

void Matrix::setsize(int rows, int cols)
{
	int i;
	
	if(rows < 0 || cols < 0 || ((rows == 0 || cols == 0) && cols != rows)){
		cerr << "Matrix size impossible (negative or zero rows or cols)" << endl;
		exit(1);
	}
	Nrows = rows;
	Ncols = cols;
	
	if(Nrows == 0)
		row = NULL;
	else{
		row = new Vector[rows];
		if(!row){
			cerr << "not enough memory to allocate " << rows << " x " << cols << 
			" Matrix" << endl;
			exit(1);
		}
		for(i = 0; i < rows; i++)
			row[i].setsize(cols);
	}
	
}

int Matrix::nrows() const
{
	return Nrows;
}

int Matrix::ncols() const
{
	return Ncols;
}

//
// return row i of matrix as a vector
// Routines returning an lvalue: i.e. M[i] returns ref to row i
//
Vector2d& Matrix2x2::operator[](int i)
{
	if(i < 0 || i > 1){
		cerr << "Matrix2x2 row index " << i << " out of bounds" << endl;
		exit(1);
	}
	
	return (row[i]);
}
Vector3d& Matrix3x3::operator[](int i)
{
	if(i < 0 || i > 2){
		cerr << "Matrix3x3 row index " << i << " out of bounds" << endl;
		exit(1);
	}
	
	return (row[i]);
}
Vector4d& Matrix4x4::operator[](int i)
{
	if(i < 0 || i > 3){
		cerr << "Matrix4x4 row index " << i << " out of bounds" << endl;
		exit(1);
	}
	
	return (row[i]);
}
Vector& Matrix::operator[](int i)
{
	if(i < 0 || i >= Nrows){
		cerr << "Matrix row index of " << i << " out of bounds" << endl;
		exit(1);
	}
	
	return (row[i]);
}

//
// return row i of matrix as a vector
// Routines returning an rvalue: i.e. X[i] returns const ref to row i
//
const Vector2d& Matrix2x2::operator[](int i) const
{
	if(i < 0 || i > 1){
		cerr << "Matrix2x2 row index " << i << " out of bounds" << endl;
		exit(1);
	}
	
	return (row[i]);
}
const Vector3d& Matrix3x3::operator[](int i) const
{
	if(i < 0 || i > 2){
		cerr << "Matrix3x3 row index " << i << " out of bounds" << endl;
		exit(1);
	}
	
	return (row[i]);
}
const Vector4d& Matrix4x4::operator[](int i) const
{
	if(i < 0 || i > 3){
		cerr << "Matrix4x4 row index " << i << " out of bounds" << endl;
		exit(1);
	}
	
	return (row[i]);
}
const Vector& Matrix::operator[](int i) const
{
	if(i < 0 || i >= Nrows){
		cerr << "Matrix row index of " << i << " out of bounds" << endl;
		exit(1);
	}
	
	return (row[i]);
}

// Casts of Matrices into other forms of Matrices
Matrix2x2::operator Matrix3x3()
{
	Matrix3x3 m;
	int i, j;
	
	for(i = 0; i < 2; i++)
		for(j = 0; j < 2; j++)
			m[i][j] = row[i][j];
	
	m[2].z = 1.0;
	
	return m;
}

Matrix2x2::operator Matrix4x4()
{
	Matrix4x4 m;
	int i, j;
	
	for(i = 0; i < 2; i++)
		for(j = 0; j < 2; j++)
			m[i][j] = row[i][j];
	
	m[2].z = 1.0;
	m[3].w = 1.0;
	
	return m;
}

Matrix2x2::operator Matrix()
{
	Matrix m(2, 2);
	int i, j;
	
	for(i = 0; i < 2; i++)
		for(j = 0; j < 2; j++)
			m[i][j] = row[i][j];
	
	return m;
}

Matrix3x3::operator Matrix4x4()
{
	Matrix4x4 m;
	int i, j;
	
	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++)
			m[i][j] = row[i][j];
	
	m[3].w = 1.0;
	
	return m;
}

Matrix3x3::operator Matrix()
{
	Matrix m(3, 3);
	int i, j;
	
	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++)
			m[i][j] = row[i][j];
	
	return m;
}

Matrix4x4::operator Matrix()
{
	Matrix m(4, 4);
	int i, j;
	
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			m[i][j] = row[i][j];
	
	return m;
}

Matrix::operator Matrix2x2()
{
	if(Nrows != 2 || Ncols != 2){
		cerr << "cannot cast " << Nrows << " x " << Ncols << 
		" Matrix to Matrix2x2" << endl;
		exit(1);
	}
	
	Matrix2x2 m;
	int i, j;
	
	for(i = 0; i < 2; i++)
		for(j = 0; j < 2; j++)
			m[i][j] = row[i][j];
	
	return m;  
}

Matrix::operator Matrix3x3()
{
	if(Nrows != 3 || Ncols != 3){
		cerr << "cannot cast " << Nrows << " x " << Ncols << 
		" Matrix to Matrix3x3" << endl;
		exit(1);
	}
	
	Matrix3x3 m;
	int i, j;
	
	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++)
			m[i][j] = row[i][j];
	
	return m;  
}

Matrix::operator Matrix4x4()
{
	if(Nrows != 4 || Ncols != 4){
		cerr << "cannot cast " << Nrows << " x " << Ncols << 
		" Matrix to Matrix4x4" << endl;
		exit(1);
	}
	
	Matrix4x4 m;
	int i, j;
	
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			m[i][j] = row[i][j];
	
	return m;  
}

void Matrix2x2::print(int w, int p) const
{
	int i;
	
	for(i = 0; i < 2; i++){
		cout << setw(w) << setprecision(p) << Round(row[i].x, p) << " ";
		cout << setw(w) << setprecision(p) << Round(row[i].y, p);
		cout << endl;
	}
}

void Matrix3x3::print(int w, int p) const
{
	int i;
	
	for(i = 0; i < 3; i++){
		cout << setw(w) << setprecision(p) << Round(row[i].x, p) << " ";
		cout << setw(w) << setprecision(p) << Round(row[i].y, p) << " ";
		cout << setw(w) << setprecision(p) << Round(row[i].z, p);
		cout << endl;
	}
}

void Matrix4x4::print(int w, int p) const
{
	int i;
	
	for(i = 0; i < 4; i++){
		cout << setw(w) << setprecision(p) << Round(row[i].x, p) << " ";
		cout << setw(w) << setprecision(p) << Round(row[i].y, p) << " ";
		cout << setw(w) << setprecision(p) << Round(row[i].z, p) << " ";
		cout << setw(w) << setprecision(p) << Round(row[i].w, p);
		cout << endl;
	}
}

void Matrix::print(int w, int p) const
{
	int i, j;
	
	for(i = 0; i < Nrows; i++){
		for(j = 0; j < Ncols; j++)
			cout << setw(w) << setprecision(p) << Round(row[i][j], p) << " ";
		cout << endl;
	}
}

void Matrix2x2::set(double a11, double a12,
					double a21, double a22)
{
	row[0].set(a11, a12);
	row[1].set(a21, a22);
}

void Matrix3x3::set(double a11, double a12,
					double a21, double a22)
{
	row[0].set(a11, a12, 0);
	row[1].set(a21, a22, 0);
	row[2].set(0, 0, 1);
}

void Matrix3x3::set(double a11, double a12, double a13,
					double a21, double a22, double a23,
					double a31, double a32, double a33)
{
	row[0].set(a11, a12, a13);
	row[1].set(a21, a22, a23);
	row[2].set(a31, a32, a33);
}

void Matrix4x4::set(double a11, double a12, double a13,
					double a21, double a22, double a23,
					double a31, double a32, double a33)
{
	row[0].set(a11, a12, a13, 0);
	row[1].set(a21, a22, a23, 0);
	row[2].set(a31, a32, a33, 0);
	row[3].set(0, 0, 0, 1);
}

void Matrix4x4::set(double a11, double a12, double a13, double a14,
					double a21, double a22, double a23, double a24,
					double a31, double a32, double a33, double a34,
					double a41, double a42, double a43, double a44)
{
	row[0].set(a11, a12, a13, a14);
	row[1].set(a21, a22, a23, a24);
	row[2].set(a31, a32, a33, a34);
	row[3].set(a41, a42, a43, a44);
}

void Matrix::set(double *M)
{
	int i, j;
	
	for(i = 0; i < Nrows; i++)
		for(j = 0; j < Ncols; j++)
			row[i][j] = M[i * Ncols + j];
}

void Matrix::set(double a11, double a12,
				 double a21, double a22)
{
	if(Nrows != 2 || Ncols != 2){
		cerr << "2 x 2 set of " << Nrows << " x " << Ncols << " Matrix" << endl;
		exit(1);
	}
	row[0].set(a11, a12);
	row[1].set(a21, a22);
}

void Matrix::set(double a11, double a12, double a13,
				 double a21, double a22, double a23,
				 double a31, double a32, double a33)
{
	if(Nrows != 3 || Ncols != 3){
		cerr << "3 x 3 set of " << Nrows << " x " << Ncols << 
		" Matrix" << endl;
		exit(1);
	}
	row[0].set(a11, a12, a13);
	row[1].set(a21, a22, a23);
	row[2].set(a31, a32, a33);
}

void Matrix::set(double a11, double a12, double a13, double a14,
				 double a21, double a22, double a23, double a24,
				 double a31, double a32, double a33, double a34,
				 double a41, double a42, double a43, double a44)
{
	if(Nrows != 4 || Ncols != 4){
		cerr << "4 x 4 set of " << Nrows << " x " << Ncols << " Matrix" << endl;
		exit(1);
	}
	row[0].set(a11, a12, a13, a14);
	row[1].set(a21, a22, a23, a24);
	row[2].set(a31, a32, a33, a34);
	row[3].set(a41, a42, a43, a44);
}

void Matrix2x2::identity()
{
	set(1, 0, 0, 1);
}

void Matrix3x3::identity()
{
	set(1, 0, 0, 0, 1, 0, 0, 0, 1);
}

void Matrix4x4::identity()
{
	set(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
}

void Matrix::identity()
{
	int i, j;
	
	for(i = 0; i < Nrows; i++)
		for(j = 0; j < Ncols; j++)
			if(i == j)
				row[i][j] = 1;
			else
				row[i][j] = 0;
}

Matrix2x2 Matrix2x2::transpose() const
{
	Matrix2x2 transM;
	
	transM[0].x = row[0].x;
	transM[1].x = row[0].y;
	transM[0].y = row[1].x;
	transM[1].y = row[1].y;
	
	return transM;
}

Matrix3x3 Matrix3x3::transpose() const
{
	Matrix3x3 transM;
	
	transM[0].x = row[0].x;
	transM[1].x = row[0].y;
	transM[2].x = row[0].z;
	transM[0].y = row[1].x;
	transM[1].y = row[1].y;
	transM[2].y = row[1].z;
	transM[0].z = row[2].x;
	transM[1].z = row[2].y;
	transM[2].z = row[2].z;
	
	return transM;
}

Matrix4x4 Matrix4x4::transpose() const
{
	Matrix4x4 transM;
	
	transM[0].x = row[0].x;
	transM[1].x = row[0].y;
	transM[2].x = row[0].z;
	transM[3].x = row[0].w;
	transM[0].y = row[1].x;
	transM[1].y = row[1].y;
	transM[2].y = row[1].z;
	transM[3].y = row[1].w;
	transM[0].z = row[2].x;
	transM[1].z = row[2].y;
	transM[2].z = row[2].z;
	transM[3].z = row[2].w;
	transM[0].w = row[3].x;
	transM[1].w = row[3].y;
	transM[2].w = row[3].z;
	transM[3].w = row[3].w;
	
	return transM;
}

Matrix Matrix::transpose() const
{
	Matrix transM(Ncols, Nrows);
	int i, j;
	
	for(i = 0; i < Nrows; i++)
		for(j = 0; j < Ncols; j++)
			transM[j][i] = row[i][j];
	
	return transM;
}

//////////////////////////////////////////////////////////////////////////
// the following matrix operations are used to find the inverse of an 
// NxN matrix. Adapted from Numerical Recipes by (Frank) Sebastian Grassia
//////////////////////////////////////////////////////////////////////////

Matrix2x2 Matrix2x2::inv() const
{
	Matrix2x2 invM;
	double d;
	
	d = row[0].x*row[1].y - row[0].y*row[1].x;
	
	if(d == 0.0)
		cerr << "inverse of singular Matrix2x2" << endl;
	
	invM[0].x = row[1].y / d;
	invM[0].y = -row[0].y / d;
	invM[1].x = -row[1].x / d;
	invM[1].y = row[0].x / d;
	
	return invM;
}

Matrix3x3 Matrix3x3::inv() const
{
	Matrix3x3 invM;
	double d;
	
	d = row[0].x*row[1].y*row[2].z + row[0].y*row[1].z*row[2].x +
    row[0].z*row[2].y*row[1].x - row[0].z*row[1].y*row[2].x - 
    row[0].y*row[1].x*row[2].z - row[0].x*row[2].y*row[1].z;
	
	if(d == 0.0)
		cerr << "inverse of singular Matrix3x3" << endl;
	
	invM[0].x = (row[1].y*row[2].z - row[1].z*row[2].y) / d;
	invM[0].y = (row[0].z*row[2].y - row[0].y*row[2].z) / d;
	invM[0].z = (row[0].y*row[1].z - row[0].z*row[1].y) / d;
	invM[1].x = (row[1].z*row[2].x - row[1].x*row[2].z) / d;
	invM[1].y = (row[0].x*row[2].z - row[0].z*row[2].x) / d;
	invM[1].z = (row[0].z*row[1].x - row[0].x*row[1].z) / d;
	invM[2].x = (row[1].x*row[2].y - row[1].y*row[2].x) / d;
	invM[2].y = (row[0].y*row[2].x - row[0].x*row[2].y) / d;
	invM[2].z = (row[0].x*row[1].y - row[0].y*row[1].x) / d;
	
	return invM;
}

Matrix4x4 LU_Decompose(const Matrix4x4& M, int *indx)
{
	int  i, imax, j, k;
	double big, dum, sum, temp;
	double vv[4];
	Matrix4x4 LU_M = M;
	int N = 4;
	
	for(i = 0; i < N; i++){
		big = 0.0;
		for(j = 0; j < N; j++)
			if((temp = fabs(M.row[i][j])) > big)
				big = temp;
		if(big == 0.0)
			cerr << "inverse of singular Matrix4x4" << endl;
		
		vv[i] = 1.0 / big;
	}
	
	for(j = 0; j < N; j++){
		for(i = 0; i < j; i++){
			sum = LU_M[i][j];
			for(k = 0; k < i; k++)
				sum -= LU_M[i][k] * LU_M[k][j];
			LU_M[i][j] = sum;
		}
		big = 0.0;
		for(i = j; i < N; i++){
			sum = LU_M[i][j];
			for(k = 0; k < j; k++)
				sum -= LU_M[i][k] * LU_M[k][j];
			LU_M[i][j] = sum;
			if((dum = vv[i]*fabs(sum)) >= big){
				big = dum;
				imax = i;
			}
		}
		if(j != imax){
			for(k = 0; k < N; k++){
				dum = LU_M[imax][k];
				LU_M[imax][k] = LU_M[j][k];
				LU_M[j][k] = dum;
			}
			vv[imax] = vv[j];
		}
		indx[j] = imax;
		if(j < N - 1){
			dum = 1.0 / LU_M[j][j];
			for(i = j + 1; i < N; i++)
				LU_M[i][j] *= dum;
		}
	}
	
	return LU_M;
}

void LU_back_substitution(const Matrix4x4& M, int *indx, double col[])
{
	int i, ii = -1, ip,j;
	double sum;
	int N = 4;
	
	for(i = 0; i < N; i++){
		ip = indx[i];
		sum = col[ip];
		col[ip] = col[i];
		if(ii >= 0)
			for(j = ii; j < i; j++)
				sum -= M.row[i][j] * col[j];
		else if(sum)
			ii = i;
		col[i] = sum;
	}
	
	for(i = N - 1; i >= 0; i--){
		sum = col[i];
		for(j = i + 1; j < N; j++)
			sum -= M.row[i][j] * col[j];
		col[i] = sum / M.row[i][i];
	}
}

Matrix4x4 Matrix4x4::inv() const
{
	Matrix4x4 LU_M, invM;
	int i, j, indx[4];
	double col[4];
	
	LU_M = LU_Decompose(*this, indx);
	
	for(j = 0; j < 4; j++){
		for(i = 0; i < 4; i++)
			col[i] = 0.0;
		col[j] = 1.0;
		LU_back_substitution(LU_M, indx, col);
		for(i = 0; i < 4; i++)
			invM[i][j] = col[i];
	}
	
	return invM;
}

Matrix LU_Decompose(const Matrix& M, int *indx)
{
	int  i, imax, j, k;
	double big, dum, sum, temp;
	double *vv = new double[M.Nrows];
	Matrix LU_M(M);
	int N = M.Nrows;
	
	for(i = 0; i < N; i++){
		big = 0.0;
		for(j = 0; j < N; j++)
			if((temp = fabs(M[i][j])) > big)
				big = temp;
		if(big == 0.0)
			cerr << "inverse of singular " << M.Nrows << " x " << M.Ncols << 
			" Matrix" << endl;
		
		vv[i] = 1.0 / big;
	}
	
	for(j = 0; j < N; j++){
		for(i = 0; i < j; i++){
			sum = LU_M[i][j];
			for(k = 0; k < i; k++)
				sum -= LU_M[i][k] * LU_M[k][j];
			LU_M[i][j] = sum;
		}
		big = 0.0;
		for(i = j; i < N; i++){
			sum = LU_M[i][j];
			for(k = 0; k < j; k++)
				sum -= LU_M[i][k] * LU_M[k][j];
			LU_M[i][j] = sum;
			if((dum = vv[i]*fabs(sum)) >= big){
				big = dum;
				imax = i;
			}
		}
		if(j != imax){
			for(k = 0; k < N; k++){
				dum = LU_M[imax][k];
				LU_M[imax][k] = LU_M[j][k];
				LU_M[j][k] = dum;
			}
			vv[imax] = vv[j];
		}
		indx[j] = imax;
		if(j < N - 1){
			dum = 1.0 / LU_M[j][j];
			for(i = j + 1; i < N; i++)
				LU_M[i][j] *= dum;
		}
	}
	
	delete vv;
	return LU_M;
}

void LU_back_substitution(const Matrix& M, int *indx, double col[])
{
	int i, ii = -1, ip,j;
	double sum;
	int N = M.Nrows;
	
	for(i = 0; i < N; i++){
		ip = indx[i];
		sum = col[ip];
		col[ip] = col[i];
		if(ii >= 0)
			for(j = ii; j < i; j++)
				sum -= M.row[i][j] * col[j];
		else if(sum)
			ii = i;
		col[i] = sum;
	}
	
	for(i = N - 1; i >= 0; i--){
		sum = col[i];
		for(j = i + 1; j < N; j++)
			sum -= M.row[i][j] * col[j];
		col[i] = sum / M.row[i][i];
	}
}

Matrix Matrix::inv() const
{
	if(Nrows != Ncols){
		cerr << "cannot invert non-square " << Nrows << " x " << Ncols << 
		" Matrix" << endl;
		exit(1);
	}
	
	Matrix LU_M(Nrows, Nrows);
	Matrix invM(Nrows, Nrows);
	int i, j;
	int *indx = new int[Nrows];
	double *col = new double[Nrows];
	
	LU_M = LU_Decompose(*this, indx);
	
	for(j = 0; j < Nrows; j++){
		for(i = 0; i < Nrows; i++)
			col[i] = 0.0;
		col[j] = 1.0;
		LU_back_substitution(LU_M, indx, col);
		for(i = 0; i < Nrows; i++)
			invM[i][j] = col[i];
	}
	
	delete indx;
	delete col;
	return invM;
}

//
// Singular Value Decomposition of a Matrix: M = U W V^  (V^ is V transpose)
//
// U orthogonal matrix whose columns, for which wi != 0, span range of M
// W diagonal matrix of singular values (wi = 0 denotes a singularity)
// V orthogonal matrix whose columns, for which wi = 0, span null space of M
// M is (m x n), U is (m x n), W is (n x n), V is (n x n)
//
// U and V updated and returned as matrices
// W is updated and returned as a vector, since W is diagonal
//
void Matrix::svd(Matrix &U, Vector &W, Matrix &V) const
{
	int flag, i, its, j, jj, k, l, nm;
	double anorm, c, f, g, h, s, scale, x, y, z;
	
	int m = nrows();
	int n = ncols();
	
	// copy this matrix to U
	U = *this;
	
	// make sure that the other output matrices are the right size
	W.setsize(n);
	V.setsize(n, n);
	
	// create working vector
	Vector rv1(n);
	
	// Householder reduction to bidiagonal form
	g = scale = anorm = 0;
	
	for(i = 0; i < n; i++){
		l = i + 1;
		rv1[i] = scale * g;
		g = s = scale = 0;
		
		if(i < m){
			for(k = i; k < m; k++)
				scale += fabs(U[k][i]);
			if(scale != 0){
				for(k = i; k < m; k++){
					U[k][i] /= scale;
					s += Sqr(U[k][i]);
				}
				f = U[i][i];
				g = -ApplySign(sqrt(s), f);
				h = f * g - s;
				U[i][i] = f - g;
				for(j = l; j < n; j++){
					for(s = 0, k = i; k < m; k++)
						s += U[k][i] * U[k][j];
					f = s / h;
					for(k = i; k < m; k++)
						U[k][j] += f * U[k][i];
				}
				for(k = i; k < m; k++)
					U[k][i] = U[k][i] * scale;
			}
		}
		
		W[i] = scale * g;
		g = s = scale = 0;
		
		if(i < m && i != (n - 1)){
			for(k = l; k < n; k++)
				scale += fabs(U[i][k]);
			if(scale != 0){
				for(k = l; k < n; k++){
					U[i][k] /= scale;
					s += Sqr(U[i][k]);
				}
				f = U[i][l];
				g = -ApplySign(sqrt(s), f);
				h = f * g - s;
				U[i][l] = f - g;
				for(k = l; k < n; k++)
					rv1[k] = U[i][k] / h;
				for(j = l; j < m; j++){
					for(s = 0, k = l; k < n; k++)
						s += U[j][k] * U[i][k];
					for(k = l; k < n; k++)
						U[j][k] += s * rv1[k];
				}
				for(k = l; k < n; k++)
					U[i][k] *= scale;
			}
		}
		anorm = Max(anorm, (fabs(W[i]) + fabs(rv1[i])));
	}
	
	// accumulation of hight-hand transformation
	for(i = n - 1; i >= 0; i--){
		if(i < n - 1){
			if(g != 0){
				for(j = l; j < n; j++)
					V[j][i] = (U[i][j] / U[i][l]) / g;
				for(j = l; j < n; j++){
					for(s = 0, k = l; k < n; k++)
						s += U[i][k] * V[k][j];
					for(k = l; k < n; k++)
						V[k][j] += s * V[k][i];
				}
			}
			for(j = l; j < n; j++)
				V[i][j] = V[j][i] = 0;
		}
		V[i][i] = 1;
		g = rv1[i];
		l = i;
	}
	
	// accumulation of left-hand transformation
	for(i = Min(m, n) - 1; i >= 0; i--){
		l = i + 1;
		g = W[i];
		for(j = l; j < n; j++)
			U[i][j] = 0;
		if(g != 0){
			g = 1 / g;
			for(j = l; j < n; j++){
				for(s = 0, k = l; k < m; k++)
					s += U[k][i] * U[k][j];
				f = (s / U[i][i]) * g;
				for(k = i; k < m; k++)
					U[k][j] = U[k][j] + f * U[k][i];
			}
			for(j = i; j < m; j++)
				U[j][i] *= g;
		}
		else
			for(j = i; j < m; j++)
				U[j][i] = 0;
		U[i][i]++;
	}
	
	// diagonalization of the bidiagonal form: loop over singular values,
	// and over allowed iterations
	for(k = n - 1; k >= 0; k--){
		for(its = 1; its <= 30; its++){
			flag = 1;
			for(l = k; l >= 0; l--){		// test for splitting
				nm = l - 1;			// note that rv1[0] is always zero
				if(double(fabs(rv1[l]) + anorm) == anorm){
					flag = 0;
					break;
				}
				if(double(fabs(W[nm]) + anorm) == anorm)
					break;
			}
			if(flag){
				c =  0;
				s = 1;
				for(i = l; i <= k; i++){
					f = s * rv1[i];
					rv1[i] = c * rv1[i];
					if(double(fabs(f) + anorm) == anorm)
						break;
					g = W[i];
					h = pythag(f, g);
					W[i] = h;
					h = 1 / h;
					c = g * h;
					s = -f * h;
					for(j = 0; j < m; j++){
						y = U[j][nm];
						z = U[j][i];
						U[j][nm] = y * c + z * s;
						U[j][i] = z * c - y * s;
					}
				}
			}
			z = W[k];
			
			if(l == k){		// convergence
				if(z < 0){		// singular value is made nonnegative
					W[k] = -z;
					for(j = 0; j < n; j++)
						V[j][k] = - V[j][k];
				}
				break;
			}
			
			if(its == 30)
				abort("no convergence in 30 svdcmp iterations");
			
			x = W[l];
			nm = k - 1;
			y = W[nm];
			g = rv1[nm];
			h = rv1[k];
			f = ((y - z) * ( y + z) + (g - h) * (g + h)) / (2 * h * y);
			g = pythag(f, 1);
			f = ((x- z) * (x + z) + h * ((y / (f + ApplySign(g, f))) - h)) / x;
			c = s = 1;
			for(j = l; j <= nm; j++){
				i = j + 1;
				g = rv1[i];
				y = W[i];
				h = s * g;
				g = c * g;
				z = pythag(f, h);
				rv1[j] = z;
				c = f / z;
				s = h / z;
				f = x * c + g * s;
				g = g * c - x * s;
				h = y * s;
				y *= c;
				for(jj = 0; jj < n; jj++){
					x = V[jj][j];
					z = V[jj][i];
					V[jj][j] = x * c + z * s;
					V[jj][i] = z * c - x * s;
				}
				z = pythag(f, h);
				
				W[j] = z;		// rotation can be arbitrary if z = 0
				
				if(z != 0){
					z = 1 / z;
					c = f * z;
					s = h * z;
				}
				
				f = c * g + s * y;
				x = c * y - s * g;
				
				for(jj = 0; jj < m; jj++){
					y = U[jj][j];
					z = U[jj][i];
					U[jj][j] = y * c + z * s;
					U[jj][i] = z * c - y * s;
				}
			}
			rv1[l] = 0;
			rv1[k] = f;
			W[k] = x;
		}
	}
}

// returns a diagonal matrix with diagonal elements from the input vector
// note: this is a friend of Matrix class rather than being a method of 
// Vector class, since we wanted to maintain the independence of Vector 
// from Matrix. This is so that programs that need vectors but not 
// matrices need not include Matrix.h
Matrix diag(const Vector &V)
{
	int i;
	int n = V.getn();
	Matrix d(n, n);
	
	for(i = 0; i < n; i++)
		d[i][i] = V[i];
	
	return d;
}

/* Matrix op Matrix operations */

Matrix2x2 operator+(const Matrix2x2& m1, const Matrix2x2& m2)
{
	int i;
	Matrix2x2 result;
	
	for(i = 0; i < 2; i++){
		result.row[i].x = m1.row[i].x + m2.row[i].x;
		result.row[i].y = m1.row[i].y + m2.row[i].y;
	}
	
	return result;
}

Matrix2x2 operator-(const Matrix2x2& m1, const Matrix2x2& m2)
{
	int i;
	Matrix2x2 result;
	
	for(i = 0; i < 2; i++){
		result.row[i].x = m1.row[i].x - m2.row[i].x;
		result.row[i].y = m1.row[i].y - m2.row[i].y;
	}
	
	return result;
}

Matrix2x2 operator*(double a, const Matrix2x2& m)
{
	int i;
	Matrix2x2 result;
	
	for(i = 0; i < 2; i++){
		result.row[i].x = a * m.row[i].x;
		result.row[i].y = a * m.row[i].y;
	}
	
	return result;
}

Matrix2x2 Matrix2x2::operator*(double a) const
{
	int i;
	Matrix2x2 result;
	
	for(i = 0; i < 2; i++){
		result.row[i].x = a * row[i].x;
		result.row[i].y = a * row[i].y;
	}
	
	return result;
}

ostream& operator<< (ostream& os, const Matrix2x2& m){
	int i, j;
	
	for(i = 0; i < 2; i++){
		for(j = 0; j < 2; j++)
			os << setw(7) << setprecision(3) << Round(m.row[i][j], 3) << " ";
		os << endl;
	}
	
	return os;
}


Matrix2x2 operator*(const Matrix2x2& m1, const Matrix2x2& m2)
{
	int i, j, rc;
	Matrix2x2 result;
	
	for(i = 0; i < 2; i++)
		for(j = 0; j < 2; j++){
			result.row[i][j] = 0;
			for(rc = 0; rc < 2; rc++)
				result.row[i][j] += m1.row[i][rc] * m2.row[rc][j];
		}
	
	return result;
}

Matrix3x3 operator+(const Matrix3x3& m1, const Matrix3x3& m2)
{
	int i, j;
	Matrix3x3 result;
	
	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++)
			result.row[i][j] = m1.row[i][j] + m2.row[i][j];
	
	return result;
}

Matrix3x3 operator-(const Matrix3x3& m1, const Matrix3x3& m2)
{
	int i, j;
	Matrix3x3 result;
	
	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++)
			result.row[i][j] = m1.row[i][j] - m2.row[i][j];
	
	return result;
}

Matrix3x3 operator*(double a, const Matrix3x3& m)
{
	Matrix3x3 result;
	int i, j;
	
	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++)
			result.row[i][j] = a * m.row[i][j];
	
	return result;
}

Matrix3x3 Matrix3x3::operator*(double a) const
{
	Matrix3x3 result;
	int i, j;
	
	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++)
			result[i][j] = a * row[i][j];
	
	return result;
}

ostream& operator<< (ostream& os, const Matrix3x3& m){
	int i, j;
	
	for(i = 0; i < 3; i++){
		for(j = 0; j < 3; j++)
			os << setw(7) << setprecision(3) << Round(m.row[i][j], 3) << " ";
		os << endl;
	}
	
	return os;
}


Matrix3x3 operator*(const Matrix3x3& m1, const Matrix3x3& m2)
{
	int i, j, rc;
	Matrix3x3 result;
	
	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++){
			result.row[i][j] = 0;
			for(rc = 0; rc < 3; rc++)
				result.row[i][j] += m1.row[i][rc] * m2[rc][j];
		}
	
	return result;
}

Matrix4x4 operator+(const Matrix4x4& m1, const Matrix4x4& m2)
{
	int i, j;
	Matrix4x4 result;
	
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			result.row[i][j] = m1.row[i][j] + m2.row[i][j];
	
	return result;
}

Matrix4x4 operator-(const Matrix4x4& m1, const Matrix4x4& m2)
{
	int i, j;
	Matrix4x4 result;
	
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			result.row[i][j] = m1.row[i][j] - m2.row[i][j];
	
	return result;
}

Matrix4x4 operator*(double a, const Matrix4x4& m)
{
	Matrix4x4 result;
	
	int i, j;
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			result.row[i][j] = a * m.row[i][j];
	
	return result;
}

Matrix4x4 Matrix4x4::operator*(double a) const
{
	Matrix4x4 result;
	
	int i, j;
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			result[i][j] = a * row[i][j];
	
	return result;
}

ostream& operator<< (ostream& os, const Matrix4x4& m){
	int i, j;
	
	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++)
			os << setw(7) << setprecision(3) << Round(m.row[i][j], 3) << " ";
		os << endl;
	}
	
	return os;
}

Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2)
{
	int i, j, rc;
	Matrix4x4 result;
	
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++){
			result.row[i][j] = 0;
			for(rc = 0; rc < 4; rc++)
				result.row[i][j] += m1.row[i][rc] * m2[rc][j];
		}
	
	return result;
}

Matrix operator+(const Matrix& m1, const Matrix& m2)
{
	if(m1.Nrows != m2.Nrows || m1.Ncols != m2.Ncols){
		cerr << "cannot add " << m1.Nrows << " x " << m1.Ncols << 
		" Matrix to " << m2.Nrows << " x " << m2.Ncols << " Matrix" << endl;
		exit(1);
	}
	Matrix result(m1.Nrows, m1.Ncols);
	
	int i, j;
	for(i = 0; i < m1.Nrows; i++)
		for(j = 0; j < m1.Ncols; j++)
			result.row[i][j] = m1.row[i][j] + m2.row[i][j];
	
	return result;
}

Matrix operator-(const Matrix& m1, const Matrix& m2)
{
	if(m1.Nrows != m2.Nrows || m1.Ncols != m2.Ncols){
		cerr << "cannot subtract " << m2.Nrows << " x " << m2.Ncols << " Matrix from " << m1.Nrows << " x " << m1.Ncols << " Matrix" << endl;
		exit(1);
	}
	Matrix result(m1.Nrows, m1.Ncols);
	
	int i, j;
	for(i = 0; i < m1.Nrows; i++)
		for(j = 0; j < m1.Ncols; j++)
			result.row[i][j] = m1.row[i][j] - m2.row[i][j];
	
	return result;
}

Matrix operator*(double a, const Matrix& m)
{
	Matrix result(m.Nrows, m.Ncols);
	
	int i, j;
	for(i = 0; i < m.Nrows; i++)
		for(j = 0; j < m.Ncols; j++)
			result.row[i][j] = a * m.row[i][j];
	
	return result;
}

Matrix Matrix::operator*(double a) const
{
	Matrix result(Nrows, Ncols);
	
	int i, j;
	for(i = 0; i < Nrows; i++)
		for(j = 0; j < Ncols; j++)
			result[i][j] = a * row[i][j];
	
	return result;
}

ostream& operator<< (ostream& os, const Matrix& m){
	int i, j;
	
	for(i = 0; i < m.Nrows; i++){
		for(j = 0; j < m.Ncols; j++)
			os << setw(7) << setprecision(3) << Round(m.row[i][j], 3) << " ";
		os << endl;
	}
	
	return os;
}

Matrix operator*(const Matrix& m1, const Matrix& m2)
{
	if(m1.Ncols != m2.Nrows){
		cerr << "cannot multiply " << m2.Nrows << " x " << m2.Ncols << 
		" Matrix by " << m1.Nrows << " x " << m1.Ncols << " Matrix" << endl;
		exit(1);
	}
	Matrix result(m1.Nrows, m2.Ncols);
	
	int i, j, rc;
	for(i = 0; i < m1.Nrows; i++)
		for(j = 0; j < m2.Ncols; j++){
			result.row[i][j] = 0;
			for(rc = 0; rc < m1.Ncols; rc++)
				result.row[i][j] += m1.row[i][rc] * m2[rc][j];
		}
	
	return result;
}

// Assignment
const Matrix& Matrix::operator=(const Matrix& m2)
{
	int i, j;
	
	if(Nrows != m2.Nrows || Ncols != m2.Ncols){
		delete []row;
		setsize(m2.Nrows, m2.Ncols);
	}
	
	for(i = 0; i < Nrows; i++)
		for(j = 0; j < Ncols; j++)
			(row[i])[j] = (m2.row[i])[j];
	
	return (*this);
}

/* Matrix-Vector Operations */

Vector2d operator*(const Matrix2x2& m, const Vector2d& v)
{
	int i, j;
	double sum;
	Vector2d result;
	
	for(i = 0; i < 2; i++){
		sum = 0;
		for(j = 0; j < 2; j++)
			sum += m.row[i][j] * v[j];
		result[i] = sum;
	}
	
	return result;
}

Vector3d operator*(const Matrix3x3& m, const Vector3d& v)
{
	int i, j;
	double sum;
	Vector3d result;
	
	for(i = 0; i < 3; i++){
		sum = 0;
		for(j = 0; j < 3; j++)
			sum += m.row[i][j] * v[j];
		result[i] = sum;
	}
	
	return result;
}

Vector4d operator*(const Matrix4x4& m, const Vector4d& v)
{
	int i, j;
	double sum;
	Vector4d result;
	
	for(i = 0; i < 4; i++){
		sum = 0;
		for(j = 0; j < 4; j++)
			sum += m.row[i][j] * v[j];
		result[i] = sum;
	}
	
	return result;
}

Vector operator*(const Matrix& M, const Vector& V)
{
	if(M.Ncols != V.getn()){
		cerr << "multiply of " << M.Nrows << " x " << M.Ncols << 
		" Matrix by " << V.getn() << " Vector" << endl;
		exit(1);
	}
	
	int i, j;
	double sum;
	Vector result(M.Nrows);
	
	for(i = 0; i < M.Nrows; i++){
		sum = 0;
		for(j = 0; j < M.Ncols; j++)
			sum += M[i][j] * V[j];
		result[i] = sum;
	}
	
	return result;
}

Vector2d operator*(const Vector2d& v, const Matrix2x2& m)
{
	int i, j;
	double sum;
	Vector2d result;
	
	for(j = 0; j < 2; j++){
		sum = 0;
		for(i = 0; i < 2; i++)
			sum += v[i] * m.row[i][j];
		result[j] = sum;
	}
	
	return result;
}

Vector3d operator*(const Vector3d& v, const Matrix3x3& m)
{
	int i, j;
	double sum;
	Vector3d result;
	
	for(j = 0; j < 3; j++){
		sum = 0;
		for(i = 0; i < 3; i++)
			sum += v[i] * m.row[i][j];
		result[j] = sum;
	}
	
	return result;
}

Vector4d operator*(const Vector4d& v, const Matrix4x4& m)
{
	int i, j;
	double sum;
	Vector4d result;
	
	for(j = 0; j < 4; j++){
		sum = 0;
		for(i = 0; i < 4; i++)
			sum += v[i] * m.row[i][j];
		result[j] = sum;
	}
	
	return result;
}

Vector operator*(const Vector& V, const Matrix& M)
{
	if(M.Nrows != V.getn()){
		cerr << "multiply of " << V.getn() << " Vector by " << M.Nrows << 
		" x " << M.Ncols << " Matrix" << endl;
		exit(1);
	}
	
	int i, j;
	double sum;
	Vector result(M.Nrows);
	
	for(j = 0; j < M.Ncols; j++){
		sum = 0;
		for(i = 0; i < M.Nrows; i++)
			sum += M[i][j] * V[i];
		result[j] = sum;
	}
	
	return result;
}

// Outer product of v1, v2 (i.e. v1 times v2 transpose)
Matrix2x2 operator&(const Vector2d& v1, const Vector2d& v2)
{
	int i, j;
	Matrix2x2 product;
	
	for(i = 0; i < 2; i++)
		for(j = 0; j < 2; j++)
			product.row[i][j] = v1[i] * v2[j];
	
	return product;
}
Matrix3x3 operator&(const Vector3d& v1, const Vector3d& v2)
{
	int i, j;
	Matrix3x3 product;
	
	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++)
			product.row[i][j] = v1[i] * v2[j];
	
	return product;
}
Matrix operator&(const Vector& v1, const Vector& v2)
{
	int N = v1.getn();
	if(N != v2.getn()){
		cerr << "cannot do outer product of " << N << " Vector with " <<
		v2.getn() << " Vector" << endl;
		exit(1);
	}
	
	int i, j;
	Matrix product(N, N);
	
	for(i = 0; i < N; i++)
		for(j = 0; j < N; j++)
			product.row[i][j] = v1[i] * v2[j];
	
	return product;
}
