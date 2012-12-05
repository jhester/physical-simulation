//
// Camera.cpp
// OpenGL Camera Class
// 
// Christopher Root, 2006
// Modifications by Donald House, 2009
//   corrected calculation of camera coordinates
//
//

#include "Camera.h"

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

using namespace std;

double DeltaAzim;
double DeltaElev;
double LocalDeltaAzim;
double LocalDeltaElev;

int MouseStartX;
int MouseStartY;
int MousePrevX;
int MousePrevY;

const float epsilon = 0.0001;

GLdouble MvMatrix[16];
GLdouble ProjMatrix[16];
GLint ViewPort[4];

int CameraMode = INACTIVE;

Vector3d PrevMousePos;

//
// Rotation routines - regular functions, not methods
//

void RotateX(Vector3d &v, double degree){
  double c = cos(DegToRad(degree));
  double s = sin(DegToRad(degree));
  double v1 = v[1] * c - v[2] * s;
  double v2 = v[1] * s + v[2] * c;
  v[1] = v1; v[2] = v2;
}

void RotateY(Vector3d &v, double degree){
  double c = cos(DegToRad(degree));
  double s = sin(DegToRad(degree));
  double v0 = v[0] * c + v[2] * s;
  double v2 = -v[0] * s + v[2] * c;
  v[0] = v0; v[2] = v2;
}

/* 
 * ArbitraryRotate() - rotate around an arbitrary coordinate system specified by
 *                     U, V, & W
 */
void ArbitraryRotate(Vector3d U, Vector3d V, Vector3d W, double degreeX, double degreeY, Vector3d& point, Vector3d aim) {
  double cx = cos(DegToRad(degreeX));
  double sx = sin(DegToRad(degreeX));
  double cy = cos(DegToRad(degreeY));
  double sy = sin(DegToRad(degreeY)); 
  
  Matrix4x4 trans(1, 0, 0, -aim[0],
		  0, 1, 0, -aim[1],
		  0, 0, 1, -aim[2],
		  0, 0, 0, 1);
  
  Matrix4x4 mat(U[0], U[1], U[2], 0,
		V[0], V[1], V[2], 0,
		W[0], W[1], W[2], 0,
		0, 0, 0, 1);
  
  Matrix4x4 rot;
  Vector4d pos(point[0], point[1], point[2], 1);
  
  pos = trans*pos;
  
  pos = mat*pos;
  
  rot.set(1,   0,  0, 0,
	  0,  cx, sx, 0,
	  0, -sx, cx, 0,
	  0,   0,  0, 1);
  
  pos = rot*pos;
  
  rot.set( cy, 0, sy, 0,
	  0, 1,  0, 0,
	  -sy, 0, cy, 0,
	  0, 0,  0, 1);
  
  pos = rot*pos;
  
  pos = mat.inv()*pos;
  
  pos = trans.inv()*pos;
  
  point.set(pos[0], pos[1], pos[2]);
}

//
// Private Helper Methods
//

// set camera position, aim point, and up vector. verify ok, and make up the true up direction
void Camera::ComputeCoords(const Vector3d &P, const Vector3d &A, const Vector3d &U){
  Vector3d zaxis = P - A;
  // if camera positoin and aim position coincident, no way to aim the camera
  if (zaxis.norm() < epsilon) {
    fprintf (stderr, "Camera position and aim position the same. Can't aim camera!\n");
    exit(1);
  }
  Vector3d dir = -zaxis.normalize();
  
  Vector3d up = U.normalize();
  Vector3d xaxis = dir % up;

  // if up vector and aim vector parallel, no way to tell which way is up
  if (xaxis.norm() < epsilon) {
    fprintf (stderr, "Up parallel to aim. Can't tell which way is up!\n");
    exit(1);
  }

  Pos = P;
  Aim = A;
  Up = xaxis.normalize() % dir; // correct up vector to be perpendicular to dir
}

// Initialize routine setting up defaults
void Camera::Initialize() {
  Vector3d tmp, tmp1, tmp2;
  Vector3d axisOrigin, updatePos;
  double dist;
  
  DefaultPos = Pos;
  DefaultAim = Aim;
  DefaultUp = Up;
  
  // find the angle around the x axis 
  updatePos = Pos - Aim;
  axisOrigin.set(updatePos.x, 0, 0);
  dist = (axisOrigin-updatePos).norm();
  tmp1.set(updatePos.x, 0, dist);
  
  tmp = updatePos.normalize();
  tmp1 = tmp1.normalize();
  
  CurrentElev = RadToDeg(acos(tmp * tmp1));
  
  // find the angle around the y axis
  axisOrigin.set(0, updatePos.y, 0);
  dist = (axisOrigin-updatePos).norm();
  
  tmp2.set(0, updatePos.y, dist);
  tmp2 = tmp2.normalize();
  
  CurrentAzim = 360.0 - RadToDeg(acos(tmp2*tmp));
  
  DefaultElev = CurrentElev;
  DefaultAzim = CurrentAzim;
}

/* constructors */

// default constructor... sets position to 0, 0, 5, aimed at the origin
// with the up vector set to the y axis
Camera::Camera() {
  Pos.set(0, 0, 5);
  Aim.set(0, 0, 0);
  Up.set(0, 1, 0);
  
  // set default view volume
  NearPlane = 0.1;
  FarPlane = 1000.0;
  Fov = 60.0;
  
  Initialize();
}

/* 
 * constructor to set a camera to a desired orientation
 * P is position in 3D
 * A is the aim coordinate
 * U is the up vector 
 */
Camera::Camera(Vector3d P, Vector3d A, Vector3d U) {
  ComputeCoords(P, A, U);
  
  // set default view volume
  NearPlane = 0.1;
  FarPlane = 1000.0;
  Fov = 60.0;
  
  Initialize();
}

/*
 * Constructor setting up all values
 */
Camera::Camera(Vector3d P, Vector3d A, Vector3d U, 
	       float Near, float Far, float ViewAngle) {
  ComputeCoords(P, A, U);
  
  NearPlane = Near;
  FarPlane = Far;
  Fov = ViewAngle;

  Initialize();
}

// set functions for the Pos, Aim, and Up vectors....
// just remember that |Aim - Pos| != 0, and (Aim - Pos) % Up != 0, and or you'll see problems
void Camera::SetPos(Vector3d P) {
  ComputeCoords(P, Aim, Up);
}

void Camera::SetAim(Vector3d A) {
  ComputeCoords(Pos, A, Up);
}

void Camera::SetUp(Vector3d U) {
  ComputeCoords(Pos, Aim, U);
}

/*
 * sets the near and far clipping planes for the camera view
 */
void Camera::SetClippingPlanes(float Near, float Far) {
  NearPlane = Near;
  FarPlane = Far;
}

/*
 * sets the field of view of the camera, ViewAngle is in degrees
 */
void Camera::SetFOV(float ViewAngle) {
  Fov = ViewAngle;
}

/*
 * resets the camera to its original orientation
 */
void Camera::Reset() {
  Pos = DefaultPos;
  Aim = DefaultAim;
  Up = DefaultUp;
  
  CurrentElev = DefaultElev;
  CurrentAzim = DefaultAzim;
}

/*
 * sets the camera's aim to be the given vector v
 */
void Camera::SetCenterOfFocus(Vector3d NewAim) {
  Vector3d dif = NewAim - Aim;
  ComputeCoords(Pos + dif, NewAim, Up);
}

/*
 * draws an opengl window with the camera orientation
 * W and H are the width and height of the window respectively
 */
void Camera::PerspectiveDisplay(int W, int H) {
  
  // set up the projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  gluPerspective(Fov, (float) W/(float) H, NearPlane, FarPlane);
  gluLookAt(Pos.x, Pos.y, Pos.z, 
	    Aim.x, Aim.y, Aim.z,
	    Up.x, Up.y, Up.z);
  
}

/*
 * mouse event handler function... should be called in the 
 * mouse event handler function of your own code
 */
void Camera::HandleMouseEvent(int button, int state, int x, int y) {
  double realy, wx, wy, wz;
  
  // check to see if the ALT key has been used
  int mode = glutGetModifiers();
  
  if (state == GLUT_UP && CameraMode != INACTIVE) {
    // update the elevation and roll of the camera
    CurrentElev += DeltaElev;
    CurrentAzim += DeltaAzim;
    
    //printf("%f %f\n", CurrentElev, CurrentAzim);
    
    // reset the change in elevation and roll of the camera
    DeltaElev = DeltaAzim = 0.0;
    
    CameraMode = INACTIVE;
  } else if (state == GLUT_DOWN){ // && mode == GLUT_ACTIVE_ALT) {
    
    // set the new mouse state
    MouseStartX = MousePrevX = x;
    MouseStartY = MousePrevY = y;
    
    // alt key and mouse button have been pressed, camera will move
    
    switch (button) {
      case GLUT_LEFT_BUTTON:
	// rotating camera
	CameraMode = ROTATE;
	break;
      case GLUT_MIDDLE_BUTTON:
	// translating camera:
	CameraMode = TRANSLATE;
	
	// get the modelview and projection matrices for projection
	// of the mouse's cursor into 3D
	glGetIntegerv(GL_VIEWPORT, ViewPort);
	glGetDoublev(GL_MODELVIEW_MATRIX, MvMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, ProjMatrix);
	
	// viewport[3] is height of window in pixels
	realy = ViewPort[3] - y - 1;
	
	// project the aim of the camera into window coordinates
	// only concerned about getting the depth (wz) from here
	gluProject(Aim.x, Aim.y, Aim.z, 
		   MvMatrix, ProjMatrix, ViewPort,
		   &wx, &wy, &wz);
	
	// from the depth found from the previous call, project the
	// mouse coordinates into 3D coordinates
	gluUnProject((GLdouble) x, (GLdouble) realy, wz,
		     MvMatrix, ProjMatrix, ViewPort, 
		     &PrevMousePos.x, &PrevMousePos.y, &PrevMousePos.z);
	
	break;
      case GLUT_RIGHT_BUTTON:
	// zooming camera:
	CameraMode = ZOOM;
	break;
    }
  }
}

/*
 * Mouse Motion handler function... should be called in the 
 * mouse motion function of your own code
 */
void Camera::HandleMouseMotion(int x, int y) {
  int mouse_dx, mouse_dy, d;
  double z;
  Vector3d MousePos, dir;
  Vector3d WindowX, WindowY, WindowZ;
  float realy;
  double wx, wy, wz;
  
  if (CameraMode != INACTIVE) {
    
    // find the greates change in mouse position 
    mouse_dx = x - MousePrevX;
    mouse_dy = y - MousePrevY;
    
    if (abs(mouse_dx) > abs(mouse_dy)) 
      d = mouse_dx;
    else
      d = mouse_dy;
    
    switch (CameraMode) {
      case ZOOM:
	// camera is zooming in
	z = (double) d / 100.0;
	
	dir = Aim - Pos;
	
	if (dir.norm() < 0.1 && z > 0) {
	  // move the aim position too when you get in really close
	  z *= 10.0;
	  Aim = Aim + z*dir;
	}
	
	// update the new position
	Pos = Pos + z * dir;
	break;
      case ROTATE:
	// camera is rotating
	
	// get rate of change in screen coordinates from when the 
	// mouse was first pressed
	DeltaAzim = ((double) (x - MouseStartX)) / 5.0;
	DeltaElev = ((double) (y - MouseStartY)) / 5.0;
	
	// get rate of change in screen coordinate from prev mouse pos
	LocalDeltaAzim = ((double) mouse_dx)  / 5.0;
	LocalDeltaElev = ((double) mouse_dy) / 5.0;
	
	// rotate the window coordinate system by the rate of change
	// from the onset of the mouse event
	
	// got this small section of code from Dr. House
	WindowX.set(1, 0, 0);
	WindowY.set(0, 1, 0);
	
	RotateX(WindowX, CurrentElev + DeltaElev);
	RotateY(WindowX, CurrentAzim + DeltaAzim);
	WindowX.z = -WindowX.z;
	
	RotateX(WindowY, CurrentElev+DeltaElev);
	RotateY(WindowY, CurrentAzim+DeltaAzim);
	WindowY.z = -WindowY.z;
	
	WindowZ = (WindowX % WindowY).normalize();
	
	ArbitraryRotate(WindowX, WindowY, WindowZ, 
			LocalDeltaElev, 0, Pos, Aim);
	
	ArbitraryRotate(Vector3d(1, 0, 0), Vector3d(0, 1, 0), 
			Vector3d(0, 0, 1), 0, -LocalDeltaAzim, Pos, Aim);
	
	Up = WindowY.normalize();
	
	break;
      case TRANSLATE:
	// camera is translating
	
	realy = ViewPort[3] - y - 1;
	
	gluProject(Aim.x, Aim.y, Aim.z, 
		   MvMatrix, ProjMatrix, ViewPort, 
		   &wx, &wy, &wz);
	
	gluUnProject((GLdouble) x, (GLdouble) realy, wz, 
		     MvMatrix, ProjMatrix, ViewPort,
		     &MousePos.x, &MousePos.y, &MousePos.z);
	
	// move both the camera position and its aim coordinate
	dir = MousePos - PrevMousePos;
	Pos = Pos - dir;
	Aim = Aim - dir;
	
	PrevMousePos = MousePos;
	break;
    }
    
    MousePrevX = x;
    MousePrevY = y;
  }
}

// assignment operator
const Camera& Camera::operator=(const Camera& Cam) {
  Aim = Cam.Aim;
  Pos = Cam.Pos;
  Up = Cam.Up;
  
  return *this;
}
