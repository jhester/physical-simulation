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
#include <vector>
#include <set>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "gauss.h"

const int WIDTH = 1024;
const int HEIGHT = 768;
const int TIMERMSECS = 1000 / 120;

float GravityVY = -8.9;
float Restitution = 0.8;
float PointSize = 1.0;
float KS=1.0;
float KD=0.1;
GLfloat centerOfMass[3];

int persp_win;

Camera *camera;

bool showGrid = true;

GLuint computeProgram, phongProgram; // programs
GLuint vbo, cbo;
GLfloat step=0.0;

int numVerts=0;
int numFaces=0;
int numNorms=0;
int arraySize=0;
int neighborArraySize=0;
GLuint *face_strip;

int installShaders(const char *, const char *);

static GLuint tex_neighbours, tex_neighbourhood, tex_forces, tex_velocity, tex_color, tex_position, fbo;
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
    if(showGrid)
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
    glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, tex_forces);
    glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, tex_neighbourhood);
    glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, tex_neighbours);
    
    // This is the magic speed component.
    // Here we get super fast access to the pixels on the GPU
    // and load them into a Vertex Buffer Object for reading.
    // We do this as opposed to loading into a heap allocatd array
    // Which saves a HUGE amount of process time, since everything virtually stays on
    // the GPU, no extra BUS accesses
	glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, vbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, arraySize, arraySize, GL_RGBA, GL_FLOAT, NULL); // MAGIC!!!
    glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
    
    glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, cbo);
    glReadBuffer(GL_COLOR_ATTACHMENT2);
    glReadPixels(0, 0, arraySize, arraySize, GL_RGBA, GL_FLOAT, NULL); // MAGIC!!!
    glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);

    // Go back to the main framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
    // We have uniform locations to make the computation simpler
    // So scale and translate into position
    //glScalef(10.0, 10.0, 10.0);
    // Setup blending and make the particles look nice
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);

    // Enable vertex arrays, bind our buffer with the texture PBO converted to VBO
    // Then draw the verts.
    //glEnable(GL_COLOR_MATERIAL);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo); 
    glVertexPointer(4, GL_FLOAT, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, cbo);
    glColorPointer(4, GL_FLOAT, 0, 0);
    
    glDrawElements(GL_TRIANGLES,numFaces*3,GL_UNSIGNED_INT,face_strip);
    glDrawArrays(GL_POINTS, 0, arraySize*arraySize);
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
	glViewport(0, 0, arraySize, arraySize);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	gluOrtho2D(0.0, arraySize, 0.0, arraySize);
	
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
    glUniform1i(glGetUniformLocation(computeProgram, "forcesTex"), 3);
    glUniform1i(glGetUniformLocation(computeProgram, "neighbourhoodTex"), 4);
    glUniform1i(glGetUniformLocation(computeProgram, "neighboursTex"), 5);
    glUniform1f(glGetUniformLocation(computeProgram, "time_step"), (GLfloat)time_step);
    glUniform1i(glGetUniformLocation(computeProgram, "texSixe"), arraySize);
    glUniform1i(glGetUniformLocation(computeProgram, "neighborTexSize"), neighborArraySize);
    glUniform1i(glGetUniformLocation(computeProgram, "numVerts"), numVerts);
    glUniform1i(glGetUniformLocation(computeProgram, "KS"), KS);
    glUniform1i(glGetUniformLocation(computeProgram, "KD"), KD);
    glUniform3f(glGetUniformLocation(computeProgram, "gravity"), (GLfloat)0.0, (GLfloat)GravityVY, (GLfloat)0.0);
    glUniform3f(glGetUniformLocation(computeProgram, "centerOfMass"), centerOfMass[0], centerOfMass[1], centerOfMass[2]);
    glUniform1f(glGetUniformLocation(computeProgram, "restitution"), (GLfloat)Restitution);
    
    // Draw the quad ie create a texture(s) that hold our position, velocity, and color
	glShadeModel(GL_FLAT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex2f(0.0, 0.0);
	glTexCoord2f(1.0, 0.0);
	glVertex2f( arraySize, 0.0);
	glTexCoord2f(1.0, 1.0);
	glVertex2f( arraySize,  arraySize);
	glTexCoord2f(0.0, 1.0);
	glVertex2f(0.0, arraySize);
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
	if (step >= arraySize*arraySize)
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
string objString("bunny.obj");
void LoadParameters(const char *filename){
    
    FILE *paramfile;
    
    if((paramfile = fopen(filename, "r")) == NULL){
        fprintf(stderr, "error opening parameter file %s\n", filename);
        exit(1);
    }
    ParamFilename = filename;
    char readin[50];
    if(fscanf(paramfile, " %f %f %f %f %s",
              &time_step, &PointSize, &KS, &KD, readin) != 5){
        fprintf(stderr, "error reading parameter file %s\n", filename);
        fclose(paramfile);
        exit(1);
    }
    objString = string(readin);
}

void preProcessOBJFile() {
	ifstream infile(objString.c_str());
	numVerts = numFaces = numNorms = 0;
    
	string line;
	while (getline(infile, line)) {
		istringstream iss(line);
		string type;
		iss >> type;
		if(type == "v") { // Count verts
			numVerts++;
			continue;
		}
        if(type == "vn") { // Count norms
			numNorms++;
			continue;
		}
		if(type == "f") { // Count faces
			numFaces++;
			continue;
		}
	}
    cout << objString << " model has " << numVerts << " vertices, " << numFaces << " faces." << endl;
}

void setupTextures () {
    cout << "Pre-processing OBJ file." << endl;
    preProcessOBJFile();
    cout << "Reading OBJ file, finding all neighbours." << endl;
    arraySize = sqrt(numVerts)+1; // Does this need to be +1?
    face_strip = new GLuint[3 * numFaces];
    
	GLfloat *tex_data = new GLfloat[4*arraySize*arraySize]; // This is key, EXC_BAD_ACCESS if not arraySize^2 instead of numVerts
    vector<set<GLuint> > neighborFaces(numVerts, set<GLuint>()); // faces are 1 indexed, dumb
    vector<Vector3d> verts;
    verts.reserve(arraySize*arraySize);
    
    ifstream infile(objString.c_str());
	string line, token, mtllib;
	int currentVertex = 0;
    int faceCount=0;
    while (getline(infile, line)) {
		istringstream iss(line);
		string type;
		iss >> type;
		
		if(type == "v") { // Vertex
			iss >> tex_data[4*currentVertex + 0] >> tex_data[4*currentVertex + 1] >> tex_data[4*currentVertex + 2];
			tex_data[4*currentVertex + 3] = 1.0;
            // Use this for laterz
            verts.push_back(Vector3d(tex_data[4*currentVertex + 0], tex_data[4*currentVertex + 1], tex_data[4*currentVertex + 2]));
            
            // Compute rudimenatary center of mass
            centerOfMass[0]+=tex_data[4*currentVertex + 0];
            centerOfMass[1]+=tex_data[4*currentVertex + 1];
            centerOfMass[2]+=tex_data[4*currentVertex + 2];
            ++currentVertex;
			continue;
		}
        
        if(type == "f") { // Face, remember there 1-indexed
			// Get face indices
            /*string indices;
            GLuint ndxs[3] = {0, 0, 0};
            getline(iss, indices);
            char * p = strtok((char *)indices.c_str(), " /");
            int it=0;
            while (p != NULL) {
                // Take 0, 2, 4
                if(atoi(p)-1 > numVerts) {
                    cout << indices << endl;
                    cout << p << endl;
                }
                if(it == 0) ndxs[0] = atoi(p)-1;
                if(it == 2) ndxs[1] = atoi(p)-1;
                if(it == 4) ndxs[2] = atoi(p)-1;
                ++it;
                p = strtok (NULL, " /");
            } */
            string indices[3];
            GLuint *ndxs = new GLuint[3];
    		for(int i=0;i<3;i++) {
				getline( iss, indices[0], '/' );
				getline( iss, indices[1],  '/' );
				getline( iss, indices[2],   ' ' );
				GLuint *ndxs1 = new GLuint[3];
				// Decrement indices to 0-based
				ndxs[i] = ndxs1[0] = atoi(indices[0].c_str())-1;
				ndxs1[1] = atoi(indices[1].c_str())-1;
				ndxs1[2] = atoi(indices[2].c_str())-1;
                face_strip[faceCount++] = atoi(indices[0].c_str())-1;
			}
            // Now assign neighbors
            neighborFaces[ndxs[0]].insert(ndxs[1]);
            neighborFaces[ndxs[0]].insert(ndxs[2]);
            
            neighborFaces[ndxs[1]].insert(ndxs[0]);
            neighborFaces[ndxs[1]].insert(ndxs[2]);
            
            neighborFaces[ndxs[2]].insert(ndxs[0]);
            neighborFaces[ndxs[2]].insert(ndxs[1]);
		}
    }
    
    cout << "Calculating center of mass for volume preservation." << endl;
    // Center of mass for volume saving
    // Also add to vertex texture
    tex_data[4*(arraySize*arraySize-1) +0] = centerOfMass[0] /= numVerts;
    tex_data[4*(arraySize*arraySize-1) +1] = centerOfMass[1] /= numVerts;
    tex_data[4*(arraySize*arraySize-1) +2] = centerOfMass[2] /= numVerts;
    // Important to add this in 
    verts[arraySize*arraySize-1] = Vector3d(centerOfMass[0], centerOfMass[1], centerOfMass[2]);
    
	cout << "Generate position texture w=" << arraySize << " h=" << arraySize<< endl;
	glGenTextures(1, &tex_position);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_position);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, arraySize, arraySize, 0, GL_RGBA, GL_FLOAT, &tex_data[0]);

    // Init all the velocities
	for (int i = 0; i < numVerts; ++i) {
		tex_data[i*4+0] = 0.0;
		tex_data[i*4+1] = 0.0;
		tex_data[i*4+2] = 0.0;
		tex_data[i*4+3] = 0.0;
	}
    
    // Init center of mass velocity
    tex_data[4*(arraySize*arraySize-1) +0] = 0.0;
    tex_data[4*(arraySize*arraySize-1) +1] = 0.0;
    tex_data[4*(arraySize*arraySize-1) +2] = 0.0;
    
    cout << "Generate velocity texture w=" << arraySize << " h=" << arraySize<< endl;
	glGenTextures(1, &tex_velocity);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex_velocity);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, arraySize, arraySize, 0, GL_RGBA, GL_FLOAT, &tex_data[0]);
    
    // Init all the colors and storage
    for (int i = 0; i < numVerts; ++i) {
        tex_data[i*4+0] = ((float) rand() / RAND_MAX) * 0.1 + 0.5;
        tex_data[i*4+1] = ((float) rand() / RAND_MAX) * 0.2;
        tex_data[i*4+2] = ((float) rand() / RAND_MAX) * 0.2;
        tex_data[i*4+3] = 1.0;
	}
    
    cout << "Generate color texture w=" << arraySize << " h=" << arraySize<< endl;
	glGenTextures(1, &tex_color);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tex_color);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, arraySize ,arraySize, 0, GL_RGBA, GL_FLOAT, &tex_data[0]);
    
    // Init all the external forces (lets do random for now, see what happens)
	for (int i = 0; i < numVerts; ++i) {
        tex_data[i*4+0] = ((float) rand() / RAND_MAX) * 0.1 + 0.5;
        tex_data[i*4+1] = ((float) rand() / RAND_MAX) * 0.2;
        tex_data[i*4+2] = ((float) rand() / RAND_MAX) * 0.2;
        tex_data[i*4+3] = 1.0;
	}
    
    // Center of mass external force, This could be very cool to vary this
    tex_data[4*(arraySize*arraySize-1) +0] = 0.0;
    tex_data[4*(arraySize*arraySize-1) +1] = 0.0;
    tex_data[4*(arraySize*arraySize-1) +2] = 0.0;
    
    cout << "Generate external forces texture w=" << arraySize << " h=" << arraySize<< endl;
	glGenTextures(1, &tex_forces);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, tex_forces);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, arraySize, arraySize, 0, GL_RGBA, GL_FLOAT, &tex_data[0]);
    
    cout << "Calculating neighbors and neighbourhood, rest lengths etc."<< endl;
    // Calculate size of the neighbors tex
    int sum=0;
    Vector cmass(centerOfMass[0], centerOfMass[1], centerOfMass[2]);
    for (int i=0; i<neighborFaces.size(); i++) {
        // Add center of mass to each neighbourSet
        neighborFaces[i].insert(arraySize*arraySize-1);
        sum+=neighborFaces[i].size();
    }
    neighborArraySize = sqrt(sum)+1; // +1??
    
    // Init the neighbors and neighbourhood tex, using faces info from OBJ
    // Center of mass is a neighbour, save its rest length
    // Then update its position etc in (arraySize-1, arraySize-1)
    // Neighborhood= (tex_neighbours_start_x, tex_neighbours_start_y, number_of_neighbours)
    // Neighbors=(tex_position_x, tex_position_y, rest_length, index)
    GLfloat *tex_neighbours_data = new GLfloat[4*neighborArraySize*neighborArraySize];
    int offset=0;
	for (int i = 0; i < numVerts; ++i) {
		tex_data[i*4+0] = (GLuint)(offset % neighborArraySize); // start X in neighbors tex
		tex_data[i*4+1] = (GLuint)(offset / neighborArraySize); // start Y in neighbors tex
		tex_data[i*4+2] = neighborFaces[i].size(); // # of neighbors
		tex_data[i*4+3] = 0.0;

        // Now fill out neighbourhood
        set<GLuint> neighboursset =  neighborFaces[i];
        set<GLuint>::const_iterator iter = neighboursset.begin();
        int j=0;
        //cout << "Vertex " << i << ", X: " << tex_data[i*4+0] << " Y: " << tex_data[i*4+1] << endl;
        while(iter != neighboursset.end()) {
            tex_neighbours_data[(offset+j)*4+0] = (*iter) % arraySize; // X coord of this vertex / particle in position_tex etc
            tex_neighbours_data[(offset+j)*4+1] = (*iter) / arraySize; // Y coord of this vertex / particle in position_tex etc
            
            Vector3d restLength = verts[i] - verts[*iter];
            tex_neighbours_data[(offset+j)*4+2] = restLength.norm();
            
            tex_neighbours_data[(offset+j)*4+3] = *iter;
            
            //cout << " TND: " << (offset+j)*4+0 << " X: " << tex_neighbours_data[(offset+j)*4+0] << " Y: " << tex_neighbours_data[(offset+j)*4+1]  << " L: " << tex_neighbours_data[(offset+j)*4+2] << endl;
            ++iter;++j;
        }
        
        offset+=neighboursset.size();
	}
    
    // Init neighbour support textures
    cout << "Generate neighbourhood texture w=" << arraySize << " h=" << arraySize<< endl;
    glGenTextures(1, &tex_neighbourhood);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, tex_neighbourhood);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, arraySize, arraySize, 0, GL_RGBA, GL_FLOAT, &tex_data[0]);
    
    cout << "Generate neighbours texture w=" << neighborArraySize << " h=" << neighborArraySize<< endl;
	glGenTextures(1, &tex_neighbours);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, tex_neighbours);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, neighborArraySize, neighborArraySize, 0, GL_RGBA, GL_FLOAT, &tex_neighbours_data[0]); // FIX
	
    delete [] tex_data;
    delete [] tex_neighbours_data;
    
    // Gen the framebuffer for rendering
    cout << "Init framebuffer, bind textures, VBO, and CBO." << endl;
	glGenFramebuffersEXT(1, &fbo);
  	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
    
    // bind textures so we can use them laterz
	glBindTexture(GL_TEXTURE_2D, tex_position);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, tex_position, 0);
    
	glBindTexture(GL_TEXTURE_2D, tex_velocity);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, tex_velocity, 0);
    
    glBindTexture(GL_TEXTURE_2D, tex_color);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, tex_color, 0);
    
    glBindTexture(GL_TEXTURE_2D, tex_forces);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_2D, tex_forces, 0);

    glBindTexture(GL_TEXTURE_2D, tex_neighbourhood);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT4_EXT, GL_TEXTURE_2D, tex_neighbourhood, 0);
    
    glBindTexture(GL_TEXTURE_2D, tex_neighbours);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT5_EXT, GL_TEXTURE_2D, tex_neighbours, 0);
	if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        printf("ERROR - Incomplete FrameBuffer\n");
    
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
    // Vertex buffer init, positions
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, arraySize*arraySize*4*sizeof(GLfloat), NULL, GL_STREAM_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Color buffer init, positions
    glGenBuffers(1, &cbo);
    glBindBuffer(GL_ARRAY_BUFFER, cbo);
    glBufferData(GL_ARRAY_BUFFER, arraySize*arraySize*4*sizeof(GLfloat), NULL, GL_STREAM_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    cout << "Were in the clear." << endl;
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
    camera = new Camera(Vector3d(0, 12, 30), Vector3d(0, 6, 0),
                        Vector3d(0, 1, 0));
    camera->SetCenterOfFocus(Vector3d(0, 6, 0));
    // grey background for window
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH);
    glDepthRange(0.0, 1.0);
    glEnable(GL_NORMALIZE);
    glPointSize(PointSize);
    glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive
    
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
    persp_win = glutCreateWindow("Springy Meshes");
    
    // initialize the camera and such
    LoadParameters("parameters");
    init();
    
    // Textures for lookup
    setupTextures();
    
    // Read in shader program char* and init shader 
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

