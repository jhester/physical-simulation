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

const int WIDTH = 1024;
const int HEIGHT = 768;
const int TIMERMSECS = 1000 / 60;
int SQR_OF_NUM_PARTICLES = 1024;
// I *think* powers of 2 are most performant, have not profiled it though
// 256 has best performance with most particles (65,536)
// 512 still runs on my little compy pretty well : 262,144 particles (woot!)
// 1024 is slow, run out of texture space at that point for my crappy card, : 1,048,576 particles (woot*2!)

float Restitution = 0.8;
float GravityVY = -4.9;
float Mass = 1.0;
float PointSize = 1.0;
float TimeMultiplier = 4.0;
int DominantSpectrum = 1;
int persp_win;

Camera *camera;

bool showGrid = true;

GLuint computeProgram, phongProgram; // programs
GLuint vbo, cbo;
GLfloat step=0.0;

int installShaders(const char *, const char *);

static GLuint arrayWidth, arrayHeight;

static GLuint tex_velocity, tex_color, tex_position, fbo;
static GLfloat time_step = 0.005;

// draws a simple grid
void makeGrid() {
    glColor3f(0.63, 0.63, 0.63);
    
  
    for (float i=-12; i<12; i++) {
        for (float j=-12; j<12; j++) {
            glBegin(GL_LINES);
            glVertex3f(i, 0, j);
            glVertex3f(i, 0, j+1);
            glEnd();
            glBegin(GL_LINES);
            glVertex3f(i, 0, j);
            glVertex3f(i+1, 0, j);
            glEnd();
            
            if (j == 11){
                glBegin(GL_LINES);
                glVertex3f(i, 0, j+1);
                glVertex3f(i+1, 0, j+1);
                glEnd();
            }
            if (i == 11){
                glBegin(GL_LINES);
                glVertex3f(i+1, 0, j);
                glVertex3f(i+1, 0, j+1);
                glEnd();
            }
        }
    }
    
    glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex3f(-12, 0, 0);
    glVertex3f(12, 0, 0);
    glEnd();
    glBegin(GL_LINES);
    glVertex3f(0, 0, -12);
    glVertex3f(0, 0, 12);
    glEnd();
    glLineWidth(1.0);
}

void PerspDisplay() {
	glDrawBuffer(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // draw the camera created in perspective
    camera->PerspectiveDisplay(WIDTH, HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    
    // Show the grid
    glLoadIdentity();
    glScalef(2.0, 0.0, 2.0);
    makeGrid();
    
    // Now get the framebuffer we rendered textures to
    // bind the textures and enable them as well
    glLoadIdentity();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_position);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex_velocity);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tex_color);
    
    
    // This is the magic speed component.
    // Here we get super fast access to the pixels on the GPU
    // and load them into a Vertex Buffer Object for reading.
    // We do this as opposed to loading into a heap allocatd array
    // Which saves a HUGE amount of process time, since everything virtually stays on
    // the GPU, no extra BUS accesses
    //
    // Took me a long time trolling messageboards and docs. These four lines took days.
    // to find out how to do this with the crappy old version of OpenGL
    // thats supported by macs video cards.
	glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, vbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, arrayWidth, arrayHeight, GL_RGBA, GL_FLOAT, NULL); // MAGIC!!!
    glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
    
    glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, cbo);
    glReadBuffer(GL_COLOR_ATTACHMENT2);
    glReadPixels(0, 0, arrayWidth, arrayHeight, GL_RGBA, GL_FLOAT, NULL); // MAGIC!!!
    glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);

    // Go back to the main framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
	// Setup blending and make the particles look nice
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    
    // We have uniform locations to make the computation easier
    // So scale and translate into position
    glScalef(10.0, 10.0, 10.0);
    glTranslatef(0.0, 2.0, 0.0);

    
    // This is big right here lots of optimizations. Enable vertex arrays, bind
    // our buffer with the texture PBO converted to VBO
    // Then draw the verts.
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo); 
    glVertexPointer(4, GL_FLOAT, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, cbo);
    glColorPointer(4, GL_FLOAT, 0, 0);
    
    glDrawArrays(GL_POINTS, 0, arrayWidth * arrayHeight);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableClientState (GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    
	glutSwapBuffers();
}

void computationPass() {
    glLoadIdentity();
	
    // Render to texture setup,
    // Esentially making a full screen quad so each texel turns into a fragment which can be
    // replaced using the frag shader. Hack magic
	glViewport(0, 0, arrayWidth, arrayHeight);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	gluOrtho2D(0.0, arrayWidth, 0.0, arrayHeight);
	
	glEnable(GL_TEXTURE_2D);
    
    // Bind to our new framebuffer that will not be seen
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
    // This is important, specify number of gl_FragData[N] outputs
	GLenum drawBufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    // N=3
	glDrawBuffers(3, drawBufs);
    
    // Use our program and set uniforms
	glUseProgram(computeProgram);
    glUniform1i(glGetUniformLocation(computeProgram, "positionTex"), 0);
    glUniform1i(glGetUniformLocation(computeProgram, "velocityTex"), 1);
    glUniform1i(glGetUniformLocation(computeProgram, "colorTex"), 2);
    glUniform1f(glGetUniformLocation(computeProgram, "step"), (GLfloat)step);
    
    glUniform1f(glGetUniformLocation(computeProgram, "time_step"), (GLfloat)time_step);
    glUniform3f(glGetUniformLocation(computeProgram, "gravity"), (GLfloat)0.0, (GLfloat)GravityVY, (GLfloat)0.0);
    glUniform1f(glGetUniformLocation(computeProgram, "restitution"), (GLfloat)Restitution);
    glUniform1i(glGetUniformLocation(computeProgram, "spectrum"), DominantSpectrum);
    
    // Draw the quad ie create a texture(s) that hold our position, velocity, and color
	glShadeModel(GL_FLAT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex2f(0.0, 0.0);
	glTexCoord2f(1.0, 0.0);
	glVertex2f( arrayWidth, 0.0);
	glTexCoord2f(1.0, 1.0);
	glVertex2f( arrayWidth,  arrayHeight);
	glTexCoord2f(0.0, 1.0);
	glVertex2f(0.0, arrayHeight);
	glEnd();
	
    glUseProgram(phongProgram);
    //glUseProgram(0);
    // Return it back to normal
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glViewport(0, 0, WIDTH, HEIGHT);
	glFlush();
    
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glDisable(GL_TEXTURE_2D);
}

void animate(int value) {
    glutPostRedisplay();
    
    computationPass();
    step+=1.0;
	if (step >= arrayWidth*arrayHeight)
		step = 0;
       
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
    if(fscanf(paramfile, "%f %f %f %f %f %d %d",
              &Restitution, &GravityVY, &time_step, &PointSize, &TimeMultiplier, &SQR_OF_NUM_PARTICLES, &DominantSpectrum ) != 7){
        fprintf(stderr, "error reading parameter file %s\n", filename);
        fclose(paramfile);
        exit(1);
    }
    if(DominantSpectrum < 0 && DominantSpectrum > 2) {
        DominantSpectrum = 0;
        cout << "Bad value for specturm default to \"Red\"" << endl;
    }
    cout << "Point Size : " << PointSize << " Time Multiplier : " << TimeMultiplier << endl;
}

void setupTextures () {
    
	int numParticles = arrayWidth*arrayHeight;
	GLfloat *tex_data = new GLfloat[4*numParticles];
	
	srand48( time(NULL) );
	int seed = rand();
    
    // Init all to fit in uniform rectangle x : [1.0, 1.0], y : [0.8, 1.0], z : [-1, 1]
	for (int i = 0; i < numParticles; ++i) {
		tex_data[4*i + 0] = 1.0;//gauss(0.0, 0.01, seed);
		tex_data[4*i + 1] = ((float) rand() / RAND_MAX)*0.2+0.8; //gauss(0.0, 0.01, seed);
		tex_data[4*i + 2] = ((float) rand() / RAND_MAX)*2.0-1.0;//gauss(0.0, 0.01, seed);
		tex_data[4*i + 3] = 1.0;
	}
	
	glGenTextures(1, &tex_position);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_position);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, arrayWidth, arrayHeight, 0, GL_RGBA, GL_FLOAT, &tex_data[0]);
    
    // Init all the velocities, generally heading negative -x
	for (int i = 0; i < numParticles; ++i) {
		tex_data[i*4+0] = -gauss(1.1, 0.11, seed);
		tex_data[i*4+1] = gauss(0.1, 0.09, seed);
		tex_data[i*4+2] = gauss(0.0, 0.01, seed);
		tex_data[i*4+3] = ((GLfloat)rand() / ((GLfloat)RAND_MAX))*TimeMultiplier; // Use this as the start time of the particle, maybe also the mass+1.0?
	}
    
	glGenTextures(1, &tex_velocity);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex_velocity);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, arrayWidth, arrayHeight, 0, GL_RGBA, GL_FLOAT, &tex_data[0]);

    // Init all the colors and storage
    for (int i = 0; i < numParticles; ++i) {
        tex_data[i*4+0] = ((float) rand() / RAND_MAX) * 0.2;
        tex_data[i*4+1] = ((float) rand() / RAND_MAX) * 0.2;
        tex_data[i*4+2] = ((float) rand() / RAND_MAX) * 0.2;
        tex_data[i*4+3] = 0.0; // Initially invisible
        if(DominantSpectrum == 0) {
            tex_data[i*4+0] = ((float) rand() / RAND_MAX) * 0.1 + 0.5;
        }
        if(DominantSpectrum == 1) {
            tex_data[i*4+1] = ((float) rand() / RAND_MAX) * 0.1 + 0.5;
        }
        if(DominantSpectrum == 2) {
       		tex_data[i*4+2] = ((float) rand() / RAND_MAX) * 0.1 + 0.5;
        }
	}
	glGenTextures(1, &tex_color);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tex_color);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, arrayWidth, arrayHeight, 0, GL_RGBA, GL_FLOAT, &tex_data[0]);
    
	delete [] tex_data;
    
    // Gen the framebuffer for rendering
	glGenFramebuffersEXT(1, &fbo);
  	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
    
    // bind textures so we can use them laterz
	glBindTexture(GL_TEXTURE_2D, tex_position);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, tex_position, 0);
    
	glBindTexture(GL_TEXTURE_2D, tex_velocity);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, tex_velocity, 0);
    
    glBindTexture(GL_TEXTURE_2D, tex_color);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, tex_color, 0);
    
	if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        printf("ERROR - Incomplete FrameBuffer\n");
    
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
    // Vertex buffer init, positions
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, arrayWidth*arrayHeight*4*sizeof(GLfloat), NULL, GL_STREAM_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Color buffer init, positions
    glGenBuffers(1, &cbo);
    glBindBuffer(GL_ARRAY_BUFFER, cbo);
    glBufferData(GL_ARRAY_BUFFER, arrayWidth*arrayHeight*4*sizeof(GLfloat), NULL, GL_STREAM_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void keyboardEventHandler(unsigned char key, int x, int y) {
    switch (key) {
        case 'c': case 'C': {
            // reset the camera to its initial position
            camera->Reset();
            break;
        }
        case 'r': case 'R': {
            LoadParameters("parameters");
            arrayHeight = SQR_OF_NUM_PARTICLES;
            arrayWidth = SQR_OF_NUM_PARTICLES;
            step = 0;
            glPointSize(PointSize);
            setupTextures();
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
    camera = new Camera(Vector3d(0, 12, 30), Vector3d(0, 12, 0),
                        Vector3d(0, 1, 0));
    camera->SetCenterOfFocus(Vector3d(0, 12, 0));
    // grey background for window
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH);
    glDepthRange(0.0, 1.0);
    glEnable(GL_NORMALIZE);
    glPointSize(PointSize);
    glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive
    arrayHeight = SQR_OF_NUM_PARTICLES;
    arrayWidth = SQR_OF_NUM_PARTICLES;
    
    // Set up lighting and materials for phong shading
    float key_diffuse[] = {0.7, 0.7, 0.7, 1.0 };
	float key_specular[] = {0.7, 0.7, 0.7, 1.0};
    float key_position[] = {0.0, 12.0, 30.0};
    float key_direction[] = {0.0, 0.0, 0.0};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, key_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, key_specular);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, key_direction);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.2);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.01); //GLossy
	glLightfv(GL_LIGHT0, GL_POSITION, key_position);
    
    float kd[] = {1.00, 0.80, 0.70, 1.0};
    float ks[] = {0.15, 0.15, 0.15, 1.0};
    float ns[] = {70.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, kd);
    glMaterialfv(GL_FRONT, GL_SPECULAR, ks);
    glMaterialfv(GL_FRONT, GL_SHININESS, ns);
}


int main(int argc, char *argv[]) {
    // set up opengl window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(50, 50);
    persp_win = glutCreateWindow("Lavafall");
    
    // initialize the camera and such
    LoadParameters("parameters");
    init();
    
    // Textures for lookup
    setupTextures();
    
    // Read in shader program char* and init shader 
    //installShaders("particles.vert", "particles.frag");
    phongProgram = installShaders("phong.vert", "phong.frag");
    computeProgram = installShaders("firstpass.vert", "firstPass.frag");
    
    // set up opengl callback functions
    glutDisplayFunc(PerspDisplay);
    glutMouseFunc(mouseEventHandler);
    glutMotionFunc(motionEventHandler);
    glutKeyboardFunc(keyboardEventHandler);
    
    glutTimerFunc(TIMERMSECS, animate, 0);
    glutMainLoop();
    return(0);
}

