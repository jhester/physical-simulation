// TODO : Motion blur? Second shader pass?

#include "Camera.h"

#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #define GL_GLEXT_PROTOTYPES 1
    #include <GL/gl.h>
    #include <GL/glu.h>
    #include <GL/glut.h>
    #include <GL/glx.h>
    #include <GL/glext.h>
#endif

#include <string>
#include <fstream>
#include <cmath>
#include "gauss.h"
#include "RigidBody.h"
const int WIDTH = 1024;
const int HEIGHT = 768;
const int TIMERMSECS = 1000 / 60;

double Restitution = 0.8;
double GravityVY = -4.9;
Vector3d gravity(0.0, -4.9, 0.0);
double Mass = 1.0;
double Drag = 0.13;
int INTEGRATION_METHOD=1; // 0-Euler, 1-Runga Kutta 4
int persp_win;

RigidBody box;

double restLength = 4.0;

Camera *camera;

bool showGrid = true;

GLuint computeProgram, phongProgram; // programs
GLuint vbo, cbo;
GLfloat timevalue=0.0;
float TIMESTEP=0.03;

float key_position[] = {10.0, 20, 10, 1.0};

int installShaders(const char *, const char *);

static GLfloat time_step = 0.005;

void set_material_floor_mat() {
    float mat_ambient[] = {0.7, 0.7, 0.7, 1.0 };
    float mat_diffuse[] = {0.4, 0.9, 0.4, 1.0 };
    float mat_specular[] = {0.4, 0.4, 0.4, 1.0};
    float mat_shininess[] = {60.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

void set_material_torus_mat() {
    float mat_ambient[] = {0.3, 0.3, 0.3, 1.0 };
    float mat_diffuse[] = {0.9, 0.5, 0.5, 1.0 };
    float mat_specular[] = {0.6, 0.6, 0.6, 1.0};
    float mat_shininess[] = {60.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

void set_material_sphere_mat() {
    float mat_ambient[] = {0.3, 0.3, 0.3, 1.0 };
    float mat_diffuse[] = {0.5, 0.5, 0.9, 1.0 };
    float mat_specular[] = {0.4, 0.4, 0.4, 1.0};
    float mat_shininess[] = {20.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

void set_material_shadow_mat() {
    float mat_ambient[] = {0.0, 0.0, 0.0, 0.4 };
    float mat_diffuse[] = {0.0, 0.0, 0.0, 0.4 };
    float mat_specular[] = {0.0, 0.0, 0.0, 0.2};
    float mat_shininess[] = {1.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}


void calcNormal(float v[3][3], float out[3]) {
	float v1[3], v2[3], length;
	static const int x = 0, y = 1, z = 2;
	
	v1[x] = v[0][x] - v[1][x];
	v1[y] = v[0][y] - v[1][y];
	v1[z] = v[0][z] - v[1][z];
	
	v2[x] = v[1][x] - v[2][x];
	v2[y] = v[1][y] - v[2][y];
	v2[z] = v[1][z] - v[2][z];
	
	out[x] = (v1[y] * v2[z]) - (v1[z] * v2[y]);
	out[y] = (v1[z] * v2[x]) - (v1[x] * v2[z]);
	out[z] = (v1[x] * v2[y]) - (v1[y] * v2[x]);
	
	length = (float)sqrt( (out[x] * out[x]) + (out[y] * out[y]) + (out[z] * out[z]) );
	
	out[x] = out[x] / length;
	out[y] = out[y] / length;
	out[z] = out[z] / length;
}

void MakeShadowMatrix(GLfloat points[3][3], GLfloat lightPos[4], GLfloat destMat[4][4]) {
	GLfloat planeCoeff[4];
	GLfloat dot;
	
	calcNormal(points, planeCoeff);
	
	planeCoeff[3] = - ( (planeCoeff[0] * points[2][0]) + (planeCoeff[1] * points[2][1]) + (planeCoeff[2] * points[2][2]) );
	
	dot = (planeCoeff[0] * lightPos[0]) + (planeCoeff[1] * lightPos[1]) + (planeCoeff[2] * lightPos[2]) + (planeCoeff[3] * lightPos[3]);
	
	destMat[0][0] = dot - (lightPos[0] * planeCoeff[0]);
	destMat[1][0] = 0.0f - (lightPos[0] * planeCoeff[1]);
	destMat[2][0] = 0.0f - (lightPos[0] * planeCoeff[2]);
	destMat[3][0] = 0.0f - (lightPos[0] * planeCoeff[3]);
	
	destMat[0][1] = 0.0f - (lightPos[1] * planeCoeff[0]);
	destMat[1][1] = dot - (lightPos[1] * planeCoeff[1]);
	destMat[2][1] = 0.0f - (lightPos[1] * planeCoeff[2]);
	destMat[3][1] = 0.0f - (lightPos[1] * planeCoeff[3]);
	
	destMat[0][2] = 0.0f - (lightPos[2] * planeCoeff[0]);
	destMat[1][2] = 0.0f - (lightPos[2] * planeCoeff[1]);
	destMat[2][2] = dot - (lightPos[2] * planeCoeff[2]);
	destMat[3][2] = 0.0f - (lightPos[2] * planeCoeff[3]);
	
	destMat[0][3] = 0.0f - (lightPos[3] * planeCoeff[0]);
	destMat[1][3] = 0.0f - (lightPos[3] * planeCoeff[1]);
	destMat[2][3] = 0.0f - (lightPos[3] * planeCoeff[2]);
	destMat[3][3] = dot - (lightPos[3] * planeCoeff[3]);
}

void draw_rigidbody(bool SHADOW) {
    if(SHADOW) set_material_shadow_mat();
    else set_material_sphere_mat();
    glPushMatrix();
    Vector3d position = box.getPosition();
    glTranslatef(position.x, position.y, position.z);
    Quaternion r = box.getRotation();
    Vector3d axis = r.axis();
    glRotatef(r.angle(), axis.x, axis.y, axis.z);
    
    glutSolidCube(8);
    glPopMatrix();
}

void draw_plane() {
    // Draw plane
    set_material_floor_mat();
    glPushMatrix();
    glBegin(GL_QUADS);
    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(-25, 0, 25);
    glVertex3f(-25, 0, -25);
    glVertex3f(25, 0, -25);
    glVertex3f(25, 0, 25);
    glEnd();
    glPopMatrix();
}

void draw_line() {
    // Draw line
    set_material_torus_mat();
    glPushMatrix();
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(2.0);
    Quaternion r = box.getRotation();
    Vector3d np = Vector3d(4, 4, 4) * r.rotation() + box.getPosition();
    glBegin(GL_LINES);
    glVertex3f(0, 40, 0);
    glVertex3f(np.x, np.y, np.z);
    glEnd();
    glDisable(GL_BLEND);
    glPopMatrix();
}

void draw_torus(bool SHADOW) {
    if(SHADOW) set_material_shadow_mat();
    else set_material_torus_mat();
    glPushMatrix();
    glTranslatef(-7.0, 5.0, 12.0);
    glRotatef(90, 1.0, -1.0, 0);
    glutSolidTorus(1.5, 4.0, 50, 60);
    glPopMatrix();
}

void PerspDisplay() {
	glDrawBuffer(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    // Shadows
    glEnable(GL_BLEND); //enable alpha blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    draw_plane();
    glDisable(GL_TEXTURE_2D); //disable texturing of the shadow
    glDisable(GL_DEPTH_TEST); //disable depth testing of the shadow;
    GLfloat shadowMat[4][4];
	GLfloat points[3][3] = {{0.0,1.0,0.0}, {0.0,1.0,1.0},{ 1.0,1.0,1.0}};

	MakeShadowMatrix(points, key_position, shadowMat);
    
	glPushMatrix();
	glMultMatrixf((GLfloat *)shadowMat);
    
    glUseProgram(0);
    // Draw torus shadow
    draw_torus(true);
    draw_rigidbody(true);
	glPopMatrix();
    
	glEnable(GL_DEPTH_TEST); //enable depth testing
    glEnable(GL_TEXTURE_2D); //enable texturing
	glDisable(GL_BLEND); //disable alpha blending

    // draw the camera created in perspective
    camera->PerspectiveDisplay(WIDTH, HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glUseProgram(phongProgram);
    
    // Draw torus
    draw_torus(false);
    draw_rigidbody(false);
    draw_line();
	glutSwapBuffers();
}

void animate(int value) {
    glutPostRedisplay();
    
    // Compute physics, RK4
    box.update(timevalue, TIMESTEP);
    timevalue+=TIMESTEP;
    
    glutTimerFunc(TIMERMSECS, animate, 0);
}

void mouseEventHandler(int button, int state, int x, int y) {
    // let the camera handle some specific mouse events (similar to maya)
    camera->HandleMouseEvent(button, state, x, y);
}

void motionEventHandler(int x, int y) {
    // let the camera handle some mouse motions if the camera is to be moved
    camera->HandleMouseMotion(x, y);
    glutPostRedisplay();
}

const char *ParamFilename = NULL;
void LoadParameters(const char *filename){
    
    FILE *paramfile;
    
    if((paramfile = fopen(filename, "r")) == NULL){
        fprintf(stderr, "error opening parameter file %s\n", filename);
        exit(1);
    }
    ParamFilename = filename;
    double newMass = 1;
    float newKs = 1.0, newKd = 0.3;
    if(fscanf(paramfile, "%lf %lf %f %lf %f %f",
              &Restitution, &GravityVY, &TIMESTEP, &newMass, &newKs, &newKd ) != 6){
        fprintf(stderr, "error reading parameter file %s\n", filename);
        fclose(paramfile);
        exit(1);
    }
    box.setParameters(newMass, newKs, newKd, Restitution, GravityVY);
    
    
}

void keyboardEventHandler(unsigned char key, int x, int y) {
    switch (key) {
        case 'c': case 'C': {
            // reset the camera to its initial position
            camera->Reset();
            break;
        }
        case 'r': case 'R': {
            box = RigidBody();
            LoadParameters("parameters");
            timevalue = 0.0;
            break;
        } case 'f': case 'F': {
            camera->SetCenterOfFocus(Vector3d(0, 10, 0));
            break;
        } case 'g': case 'G':
            showGrid = !showGrid;
            break;
            
        case 'q': case 'Q':	// q or esc - quit
        case 27:		// esc
            exit(0);
    }
    
    glutPostRedisplay();
}

GLint getUniLoc(GLuint program, const GLchar *name) {
    GLint loc;
    loc = glGetUniformLocation(program, name);
    if (loc == -1)
        printf("No such uniform named \"%s\"\n", name);
    return loc; 
}

int installShaders(const char *filenameV, const char *filenameF) {
    GLuint programRet;
    std::string line,text, text2;
    std::ifstream in(filenameV);
    while(std::getline(in, line))
    {
        text += line + "\n";
    }
    const char* brickVertex = text.c_str();
    in.close();
    
    in.open(filenameF);
    while(std::getline(in, line))
    {
        text2 += line + "\n";
    }
    const char* brickFragment = text2.c_str();

    GLuint brickVS, brickFS; 
    // handles to objects 
    GLint vertCompiled, fragCompiled; 
    // status values 
    GLint linked;
    
    // Create a vertex shader object and a fragment shader object
    brickVS = glCreateShader(GL_VERTEX_SHADER); 
    brickFS = glCreateShader(GL_FRAGMENT_SHADER);
    
    // Load source code strings into shaders
    glShaderSource(brickVS, 1, &brickVertex, NULL); 
    glShaderSource(brickFS, 1, &brickFragment, NULL);
    
    // Compile the brick vertex shader and print out // the compiler log file.
    glCompileShader(brickVS);
    // Check for OpenGL errors 
    glGetShaderiv(brickVS, GL_COMPILE_STATUS, &vertCompiled); 
    // Compile the brick fragment shader and print out 
    // the compiler log file.
    glCompileShader(brickFS);
    // Check for OpenGL errors 
    glGetShaderiv(brickFS, GL_COMPILE_STATUS, &fragCompiled); 
    
    if (!vertCompiled || !fragCompiled) {
             cout << "[ERROR] Could not compile program." << endl;
        return 0;
    }
    // Create a program object and attach the two compiled shaders
    programRet = glCreateProgram(); 
    glAttachShader(programRet, brickVS);
    glAttachShader(programRet, brickFS);

    // Link the program object and print out the info log
    glLinkProgram(programRet);
    // Check for OpenGL errors 
    glGetProgramiv(programRet, GL_LINK_STATUS, &linked); 
    if (!linked) {
        cout << "[ERROR] Did not link program." << endl;
        return 0;
    }
    
    return programRet;
}

void init() {
    // set up camera
    // parameters are eye point, aim point, up vector
    camera = new Camera(Vector3d(5, 17, 25), Vector3d(0, 0, 0),
                        Vector3d(0, 1, 0));
    camera->SetCenterOfFocus(Vector3d(0, 14, 0));
    glClearColor(0.1, 0.1, 0.1, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel (GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
    glEnable(GL_NORMALIZE);
    
    // Set up lighting and materials for phong shading
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
 	float light0_ambient[] = {0.1, 0.1, 0.1, 1.0};
  	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,1);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light0_ambient);
    
    // KEY Light
    float key_ambient[] = {0.0, 0.0, 0.0, 1.0};
	float key_diffuse[] = {1.0, 1.0, 1.0, 1.0 };
	float key_specular[] = {1.0, 1.0, 1.0, 1.0};
    
	glLightfv(GL_LIGHT0, GL_AMBIENT, key_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, key_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, key_specular);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.001);
	glLightfv(GL_LIGHT0, GL_POSITION, key_position);
}

int main(int argc, char *argv[]) {
    // set up opengl window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_ACCUM | GLUT_STENCIL);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(50, 50);
    persp_win = glutCreateWindow("Lavafall");
    
    // initialize the camera and such
    LoadParameters("parameters");
    init();
    
    // Read in shader program char* and init shader
    phongProgram = installShaders("phong.vert", "phong.frag");
    
    // set up opengl callback functions
    glutDisplayFunc(PerspDisplay);
    glutMouseFunc(mouseEventHandler);
    glutMotionFunc(motionEventHandler);
    glutKeyboardFunc(keyboardEventHandler);
    
    glutTimerFunc(TIMERMSECS, animate, 0);
    glutMainLoop();
    return(0);
}

