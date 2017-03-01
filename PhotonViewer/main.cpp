#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <cstdio>
#include <cstdlib>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include <glm/glm.hpp>

using namespace std;

static const char * APP_TITLE = "PhotonMF - Photon map viewer";

// #define OUTPUT_REDIR
#ifdef OUTPUT_REDIR
static FILE * fout;
static FILE * ferr;
#endif

static int w = 640;
static int h = 480;
static GLuint dlist;
static vector<pair<glm::vec3, glm::vec3>> photons;

void init_gl(void);
void reshape(int _w, int _h);
void keyboard(unsigned char key, int x, int y);
void release_key(unsigned char key, int x, int y);
void idle(void);
void render_scene(void);
void clean_up(void);

void build_dlist() {
  dlist = glGenLists(1);
  glNewList(dlist, GL_COMPILE); {
    glBegin(GL_POINTS); {
      for (vector<pair<glm::vec3, glm::vec3>>::iterator it = photons.begin(); it != photons.end(); it++) {
	glColor3f((*it).second.r, (*it).second.g, (*it).second.b);
	glVertex3f((*it).first.x, (*it).first.y, (*it).first.z);
      }
    }
    glEnd();
  }
  glEndList();
}

int main(int argc, char ** argv) {
#ifdef OUTPUT_REDIR
  fout = freopen("stdout.txt", "w", stdout);
  ferr  = freopen("stderr.txt", "w", stderr);
#endif

  if (argc < 2) {
    cerr << "USAGE: " << argv[0] << " FILE" << endl;
    return EXIT_FAILURE;
  }

  ifstream ifs(argv[1], ios::in);

  if (!ifs.is_open()) {
    cerr << "Failed to open the file " << argv[1] << " for reading." << endl;
    return EXIT_FAILURE;
  }

  while (!ifs.eof()) {
    float x, y, z, r, g, b;
    ifs >> x >> y >> z >> r >> g >> b;
    photons.push_back(pair<glm::vec3, glm::vec3>(glm::vec3(x, y, z), glm::vec3(r, g, b)));
  }

  ifs.close();
  
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowPosition(10, 10);
  glutInitWindowSize(w, h);
  glutCreateWindow(APP_TITLE);
  glutKeyboardFunc(keyboard);
  glutIdleFunc(idle);
  glutDisplayFunc(render_scene);
  glutReshapeFunc(reshape);

  atexit(clean_up);
  init_gl();
	
  glutMainLoop();

  return 0;
}

void init_gl(void) {
  /* Enable back-face culling. */
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  /* Enable lightning. */
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_NORMALIZE);
  glShadeModel(GL_SMOOTH);

  /* Enable textures. */
  glEnable(GL_TEXTURE_2D);

  /* Enable Z-buffer. */
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);

  /* Set OpenGL state. */
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  /* Hide the mouse pointer. */
  glutSetCursor(GLUT_CURSOR_NONE);

  build_dlist();
}

void reshape(int _w, int _h) {
  w = _w;
  h = _h;
  /* Recalculate projection matrix. */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, w, h);
  gluPerspective(90, ((float)w)/(h > 0.0f ? ((float)h) : 1.0f), 0.1, 1000);
  glTranslatef(0.0f, 0.0f, -1.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
  switch (key) {
  case 27:
    exit(0);
    break;
  }
}

void idle(void) {
  glutPostRedisplay();
}

void render_scene(void) {
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  glCallList(dlist);
  glFlush();
  glutSwapBuffers();
}

void clean_up(void) {
  glDeleteLists(dlist, 1);
#ifdef OUTPUT_REDIR
  fclose(fout);
  fclose(ferr);
#endif
}
