/**
 * @file main.cpp
 * @author Ulrich Buettemeier
 * @brief 
 * @version v0.0.3
 * @date 2021-09-12
 */

#include <iostream>

#include <GL/glew.h>
#include <GL/glut.h>

#include "stlcmd.hpp"

using namespace std;

int src_w=500, src_h=500;

float eye[3] = {0.0, 0.0, 3.0f};
float look_at[3] = {0.0, 0.0, 0.0};
float up[3] = {0, 1, 0};

float fovy = 60.0f;

// ----------- Prototypen -----------------
void help(void);
void set_cam_to_center ( void );
void init_scene();
static void glutResize (int w, int h);
static void glutDisplay ();
void keyboard ( unsigned char key, int x, int y);
static void timer (int v);

/*********************************************************************
 * @brief   help()
 */
void help()
{
    cout << "Usage: stlviewer file, ...\n";
    cout << "Example: stlviewer STL_data/baby-tux_bin.STL\n";

    cout << "\n";
}

/*******************************************************************
 * @brief Set the cam to center object
 */
void set_cam_to_center()
{
    float n[3] {0, 0, 1};
    
    float dist = stlcmd::obj_radius / tan(grad_to_rad(fovy/2.0f));

    vec3set (0, 1, 0, up);
    vec3add_vec_mul_fakt (stlcmd::center_ges, n, dist, eye);
    vec3add_vec_mul_fakt (eye, n, -1, look_at);
}

/*********************************************************************************************
 * @brief   void init_scene()
 */
void init_scene()
{
    glEnable ( GL_DEPTH_TEST );
    glEnable ( GL_COLOR_MATERIAL );
    glEnable ( GL_NORMALIZE );        // sorgt dafür, das der Norlavector auf 1.0 normalisiert wird.
    glShadeModel ( GL_SMOOTH );       // GL_FLAT oder GL_SMOOTH; s.a. https://wiki.delphigl.com/index.php/glShadeModel#Beschreibung
    glEnable ( GL_CULL_FACE );
    glFrontFace( GL_CCW );
    glCullFace( GL_BACK );
    glEnable ( GL_ALPHA_TEST );

    glEnable( GL_BLEND );
    // glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
    // glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendFunc(GL_ONE, GL_ONE);
    
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 50.0 };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    GLfloat light0_position[] = {10000.0f, 10000.0f, 20000.0f, 1.0f};
    GLfloat light0_diffuse[] = {1.0, 1.0, 1.0, 1.0};
    
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);

    GLfloat light1_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat light1_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light1_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light1_position[] = { 20000.0, 20000.0, 20000.0, 1.0 };
    GLfloat spot_direction[] = { -0.5, -1.0, 0.0 };

    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);

    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.5);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.5);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.2);

    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 45.0);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 2.0);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
}

/**********************************************************************
 * @brief       void glutResize(int w, int h) 
 */
static void glutResize(int w, int h) 
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective (fovy, (float)w/(float)h, 0.1, 100000.0);
}

/****************************************************************
 * @brief   void glutDisplay()
 */
static void glutDisplay()
{
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // -------- set camera position -----------------
    gluLookAt ( eye[0], eye[1], eye[2],                 // eye
                look_at[0], look_at[1], look_at[2],     // center
                up[0], up[1], up[2] );                  // up

    for (size_t i=0; i<stlcmd::allstl.size(); i++) 
        stlcmd::allstl[i]->display();

    glutSwapBuffers();
    glutReportErrors();
}

/*******************************************************************
 * @brief   void keyboard( unsigned char key, int x, int y) 
 */
void keyboard( unsigned char key, int x, int y) 
{
    switch (key) {
        case 27:                        // ESC
            stlcmd::clear_allstl();         // clear all stl-data
            glutDestroyWindow(glutGetWindow ());
            break;
    }
}

/******************************************************************
 * @brief   void timer(int v) 
 */
static void timer(int v) 
{
  static uint8_t counter = 0;

  glutDisplay();
  glutTimerFunc(unsigned(20), timer, ++v);

  counter++;
}

int main(int argc, char **argv) 
{
    help();
    glutInit(&argc, argv);
    // src_w = glutGet ( GLUT_SCREEN_WIDTH );
    // src_h = glutGet ( GLUT_SCREEN_HEIGHT );
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0,0);
    glutInitWindowSize (src_w, src_h);

    glutCreateWindow("STL-Viewer");
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "Glew error: %s\n", glewGetErrorString(err));
    }

    glutDisplayFunc(glutDisplay);
    glutReshapeFunc(glutResize);

    glutKeyboardFunc ( keyboard );

    init_scene();
    stlcmd::init_stlcmd();

    for (int i=1; i<argc; i++) 
        new stlcmd( argv[i] );      // read stl-data

    vec3print_vec ("min ges: ", stlcmd::min_ges);
    vec3print_vec ("max ges: ", stlcmd::max_ges);
    vec3print_vec ("center ges: ", stlcmd::center_ges);
    set_cam_to_center();

    glutTimerFunc(unsigned(20), timer, 0);
    glutMainLoop();

    return EXIT_SUCCESS;
}