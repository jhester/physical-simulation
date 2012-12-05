//
// Camera.h
// OpenGL Camera Class
// 
// Christopher Root, 2006
// Modifications by Donald House, 2009
//   corrected calculation of camera coordinates
//
//

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "Matrix.h"
#include <stdio.h>
#define INACTIVE 0
#define TRANSLATE 1
#define ROTATE 2
#define ZOOM 3

class Camera {
private:
  // all variables starting with 'Default' hold the initial camera values
  // these values are used if the camera is reset to its initial state
  Vector3d DefaultPos;
  Vector3d DefaultAim;
  Vector3d DefaultUp;
  
  double DefaultAzim;
  double DefaultElev;
  
  double CurrentAzim;
  double CurrentElev;
  
  void Initialize();
  void ComputeCoords(const Vector3d &P, const Vector3d &A, const Vector3d &U);
  
public:
  Vector3d Pos;
  Vector3d Aim; 
  Vector3d Up;
  
  float NearPlane;
  float FarPlane;
  float Fov;
  
  // constructors
  
  // default constructor
  Camera();
  
  // constructor setting up camera orientation
  // P is position in 3D, A is the aim coordinate in 3D, U is the up vector
  Camera(Vector3d P, Vector3d A, Vector3d U);
  
  // constructor setting up camera orientation and view volume
  // P is position in 3D, A is aim coordinate in 3D, U is up vector
  // Near is near clipping plane, Far is far clipping plane, 
  // ViewAngle is field of view angle in degrees
  Camera(Vector3d P, Vector3d A, Vector3d U, 
	 float Near, float Far, float ViewAngle);
  
  // sets the clipping planes of the view volume
  void SetClippingPlanes(float Near, float Far);
  
  // sets the FOV, ViewAngle should be in degrees
  void SetFOV(float ViewAngle);	
  
  // set routines for Pos, Aim, and Up vector
  void SetPos(Vector3d P);
  void SetAim(Vector3d A);
  void SetUp(Vector3d U);
  
  // reset the camera to its initial position
  void Reset();
  
  // change camera aim position, and also move camera to same
  // relative position with respect to new aim position
  void SetCenterOfFocus(Vector3d NewAim);
  
  // function to use the camera as the opengl camera
  // W and H are the width and height of the window
  void PerspectiveDisplay(int W, int H);
  
  // function that handles mouse events
  void HandleMouseEvent(int button, int state, int x, int y);
  
  // function that handles mouse movements
  void HandleMouseMotion(int x, int y);
  
  const Camera& operator=(const Camera& cam);
};

#endif

