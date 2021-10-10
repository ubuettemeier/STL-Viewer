/**
 * @file main.cpp
 * @brief simple STLViewer
 * @author Ulrich Buettemeier
 * @date 2021-09-12
 */

#define VERSION "v0.3.2"

// Mit USE_FULL_SCREEN wird das Programm mit SCREEN_WIDTH / SCREEN_HEIGHT gestartet.
// #define USE_FULL_SCREEN

#include <iostream>
#include <iomanip>
#include <limits>

#include <GL/glew.h>
#include <GL/glut.h>
// #include <glm/vec3.hpp>  // glm: c++ Vektorrechnung. Wird zur Zeit nicht benötigt. Es wird mat4.hpp genutzt.

#include "basicelement.hpp"
#include "stlcmd.hpp"

using namespace std;

basics *basic;                  // class für Ursprung, Min-Max-Quader und Hauptebenen

int win_w=500, win_h=500;

float eye[3] = {0.0, 0.0, 1.0f};        // camera Position
float look_at[3] = {0.0, 0.0, 0.0};
float up[3] = {0, 1, 0};
float fovy = 30.0f;                     // camera Öffnungswinkel. Winkel < 30° sind eventuell Problematisch

float buf_eye[3], buf_look_at[3], buf_up[3];    // Wird in mouse_func() und mouse_move() benötigt !!!

bool strg_key = 0, shift = 0;
bool button_0_down = 0;                 // mouse left button
int last_mx=-1, last_my=-1;             // Wird in mouse_func() und mouse_move() benötigt !!!
int mouse_x = 0, mouse_y = 0;

uint8_t system_is_going_down = 0;       // look at timer(), keyboard()

struct _pick_buf_ pick_buf = {false, 0, 0, 0};  // Zeigt an, ob und wo ein Element gepickt wurde. Wird in mouse_func() und mouse_move() verwendet.

// ----------- Prototypen -----------------
void help();
void show_options();
void show_special_keys();
void fit_in ();                         // Modell einpassen; up[] bleibt unverändert.
bool get_3D_from_view (int x, int y, float *ret);    // Get the 3D-Data from viewport. 3D result => ret[3]
bool get_2D_from_3Dkoor (float *v, int &x, int &y);
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
 * @brief   Funktion zeigt die wichtigsten shortcuts.
 *          Pfeiltasten sind in der Funktion show_special_keys() dokumentiert.
 */
void help()
{
    cout << "---- Keyboard shortcuts ----\n";
    cout << "h : this message\n";
    cout << "k : show special key's\n";
    cout << "t : draw triangles ON/OFF\n";
    cout << "l : draw lines ON/OFF\n";
    cout << "p : draw points ON/OFF\n";
    cout << "a : draw axis, Max, planes ON/OFF\n";
    cout << "f : Model einpassen (fit in)\n";
    cout << "v : Vorderansicht XY-plane\n";
    cout << "d : Draufsicht XZ-plane\n";
    cout << "s : Seitenansicht von links YZ-plane\n";
    cout << "o : optimiere Normal-Vektoren\n";
    cout << "c : draw Flächenrückseite (back face) on/off\n";
    cout << "\n";
}

/********************************************************************
 * @brief   show comamnd line options
 */
void show_options()
{
    cout << "Usage:   ./stlviewer  file, ...\n";
    cout << "Example: ./stlviewer  STL_data/baby-tux_bin.STL\n\n";
}

/***************************************************************
 * @brief   show special key's
 */
#define LEERSTELLEN 20
void show_special_keys(){
    cout << "            +/- : zoom\n";
    cout << "        →|←|↑|↓ : rotation 15°\n";
    cout << "Shift + →|←|↑|↓ : rotation 90°\n";
    cout << " Strg + →|←|↑|↓ : move\n";
    cout << "\n";
    cout << "mouse left button        : rotation\n";
    cout << "mouse wheel              : zoom\n";
    cout << "Strg + mouse left button : move\n";
    cout << "\n";
}

/******************************************************************
 * @brief   fit model in screen
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

/***********************************************************************
 * @brief Get the 2D from 3D Koordinate
 */
bool get_2D_from_3Dkoor (float *k, int &x, int &y)
{
    bool ret = true;
    GLdouble u, v, w;

    int viewport[4];
    double modelview[16];
    double projection[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);       //recuperer matrices
    glGetDoublev(GL_PROJECTION_MATRIX, projection);     //recuperer matrices
    glGetIntegerv(GL_VIEWPORT, viewport);               //viewport

    gluProject (k[0], k[1], k[2],
                modelview,
                projection,
                viewport,
                &u, &v, &w);
    x = u;
    y = win_h - v;

    return ret;
}

/**************************************************************************************
 * @brief Get the 3D-Data from viewport
 * @param ret wenn keine 3D bei xy liegen ist die z-Komponente von ret[2] = std::numeric_limits<float>::lowest()  ????
 * @return true: 3D select;  false: no geometrie-data at xy
 */
#define SHOW_KONTROL_AUSGABE_

bool get_3D_from_view (int x, int y, float *ret)
{
    bool is_sell = true;
    float zbuf_tiefe;
    int y_new = win_h - y -1;   // Achtung: viewplane hat den Ursprung unten-links! Window (x, y) hat den Ursprung oben-links.

    glReadPixels( x, y_new, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zbuf_tiefe);     // Achtung: Y-Ursprung muß nach bottom gelegt werden !!!

    int viewport[4];
    double modelview[16];
    double projection[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);       //recuperer matrices
    glGetDoublev(GL_PROJECTION_MATRIX, projection);     //recuperer matrices
    glGetIntegerv(GL_VIEWPORT, viewport);               //viewport

    double u, v, w;
    gluUnProject(x, y_new, zbuf_tiefe, modelview, projection, viewport, &u, &v, &w);

    if (zbuf_tiefe == 1.0f) {       // es wurde kein 3D Punkt gefunden
        is_sell = false;
        w = std::numeric_limits<float>::lowest();
    }
    vec3set (u, v, w, ret);

#ifdef SHOW_KONTROL_AUSGABE
    if (is_sell) {
        int i, j;
        get_2D_from_3Dkoor (ret, i, j);
        /**/ cout << u << "|" << v << "|" << w << endl;
        /**/ cout << "i: " << i << " j: " << j << endl;
        /**/ cout << "x: " << x << " y: " << y << endl;
    }
#endif

    return is_sell;
}

/*********************************************************************************************
 * @brief   initial OpenGl-mode's
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

    glEnable( GL_BLEND );   // Farbmischung einschalten
    // --- glBlendFunc beeinflusst das Mischen der Farben im Framebuffer
    // --- s.a. https://wiki.delphigl.com/index.php/glBlendFunc

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
 * @brief   set perspective
 */
static void glutResize(int w, int h) 
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // gluPerspective (GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);
    gluPerspective (fovy, (float)w/(float)h, 0.1, stlcmd::obj_radius * 80); 

    win_w = w;
    win_h = h;
}

/****************************************************************
 * @brief   display all
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

    basic->display();

    glutSwapBuffers();
    glutReportErrors();
}

/*******************************************************************
 * @brief   callback by standard Key
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
        case 'q': {      // 'q' use for test'
            
            } break;
        case 'c': {     // draw Flächenrückseite (back face) on/off
            GLboolean foo;
            glGetBooleanv (GL_CULL_FACE, &foo);
            foo ? glDisable ( GL_CULL_FACE ) : glEnable ( GL_CULL_FACE );
            glGetBooleanv (GL_CULL_FACE, &foo);
            cout << "draw back face= " << (foo ? "false" : "true") << endl;
            }
            break;
        case 'o': 
            stlcmd::optimise_all_normal_vec(); 
            break;
        case 'h':
            help();
            break;
        case 'k':
            show_special_keys();
            break;
        case '-':           // zoom kleiner
        case '+':           // zoom größer
            if (!shift && !strg_key) {
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
        case 'a':
            basic->draw_basics = !basic->draw_basics;
            break;
        case 'f':      // Einpassen (fit in) up[] bleibt unverändert.
            fit_in();
            break;
        case 'v':       // Vorderansicht XY plane
            if (!shift && !strg_key) {
                vec3set (0.0, 0.0, 1.0f, eye);
                vec3set (0.0, 0.0, 0.0, look_at);
                vec3set (0, 1, 0, up);
                fit_in();
            }
            break;
        case 'd':       // Draufsicht XZ plane
            if (!shift && !strg_key) {
                keyboard ('v', 0, 0);
                shift = 1;
                specialkey (103, 0, 0);
                shift = 0;
                fit_in();
            }
            break;
        case 's':       // Seitenansicht  YZ plane
            if (!shift && !strg_key) {
                keyboard ('v', 0, 0);
                shift = 1;
                specialkey (102, 0, 0);
                shift = 0;
                fit_in();
            }
            break;
    }
}

/********************************************************************
 * @brief   callback by special Key
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
                float faktor = (key == 100) ? 20.0f : -20.0f;               // Bewegungsrichtung festlegen !
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
                float faktor = (key == 103) ? 20.0f : -20.0f;           // Bewegungsrichtung festlegen !
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
 * @brief   callback by Key Up
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
 * @brief   callback by mouse-click's
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
                float foo[3];
                if ((pick_buf.ist_aktiv = get_3D_from_view (x, y, foo))) {  // Nachschauen, ob ein Element gepickt wurde.
                    vec3copy (foo, pick_buf.pv);    
                }
            } else      // button 0 up
                glutSetCursor( GLUT_CURSOR_RIGHT_ARROW );

            break;
        case 3:     // wheel scrool down
        case 4: {   // wheel scroll up
                float foo[3];
                float faktor = (button==4) ? 10.0f : -10.0f;

                if ((get_3D_from_view (x, y, foo) == false) ||        // es ist mit der Funktion <get_3D_from_view()> kein 3D Punkt gefunden worden !
                    (button == 3)) {                // zoom erfolgt in Richtung look_at
                    float r[3];     
                    vec3sub (look_at, eye, r);      // Richtung ermitteln. r[] zeigt in Richtung look_at[]
                    vec3Normalize (r);
                    vec3mul_faktor (r, stlcmd::obj_radius/faktor, r);
                    vec3add (eye, r, eye);
                    vec3add (look_at, r, look_at);
                } else {            // die Funktion <get_3D_from_view()> hat ein Punkt gefunden !
                                    // zoom erfolgt in Richtung foo[3] !!!
                    float r[3];     
                    vec3sub (foo, eye, r);      // Richtung ermitteln. r[] zeigt auf foo[].
                    vec3Normalize (r);
                    vec3mul_faktor (r, stlcmd::obj_radius/faktor, r);
                    vec3add (eye, r, eye);
                    vec3add (look_at, r, look_at);
                }
            }
            break;
        default:
            // cout << button << " | " << state << " | " << x << " | " << y << endl;
            break;
    }
}

/******************************************************************
 * @brief   callback by mouse-move with button-down
 */
void mouse_move (int x, int y)
{
    static double alpha = 0.0f;      // in [rad]
    static double dist = 0.0f;

    mouse_x = x;
    mouse_y = y;

    if (button_0_down) {
        if (strg_key) {                 // Move Camera
            double dx = x - last_mx;
            double dy = y - last_my;

            glutSetCursor( GLUT_CURSOR_CROSSHAIR );
            // ---- gepufferte Cam-Koordinaten holen. S.auch mouse_func() -------
            vec3copy (buf_eye, eye);
            vec3copy (buf_look_at, look_at);
            vec3copy (buf_up, up);

            float n[3], foo[3];
            vec3sub (look_at, eye, foo);    // foo[] = Blickrichtungs-Vector
            vec3Normalize (foo);
            vec3Normalize (up);
            vec3Cross (up, foo, n);     

            float faktor = stlcmd::obj_radius / 240.0f;     // 240.0f ist willkürlich. Hier ist noch Handlungsbedarf !!!
            vec3add_vec_mul_fakt (eye, up, dy*faktor, eye);
            vec3add_vec_mul_fakt (eye, n, dx*faktor, eye);

            vec3add_vec_mul_fakt (look_at, up, dy*faktor, look_at);
            vec3add_vec_mul_fakt (look_at, n, dx*faktor, look_at);
        } else {                        // Rotate Camera
            double dx = x - last_mx;
            double dy = y - last_my;
            dist = sqrt(dx*dx + dy*dy);
            
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
            float dp[3];                // Durchstoßpunkt.
            if (pick_buf.ist_aktiv) {
                vec3copy (pick_buf.pv, dp);     // pick-Punkt = Drehpunkt
                glutSetCursor( GLUT_CURSOR_CYCLE );
            } else {
                schnittpunkt_gerade_ebene (eye, foo,                // g: eye + foo
                                           stlcmd::center_ges, up, n,  // e: stlcmd::center_ges + up + n;
                                           dpf); 
                vec3add_vec_mul_fakt (eye, foo, dpf[2], dp);    // dp (Durchstoßpunkt) brechnen
            }
            float ap[3];
            vec3copy (up, ap);      // ap = up

            vec3rot_point_um_achse_II (np,      // np={0,0,0}: neue Rotationsachse um Cam-Richtung foo[] drehen
                                    foo,     // foo[] = Blickrichtungs-Vector
                                    alpha, 
                                    ap);

            vec3add (dp, ap, p);     // Endpunkt für Rotationsachse brechnen => p = dp + ap

            vec3add (eye, up, upp);  // Endpunkt von up berechnen => upp = eye + up. Muss anschließend zurückgerechnet werden.
            // Cam um Rotationsachse (dp[], p[]) drehen.  (dp: Durchstoßpunkt durch Ebene: stlcmd::center_ges, up, n )
            vec3rot_point_um_achse_II (dp, p, grad_to_rad(-dist/2.0f), look_at);    
            vec3rot_point_um_achse_II (dp, p, grad_to_rad(-dist/2.0f), eye);
            vec3rot_point_um_achse_II (dp, p, grad_to_rad(-dist/2.0f), upp);

            vec3sub (upp, eye, up); // up wieder herstellen.
        }
    }
}

/*******************************************************************
 * @brief   callback by mouse-move without button-click
 */
void passive_mouse_move (int x, int y)
{
    mouse_x = x;
    mouse_y = y;

    /**** Funktionalität wird noch nicht benötigt ******
    float foo[3];
    get_3D_from_view (x, y, foo);
    vec3print_vec ("foo=", foo);
    */
}

/******************************************************************
 * @brief   timer callback. Ist auf 20ms eingestellt.
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
 * @brief   
 */
int main(int argc, char **argv) 
{
    if (argc < 2)
        show_options();
    
    help();
    glutInit(&argc, argv);

#ifdef USE_FULL_SCREEN    
    win_w = glutGet ( GLUT_SCREEN_WIDTH );      // get screen width
    win_h = glutGet ( GLUT_SCREEN_HEIGHT );     // get screen height
#endif
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0,0);
    glutInitWindowSize (win_w, win_h);

    char buf[80];
    sprintf (buf, "STL-Viewer %s", VERSION);
    glutCreateWindow(buf);

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
    stlcmd::init_stlcmd();      // static's initialisieren

    for (int i=1; i<argc; i++) 
        new stlcmd( argv[i] );      // read stl-data

    basic = new basics(stlcmd::obj_radius * 0.5f);                      // Koordinatenkreus anlegen
    basic->set_max_quader (stlcmd::min_ges, stlcmd::max_ges);           // MAX-Quader anlegen
    basic->set_hauptebene (stlcmd::center_ges, stlcmd::obj_radius);     // Haupebenen anlegen.

    vec3print_vec ("min ges: ", stlcmd::min_ges);
    vec3print_vec ("max ges: ", stlcmd::max_ges);
    vec3print_vec ("center ges: ", stlcmd::center_ges);
    /**/ cout << "R: " << stlcmd::obj_radius << endl;
    /**/ cout << "Anz. triangle: " << stlcmd::get_anz_all_triangle() << endl;

    fit_in();                       // Model einpassen

    glutTimerFunc(unsigned(20), timer, 0);
    glutMainLoop();

    return EXIT_SUCCESS;
}