/**
 * @file main.cpp
 * @author Ulrich Buettemeier
 * @version v0.0.20
 * @date 2021-09-12
 */

#define USE_FULL_SCREEN_

#include <iostream>

#include <GL/glew.h>
#include <GL/glut.h>

#include "basicelement.hpp"
#include "stlcmd.hpp"

using namespace std;

basics *basic;  // Ursprung

int src_w=500, src_h=500;

float eye[3] = {0.0, 0.0, 1.0f};        // camera Position
float look_at[3] = {0.0, 0.0, 0.0};
float up[3] = {0, 1, 0};
float fovy = 30.0f;                     // camera Öffnungswinkel

float buf_eye[3], buf_look_at[3], buf_up[3];    // Wird in mouse_func() und mouse_move() benötigt !!!

bool strg_key = 0, shift = 0;
bool button_0_down = 0;                 // mouse left button
int last_mx=-1, last_my=-1;             // Wird in mouse_func() und mouse_move() benötigt !!!

uint8_t system_is_going_down = 0;       // look at timer(), keyboard()

// ----------- Prototypen -----------------
void help();
void show_special_keys();
void fit_in ();                         // Modell einpassen; up[] bleibt unverändert.
void init_scene();
static void glutResize (int w, int h);
static void glutDisplay ();
// --------- key and mouse function ------------------
void keyboard ( unsigned char key, int x, int y);
void specialkey( int key, int x, int y);
void key_up (int key, int x, int y);
void mouse_func (int button, int state, int x, int y);
void mouse_move (int x, int y);
void passive_mouse_move (int x, int y);

static void timer (int v);

/*********************************************************************
 * @brief   void help()
 */
void help()
{
    cout << "Usage:   ./stlviewer  file, ...\n";
    cout << "Example: ./stlviewer  STL_data/baby-tux_bin.STL\n\n";
    cout << "---- Keyboard shortcuts ----\n";
    cout << "h : this message\n";
    cout << "s : show special key's\n";
    cout << "t : draw triangles ON/OFF\n";
    cout << "l : draw lines ON/OFF\n";
    cout << "p : draw points ON/OFF\n";
    cout << "e : Model einpassen (fit in)\n";
    cout << "v : Vorderansicht\n";
    cout << "\n";
}

/***************************************************************
 * @brief   void show_special_keys()
 */
void show_special_keys()
{
    cout << "+ : zoom plus\n";
    cout << "- : zoom minus\n";
}

/******************************************************************
 * @brief   void fit_in ()  // Modell einpassen
 */
void fit_in ()
{
    float r[3];     // Richtung
    float dist = stlcmd::obj_radius / tan(grad_to_rad(fovy/2.0f));
    vec3sub (eye, look_at, r);      // r: Blickrichtung
    vec3Normalize (r);
    vec3add_vec_mul_fakt (stlcmd::center_ges, r, dist, eye);
    vec3add_vec_mul_fakt (eye, r, -1.0f, look_at);
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
    gluPerspective (fovy, (float)w/(float)h, 0.1, stlcmd::obj_radius * 80);
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

    basic->display();

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
        case 27:            // ESC
            system_is_going_down = 1;
            stlcmd::clear_allstl();         // clear all stl-data
            delete basic;
            glutDestroyWindow(glutGetWindow ());
            break;
        case 'h':
            help();
            break;
        case 's':
            show_special_keys();
            break;
        case '-':           // zoom kleiner
        case '+': {         // zoom größer
            float r[3];     // Richtung
            vec3sub (look_at, eye, r);
            vec3Normalize (r);
            float faktor = (key=='+') ? 5.0f : -5.0f;
            vec3mul_faktor (r, stlcmd::obj_radius/faktor, r);
            vec3add (eye, r, eye);
            vec3add (look_at, r, look_at);
            }
            break;
        case 't':       // draw triangle
        case 'l':       // draw line
        case 'p':       // draw point
            for (size_t i=0; i<stlcmd::allstl.size(); i++) {
                uint8_t akt_mode = stlcmd::allstl[i]->get_draw_mode();
                uint8_t mode;
                if (key == 't') mode = stlcmd::draw_triangle;
                if (key == 'l') mode = stlcmd::draw_line;
                if (key == 'p') mode = stlcmd::draw_point;

                (akt_mode & mode) ? akt_mode &= ~mode : akt_mode |= mode;   // toggle mode

                stlcmd::allstl[i]->set_draw_mode ( akt_mode );
            }
            break;
        case 'e':      // Einpassen (fit in) up[] bleibt unverändert.
            fit_in();
            break;
        case 'v':       // Vorderansicht XY
            vec3set (0.0, 0.0, 1.0f, eye);
            vec3set (0.0, 0.0, 0.0, look_at);
            vec3set (0, 1, 0, up);
            fit_in();
            break;
    }
}

/********************************************************************
 * @brief   void specialkey( int key, int x, int y) 
 */
void specialkey( int key, int x, int y) 
{
    // cout << key << endl;
    switch (key) {
        case 100:           // rechts
        case 102:           // links
            if (!strg_key) {        // rotation cam
                float foo[3];
                float alpha = (key == 100) ? 15.0f : -15.0f;
                if (shift)
                    alpha = (key == 100) ? 90.0f : -90.0f;
                vec3add (stlcmd::center_ges, up, foo);      // ep: foo = stlcmd::center_ges + up
                vec3rot_point_um_achse_II (stlcmd::center_ges, foo, grad_to_rad(alpha), look_at);
                vec3rot_point_um_achse_II (stlcmd::center_ges, foo, grad_to_rad(alpha), eye);
            }
            if (strg_key) {     // move cam left right
                float foo[3], n[3];
                vec3sub (look_at, eye, foo);
                vec3Normalize (foo);
                vec3Normalize (up);
                vec3Cross (foo, up, n);
                vec3Normalize (n);
                float faktor = (key == 100) ? 20.0f : -20.0f;
                vec3add_vec_mul_fakt (eye, n, stlcmd::obj_radius / faktor, eye);
                vec3add_vec_mul_fakt (look_at, n, stlcmd::obj_radius / faktor, look_at);
            }
            break;
        case 101:           // top
        case 103:           // bottom
            if (!strg_key) {            // rotation cam
                float foo[3], n[3], p[3], upp[3];
                float alpha = (key == 103) ? 15.0f : -15.0f;
                if (shift)
                    alpha = (key == 103) ? 90.0f : -90.0f;
                vec3sub (look_at, eye, foo);    // foo: Blickrichtung
                vec3Normalize (foo);
                vec3Normalize (up);
                vec3Cross (up, foo, n);         // n: Normal auf foo und up
                
                vec3add (stlcmd::center_ges, n, p);

                vec3add (eye, up, upp);         // upp=aeye+up  up muss später zurück grechnet werden.
                vec3rot_point_um_achse_II (stlcmd::center_ges, p, grad_to_rad(alpha), look_at);
                vec3rot_point_um_achse_II (stlcmd::center_ges, p, grad_to_rad(alpha), eye);
                vec3rot_point_um_achse_II (stlcmd::center_ges, p, grad_to_rad(alpha), upp);
                vec3sub (upp, eye, up);         // up neu berechnen
            }
            if (strg_key) {         // move cam top down
                float faktor = (key == 103) ? 20.0f : -20.0f;
                vec3Normalize (up);
                vec3add_vec_mul_fakt (eye, up, stlcmd::obj_radius / faktor, eye);
                vec3add_vec_mul_fakt (look_at, up, stlcmd::obj_radius / faktor, look_at);
            }
            break;
        case 112:
            shift = 1;          // SHIFT
            break;
        case 114:               // STRG
            strg_key = 1;
            break;
    }
}

/***************************************************************************************
 * @brief   void key_up (int key, int x, int y) 
 */
void key_up (int key, int x, int y) 
{
    // cout << key << endl;
    switch (key) {
        case 112:           // SHIFT
            shift = 0;
            break;
        case 114:           // STRG
            strg_key = 0;
            break;
  }
}

/*************************************************************************************
 * @brief   void mouse_func (int button, int state, int x, int y)
 */
void mouse_func (int button, int state, int x, int y)
{
    switch (button) {
        case 0:     // button 0 down
            if ((button_0_down = (state == 0) ? 1 : 0)) {  // button down
                last_mx = x, last_my = y;
                vec3copy (eye, buf_eye);            // Cam-Pos sichern. Wird in mouse_move() benötigt !!!
                vec3copy (look_at, buf_look_at);
                vec3copy (up, buf_up);
            } else {    // button 0 up
                // cout << "button up\n";
            }
            
                
            break;
        case 3:     // wheel scrool down
        case 4:     // wheel scroll up
            keyboard ((button == 3) ? '-' : '+', 0, 0);
            break;
        default:
            /**/ cout << button << " | " << state << " | " << x << " | " << y << endl;
            break;
    }
}

/******************************************************************
 * @brief   void mouse_move (int x, int y)
 */
void mouse_move (int x, int y)
{
    static double alpha = 0.0f;      // in [rad]
    static double dist = 0.0f;

    if (button_0_down) {
        if (strg_key) {                 // Move Camera
            double dx = x - last_mx;
            double dy = y - last_my;

            // ---- gepufferte Cam-Koordinaten holen. S.auch mouse_func() -------
            vec3copy (buf_eye, eye);
            vec3copy (buf_look_at, look_at);
            vec3copy (buf_up, up);

            float n[3], foo[3];
            vec3sub (look_at, eye, foo);    // foo[] = Blickrichtungs-Vector
            vec3Normalize (foo);
            vec3Normalize (up);
            vec3Cross (up, foo, n);     

            float faktor = stlcmd::obj_radius/240.0f;
            vec3add_vec_mul_fakt (eye, up, dy*faktor, eye);
            vec3add_vec_mul_fakt (eye, n, dx*faktor, eye);

            vec3add_vec_mul_fakt (look_at, up, dy*faktor, look_at);
            vec3add_vec_mul_fakt (look_at, n, dx*faktor, look_at);
        } else {                        // Rotate Camera
            double dx = x - last_mx;
            double dy = y - last_my;
            dist = sqrtf64(dx*dx + dy*dy);
            
            if (dx != 0) {
                alpha = atanf (dy / dx);
                if (dy >= 0) {
                    if (dx < 0)
                        alpha += M_PI;
                } else      // dy < 0
                    alpha = (dx < 0) ? alpha+M_PI : 2.0*M_PI+alpha;
            }
            else     // dx == 0
                alpha = (dy >= 0) ? M_PI/2.0f : M_PI+M_PI_2;

            // cout << "dx=" << dx << " dy=" << dy << " alpha=" << rad_to_grad(alpha) << " dist=" << dist << endl;

            // ---- gepufferte Cam-Koordinaten holen. S.auch mouse_func() -------
            vec3copy (buf_eye, eye);
            vec3copy (buf_look_at, look_at);
            vec3copy (buf_up, up);

            float n[3], foo[3], p[3], upp[3], np[3] = {0, 0, 0};
            
            vec3sub (look_at, eye, foo);    // foo[] = Blickrichtungs-Vector
            vec3Normalize (foo);
            vec3Normalize (up);
            vec3Cross (foo, up, n);         // Senkrechte von up und foo (Blickrichtung)

            float dpf[3] = {0, 0, 0};   // Faktoren 
            schnittpunkt_gerade_ebene (eye, foo,                // g: eye + foo
                                    stlcmd::center_ges, up, n,  // e: stlcmd::center_ges + up + n;
                                    dpf); 

            float dp[3];
            vec3add_vec_mul_fakt (eye, foo, dpf[2], dp);    // dp (Durchstoßpunkt) brechnen
            
            float ap[3];
            vec3copy (up, ap);      // ap = up

            vec3rot_point_um_achse_II (np,      // np={0,0,0}: neue Rotationsachse um Cam-Richtung foo[] drehen
                                    foo,     // foo[] = Blickrichtungs-Vector
                                    alpha, 
                                    ap);
            vec3add (eye, up, upp);             // Endpunkt von up berechnen => upp = eye + up. Muss anschließend zurückgerechnet werden.

            vec3add (dp, ap, p);     // Endpunkt für Rotationsachse brechnen => p = dp + ap

            // Cam um Rotationsachse (dp[], p[]) drehen.  (dp: Durchstoßpunkt durch Ebene: stlcmd::center_ges, up, n )
            vec3rot_point_um_achse_II (dp, p, grad_to_rad(-dist/2.0f), look_at);    
            vec3rot_point_um_achse_II (dp, p, grad_to_rad(-dist/2.0f), eye);
            vec3rot_point_um_achse_II (dp, p, grad_to_rad(-dist/2.0f), upp);

            vec3sub (upp, eye, up); // up wieder herstellen.
        }
    }
}

/*******************************************************************
 * @brief   void passive_mouse_move (int x, int y)
 */
void passive_mouse_move (int x, int y)
{
    float zbuf_tiefe;
    int y_new = src_h - y -1;

    glReadPixels( x, y_new, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zbuf_tiefe);     // Achtung: Y-Ursprung muß nach bottom gelegt werden !!!

    int viewport[4];
    double modelview[16];
    double projection[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);       //recuperer matrices
    glGetDoublev(GL_PROJECTION_MATRIX, projection);     //recuperer matrices
    glGetIntegerv(GL_VIEWPORT, viewport);               //viewport

    double u, v, w;
	gluUnProject(x, y_new, zbuf_tiefe, modelview, projection, viewport, &u, &v, &w);
    if (zbuf_tiefe != 1) {
        // cout << u << "|" << v << "|" << w << endl;
    }

    // cout << "passive_mouse_move " << x << "|" << y << endl;
}

/******************************************************************
 * @brief   void timer(int v) 
 */
static void timer(int v) 
{
    static uint8_t counter = 0;

    if (!system_is_going_down) {
        glutDisplay();
    }
    glutTimerFunc(unsigned(20), timer, ++v);    // trigger timer
    counter++;
}

/********************************************************************
 * @brief   int main(int argc, char **argv) 
 */
int main(int argc, char **argv) 
{
    help();
    glutInit(&argc, argv);

#ifdef USE_FULL_SCREEN    
    src_w = glutGet ( GLUT_SCREEN_WIDTH );
    src_h = glutGet ( GLUT_SCREEN_HEIGHT );
#endif
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
    glutSpecialFunc ( specialkey );
    glutSpecialUpFunc ( key_up );
    glutMouseFunc ( mouse_func );
    glutMotionFunc ( mouse_move );
    glutPassiveMotionFunc ( passive_mouse_move );

    cout << "OpenGL Version= " << glGetString(GL_VERSION) << endl;

    init_scene();
    stlcmd::init_stlcmd();

    for (int i=1; i<argc; i++) 
        new stlcmd( argv[i] );      // read stl-data

    basic = new basics(stlcmd::obj_radius * 0.5f);
    basic->set_max_quader (stlcmd::min_ges, stlcmd::max_ges);
    basic->set_max_quader (stlcmd::min_ges, stlcmd::max_ges);

    vec3print_vec ("min ges: ", stlcmd::min_ges);
    vec3print_vec ("max ges: ", stlcmd::max_ges);
    vec3print_vec ("center ges: ", stlcmd::center_ges);
    cout << "R: " << stlcmd::obj_radius << endl;
    fit_in();                       // Modell einpassen

    glutTimerFunc(unsigned(20), timer, 0);
    glutMainLoop();

    return EXIT_SUCCESS;
}