/**
 * @file stlcmd.cpp
 * @author Ulrich Buettemeier
 * @version v0.0.25
 * @date 2021-09-13
 * @todo select buffer bereinigen 24.Okt.2021
 */

#ifndef STLCMD_HPP
#define STLCMD_HPP

// mit SHOW_CONTROL_TEXT lassen sich Kontroll-Texte anzeigen
// #define SHOW_CONTROL_TEXT

#define MEM(x) ((float*) malloc (sizeof(float)*(x)))
#define ANZ_OBJ 3

#include <iostream>
#include <vector>
#include <fstream>
#include <cstring>
#include <cfloat>
#include <limits>
#include <time.h>               // contains clock()

#include <GL/glew.h>
#include <GL/glut.h>

#include "stldev.h"
#include "utilities.hpp"
#include "mat4.hpp"

using namespace std;

//! ----------------------------------------------------------
//! @brief  Struktur binär STL-File 
//!         uint8_t[80]     - Dateikopf (Header)
//!         uint32_t        - Anzahl Dreiecke
//!
//!         struct _stl_bin_triangle_   - Triangle Struktur (s. stldev.h)
//! ----------------------------------------------------------

class stlcmd {
public:
    enum {draw_triangle = 0x01, draw_line = 0x02, draw_point = 0x04};

    stlcmd (string filename);
    ~stlcmd ();
    stlcmd(stlcmd&) = delete;               // Kopierkonstruktor unterbinden
    void operator=(stlcmd&) = delete;       // Zuweisungsoperator verbieten

    void display ();
    void set_draw_mode (uint8_t mode);
    uint8_t get_draw_mode ();
    void set_color (float *c);
    void set_color (float r, float g, float b, float a);

// ------------- static functions ------------------------------
    static void *operator new (std::size_t size);
    static vector<stlcmd *> allstl;
    static void init_stlcmd();
    static void clear_allstl();
    static uint64_t get_anz_all_triangle();
    static void optimise_all_normal_vec();      // Funktion optimiert den Normalvektor. 

    /*********** static Funktionen für select mechanismus ************/
    static void grep_triangle (float x, float y, float z);  // Sucht das Dreieck zum Punkt xyz, s.auch >grep_stl_triangle()>
    static void clear_sel_buf ();                           // Die Dreiecks-Selection wird aufgehoben
    static void kill_sel_triangle ();

private:
    size_t grep_index (uint32_t surch_id);
    bool read_stl (std::string fname);
    bool read_tex_stl (std::string fname);
    bool read_bin_stl (std::string fname);
    void move_bin_stl_to_stlvec (struct _stl_bin_triangle_ stb);
    void get_min_max_center();
    void calc_max_r();
    void get_min_max_center_ges();      // berechnet den Gesamt Schwerpunkt.
    void make_line_vertex();
    void make_triangle_center();
    void show_fortschritt_prozent_step (uint32_t &counter, bool set_zero = false, int step_width_prozent = 5);
    bool get_gleich_index (std::vector< std::vector<uint32_t> > &index, int n);
    void optimise_normal_vec();             // Funktion optimiert den Normalvektor. 

    /*********** Funktionen für select mechanismus ************/
    bool grep_stl_triangle (float *v);
    struct _hilfs_container_ get_min_hc (std::vector<struct _hilfs_container_> hc);
    void set_select_col (long unsigned int index);
    void clear_all_sel_triagle ();                      // Selection wird aufgehoben    
    void kill_all_sel_triagle();

    uint32_t id;    
    string filename;
    bool init_by_new;
    uint8_t draw_mode = draw_triangle;

    std::vector <struct _vertex_> stlvec;   // vertex buffer for triangle, point
    std::vector <struct _vertex_> stlline;  // vertex buffer for line
    std::vector <struct _vertex_only_> tri_center;
    
    float *col = MEM(4);
    float *center = MEM(3);
    float *min = MEM(3);
    float *max = MEM(3);
    float *stl_m = MEM(16);

    GLuint vaoID[ANZ_OBJ] = {0, 0, 0};    // VAO einrichten; triangle, line, ???
    GLuint vboID[ANZ_OBJ] = {0, 0, 0};    // VBO einrichten; triangle, line, ???

    // ------------ static's ------------------------
    static uint32_t id_counter;         // used for <id>
    static bool use_new;                // true: new stlcmd("xxx"); false: stlcmd a("xxx");
public:    
    static float *center_ges;
    static float *min_ges;
    static float *max_ges;
    static float obj_radius;
    static long unsigned int all_sel_count;
};

// ---------- init static's ----------------
float *stlcmd::center_ges = MEM(3);
float *stlcmd::min_ges = MEM(3); 
float *stlcmd::max_ges = MEM(3);
float stlcmd::obj_radius = sqrt(2.0f);
long unsigned int stlcmd::all_sel_count = 0;

bool stlcmd::use_new = false;
uint32_t stlcmd::id_counter = 0;
vector<stlcmd *> stlcmd::allstl {};

/**********************************************************
 * @brief Construct a new stlcmd::stlcmd object
 */
stlcmd::stlcmd (string fname)
{
    id = ++id_counter;
    init_by_new = use_new;

    filename = fname;
    mat4Identity (stl_m);                   // set view matrix
    vec4set (0.5f, 0.5f, 0.5f, 1.0f, col);      // set color
    vec3set (1.0f, 1.0f, 1.0f, max);
    vec3set (-1.0f, -1.0f, -1.0f, min);
    vec3set (0.0f, 0.0f, 0.0f, center);

    if (read_stl(filename)) {
        get_min_max_center ();
        set_color ( col );
        make_triangle_center ();
        make_line_vertex ();

        glGenVertexArrays(ANZ_OBJ, vaoID);  // create the Vertex Array Objects
        glGenBuffers(ANZ_OBJ, vboID);       // generating Vertex Buffer Objects (VBO)

        // ------------------- Triangles, Points auf VAO 0 --------------------------
        glBindVertexArray(vaoID[0]);                    // VAO 0
        glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);        // VBO 0
        glBufferData(GL_ARRAY_BUFFER, stlvec.size() *sizeof(struct _vertex_), stlvec.data(), GL_STATIC_DRAW);

        int stride = sizeof(struct _vertex_);           // (3+3+4+2) * float
        char *offset = (char*)NULL;

        glVertexPointer(3, GL_FLOAT, stride, offset);   // position  3*float
        glEnableClientState(GL_VERTEX_ARRAY);
        
        offset = (char*)NULL + 3*sizeof(float);
        glNormalPointer (GL_FLOAT, stride, offset);     // normal
        glEnableClientState(GL_NORMAL_ARRAY);

        offset = (char*)NULL + 6*sizeof(float);         // color
        glColorPointer(4, GL_FLOAT, stride, offset);    // 4*float
        glEnableClientState(GL_COLOR_ARRAY);

        // ------------------- Linien auf VAO 1 --------------------------
        glBindVertexArray(vaoID[1]);                    // VAO 1
        glBindBuffer(GL_ARRAY_BUFFER, vboID[1]);        // VBO 1
        glBufferData(GL_ARRAY_BUFFER, stlline.size() *sizeof(struct _vertex_), stlline.data(), GL_STATIC_DRAW);

        stride = sizeof(struct _vertex_);           // int stride = sizeof(Vertex);
        offset = (char*)NULL;

        glVertexPointer(3, GL_FLOAT, stride, offset);   // position  3*float
        glEnableClientState(GL_VERTEX_ARRAY);
        
        offset = (char*)NULL + 3*sizeof(float);
        glNormalPointer (GL_FLOAT, stride, offset);     // normal
        glEnableClientState(GL_NORMAL_ARRAY);

        offset = (char*)NULL + 6*sizeof(float);         // color
        glColorPointer(4, GL_FLOAT, stride, offset);    // 4*float
        glEnableClientState(GL_COLOR_ARRAY);

    } else 
        cout << "ERROR: File <" << filename << "> not exist\n";

#ifdef SHOW_CONTROL_TEXT
    cout << this->filename << " ID=" << id << " use_new=" << use_new << endl;
#endif

    allstl.push_back ( this );      // mit new angelegte Daten merken!
    get_min_max_center_ges();
    use_new = false;
}

/**********************************************************
 * @brief Destroy the stlcmd::stlcmd object
 */
stlcmd::~stlcmd ()
{
#ifdef SHOW_CONTROL_TEXT   
    cout << "kill ID=" << id << endl;
#endif

    free (col);
    free (center);
    free (min);
    free (max);
    free (stl_m);
}

/****************************************************************
 * @brief   static void *stlcmd::operator new (std::size_t size)
 */
void *stlcmd::operator new (std::size_t size)
{
    use_new = true;
    return ::operator new(size);
}

/********************************************************
 * @brief   static void stlcmd::clear_allstl()
 */
void stlcmd::clear_allstl()
{
    for (size_t i=0; i<allstl.size(); i++) {
        if (allstl[i]->init_by_new)
            delete allstl[i];
        else 
            allstl[i]->~stlcmd();
    }
}

/**************************************************************
 * @brief   static uint64_t get_anz_all_triangle();
 *          Function ermittelt Anzahl aller triangles
 */
uint64_t stlcmd::get_anz_all_triangle()
{
    uint64_t ret = 0;
    for (size_t i=0; i<allstl.size(); i++) 
        ret += allstl[i]->stlvec.size() / 3;

    return ret;
}

/********************************************************************
 * @brief   
 * @param   c = float[4]
 */
void stlcmd::set_color (float *c) 
{
    vec4copy (c, col);

    if (vboID[0] != 0) {
        for (size_t i=0; i<stlvec.size(); i++) 
            vec4copy (c, stlvec[i].c);
        glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);       // VBO 0
        glBufferData(GL_ARRAY_BUFFER, stlvec.size() *sizeof(struct _vertex_), stlvec.data(), GL_STATIC_DRAW);
    }

    if (vboID[1] != 0) {
        for (size_t i=0; i<stlline.size(); i++)
            vec4copy (c, stlline[i].c);
        glBindBuffer(GL_ARRAY_BUFFER, vboID[1]);       // VBO 1
        glBufferData(GL_ARRAY_BUFFER, stlline.size() *sizeof(struct _vertex_), stlline.data(), GL_STATIC_DRAW);        
    }
}

/***************************************************
 * 
 */
void stlcmd::set_color (float r, float g, float b, float a)
{
    float foo[4] = {r, g, b, a};
    set_color (foo);
}

/********************************************************
 * 
 */
void stlcmd::show_fortschritt_prozent_step (uint32_t &counter, bool set_zero, int step_width_prozent)
{
    static int status = 0;  // 0%
    static int step_width = 5;
    static int zei_index = 0;
    char zei[5] = {"-\\|/"};

    if (set_zero) {
        status = 0;
        step_width = step_width_prozent;
        cout << "\n";
        return;
    }

    counter = 0;
    status += step_width;

    if (status <= 100) {
        cout << zei[zei_index] << " " << status << "%  \r";
        cout.flush();
        if (++zei_index > 3) zei_index = 0;
    }
}

/*********************************************************************
 * 
 */
bool stlcmd::get_gleich_index (std::vector< std::vector<uint32_t> > &index, int n)
{
    bool ret = false;
    int i=index.size() - 1;

    while ((i >= 0) && !ret) {      // Index-Liste rückwärts durchlaufen !
        if (vec3_equal (stlvec[index[i][0]].v, stlvec[n].v)) {      // Sind Vektoren gleich ?
            index[i].push_back(n);                                  // Vektor an Index-Listen[i] anhängen.
            ret = true;
        }
        i--;
    }

    return ret;
}

/***************************************************************
 * @brief   Funktion optimiert den Normalvektor. 
 *          Wenn der Schnittwinkel 2er Fächen kleiner als 45° ist, 
 *          wird für den Normalvektor ein Mittelwert berechnet.
 */
void stlcmd::optimise_normal_vec()
{
    std::vector<bool> is_use(stlvec.size());
    std::vector< std::vector<uint32_t> > index;
    std::vector< uint32_t > w;
    uint32_t counter = 0;
    uint64_t soll = 0;

    // ----------- Zeitmessung starten -----------------
    double time1=0.0, tstart;      // time measurment variables
    tstart = clock();              // start 

    // Werte für Fortschrittsanzeige berechnen.
    soll = stlvec.size() * 0.05;   // 5% Fortschrittsanzeige
    show_fortschritt_prozent_step (counter, true, 5);  // Fortschrittsanzeige initialisieren

    // ---------------------- gleiche vertexe finden ----------------------
    for (size_t n=0; n<stlvec.size(); n++) {
        if (!get_gleich_index(index, n)) {      // Index-Liste nach gleichen Vektor durchsuchen.
            std::vector<uint32_t> w;
            w.push_back (n);
            index.push_back (w);
        }

        if (++counter > soll)          // Forschrittsanzeige
            show_fortschritt_prozent_step (counter);
    }
    cout << endl;
    
    // ----------- Zeitmessung stopen -----------------
    time1 += clock() - tstart;              // time measurment end
    time1 = time1/CLOCKS_PER_SEC;           // rescale to seconds
    cout << "  time = " << time1 << " sec." << endl;        // Zeit ausgeben

    // -------------------------- Normalvectoren Mittelwert berechnen ------------------
    for (size_t n=0; n<index.size(); n++) {
        w.clear();
        w = index[n];
        bool do_optimize = true;

        if (w.size() > 1) {
            for (size_t i=0; i<w.size()-1; i++) {
                for (size_t n=i+1; n<w.size(); n++) {
                    float alpha = vec3alpha (stlvec[w[i]].n, stlvec[w[n]].n);
                    if (alpha > 0.8f)           // Vektorwinkel > 0,8 rad (45°)
                        do_optimize = false;
                }
            }

            if (do_optimize) {
                float foo[3] = {0.0f};
                for (size_t i=0; i<w.size(); i++) 
                    vec3add (foo, stlvec[w[i]].n, foo);

                vec3mul_faktor (foo, 1.0f/w.size(), foo);   // Mittelwert berechnen

                for (size_t i=0; i<w.size(); i++) 
                    vec3copy (foo, stlvec[w[i]].n);
            }
        }
    }
    // --------------------- vertex Daten im Buffer ablegen ----------------------------
    if (vboID[0] != 0) {
        glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);       // VBO 0
        glBufferData(GL_ARRAY_BUFFER, stlvec.size() *sizeof(struct _vertex_), stlvec.data(), GL_STATIC_DRAW);
    }
    cout << "\n";
}

/******************************************************************
 * @brief   static void optimise_all_normal_vec();
 */
void stlcmd::optimise_all_normal_vec()
{
    for (size_t i=0; i<allstl.size(); i++) {
        allstl[i]->optimise_normal_vec();   
    }
}

/*************************************************************************************************
 * @brief   für alle selectierten Flächen wird die Selektion aufgehoben
 */
void stlcmd::clear_all_sel_triagle()
{
    for (size_t i=0; i<stlvec.size(); i+=3) {
        if (stlvec[i].attrib & 0x01) {
            stlvec[i].attrib &= 0xFE;
            set_select_col (i);
        }
    }
    stlcmd::all_sel_count = 0;

    if (vboID[0] != 0) {
        glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);       // VBO 0
        glBufferData(GL_ARRAY_BUFFER, stlvec.size() *sizeof(struct _vertex_), stlvec.data(), GL_STATIC_DRAW);
    }
}

/******************************************************************
 */
void stlcmd::kill_all_sel_triagle()
{
    size_t i = 0;
    while (i < stlvec.size()) {
        if (stlvec[i].attrib & 0x01) {
            stlvec.erase (stlvec.begin()+i, stlvec.begin()+i+3);    // trinagle löschen, d.h. 3 vertexe löschen
        } else 
            i += 3;
    }
    
    stlcmd::all_sel_count = 0;

    get_min_max_center ();
    make_triangle_center ();
    make_line_vertex ();

    get_min_max_center_ges();

    if (vboID[0] != 0) {
        glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);       // VBO 0
        glBufferData(GL_ARRAY_BUFFER, stlvec.size() *sizeof(struct _vertex_), stlvec.data(), GL_STATIC_DRAW);
    }
    // ------------------- Linien auf VAO 1 --------------------------
    if (vboID[1] != 0) {
        glBindBuffer(GL_ARRAY_BUFFER, vboID[1]);        // VBO 1
        glBufferData(GL_ARRAY_BUFFER, stlline.size() *sizeof(struct _vertex_), stlline.data(), GL_STATIC_DRAW);
    }
}

/*******************************************************************
 */
void stlcmd::set_select_col (long unsigned int i)
{
    if (!(stlvec[i].attrib & 0x01)) {
        vec4set(col[0], col[1], col[2], col[3], stlvec[i].c);
        vec4set(col[0], col[1], col[2], col[3], stlvec[i+1].c);
        vec4set(col[0], col[1], col[2], col[3], stlvec[i+2].c);
        if (all_sel_count) 
            all_sel_count--;
    } else {                                        
        vec4set(1, 0, 0, 1, stlvec[i].c);       // Fläche selektieren
        vec4set(1, 0, 0, 1, stlvec[i+1].c);
        vec4set(1, 0, 0, 1, stlvec[i+2].c);
        all_sel_count++;
    }
}

/***********************************************************************
 * @brief Get the min hc object
 */
struct _hilfs_container_ stlcmd::get_min_hc (std::vector<struct _hilfs_container_> hc)
{
    struct _hilfs_container_ ret;
    double min = 999999999.9f;

    for (size_t i=0; i<hc.size(); i++) {
        if (fabs(1.0f - hc[i].f0) < min) {
            ret = hc[i];
            min = fabs (1.0f - ret.f0);
        }
    }

    return ret;
}
/************************************************************************
 * @brief 
 */
bool stlcmd::grep_stl_triangle (float *v)
{
    bool ret = 0;
    float ab[3], ac[3];
    int counter = 0;
    double m0;
    double f[3];
    std::vector <struct _hilfs_container_> hc;      // Trefferliste

    for (size_t i=0; i<stlvec.size(); i+=3) {
        vec3sub (stlvec[i+1].v, stlvec[i].v, ab);
        vec3sub (stlvec[i+2].v, stlvec[i].v, ac);

        m0 = DET_3A (stlvec[i].v[0], ab[0], ac[0],
                     stlvec[i].v[1], ab[1], ac[1],
                     stlvec[i].v[2], ab[2], ac[2]);

        if (m0 != 0.0f) {
            f[0] = DET_3A (v[0], ab[0], ac[0],
                           v[1], ab[1], ac[1],
                           v[2], ab[2], ac[2]);
            f[0] /= m0;
            // cout << f[0] << endl;
            if ((f[0] > 0.97f) && (f[0] < 1.03)) {              // Im Idealfall beträgt f[0] == 1.0f
                f[1] = DET_3A (stlvec[i].v[0], v[0], ac[0],
                            stlvec[i].v[1], v[1], ac[1],
                            stlvec[i].v[2], v[2], ac[2]);
                f[2] = DET_3A (stlvec[i].v[0], ab[0], v[0],
                            stlvec[i].v[1], ab[1], v[1],
                            stlvec[i].v[2], ab[2], v[2]);

                f[1] /= m0;
                f[2] /= m0;
                if ((f[1] >= 0.0) && (f[1] <= 1.0) &&
                    (f[2] >= 0.0) && (f[2] <= 1.0) &&
                    ((f[1]+f[2]) <= 1.0)) {
                        hc.push_back( {i, f[0] } );     // Dreieck in die Trefferliste eintragen.
                        counter++;
                        // cout << "f[0]=" << f[0] << endl;
                }
            }
        } 
    }

    // -------------- Es sind Flächen gefunden worden ---------------------------
    if (hc.size()) {

        struct _hilfs_container_ foo;
        if (hc.size() == 1)             // Es ist nur 1 Fläche gefunden worden !
            foo = hc[0];
        else                            // Es sind mehrere Flächen gefunden worden.
            foo = get_min_hc (hc);      // Dreieck mit dem genausten f[0] suchen !

        // stlvec[foo.i].attribute.is_sel = !stlvec[foo.i].attribute.is_sel;   // toogle is_sel
        stlvec[foo.i].attrib = (stlvec[foo.i].attrib & 0x01) ? stlvec[foo.i].attrib & 0xFE : stlvec[foo.i].attrib |= 0x01;
        set_select_col (foo.i);         // Farbe anpassen.

        if (vboID[0] != 0) {
            glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);       // VBO 0
            glBufferData(GL_ARRAY_BUFFER, stlvec.size() *sizeof(struct _vertex_), stlvec.data(), GL_STATIC_DRAW);
        }
    }

    return ret;
}

/****************************************************************
 * @brief   static void stlcmd::grep_triangle (float x, float y, float z)
 *          Das selectierte Dreieck wird in selbuf eingetragen !
 */
void stlcmd::grep_triangle (float x, float y, float z)
{
    float v[3];

    vec3set (x, y, z, v);
    for (size_t i=0; i<allstl.size(); i++) {
        allstl[i]->grep_stl_triangle ( v );
    }
}

/************************************************************
 * @brief   static void stlcmd::clear_sel_buf ()
 *          Die Dreiecks-Selection wird aufgehoben
 */
void stlcmd::clear_sel_buf ()
{
    for (size_t i=0; i<allstl.size(); i++) {
        allstl[i]->clear_all_sel_triagle ();
    }
}

/*********************************************************
 * 
 */
void stlcmd::kill_sel_triangle ()
{
    for (size_t i=0; i<allstl.size(); i++) {
        allstl[i]->kill_all_sel_triagle();
    }
}

/**********************************************************
 * @brief   draw OpenGL VBA's
 */
void stlcmd::display ()
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();                         // MODELVIEW sichern

    glMultMatrixf ( stl_m );                // MODELVIEW mit Bauteil-Matrix (stl_m)  multiplizieren 

    // ---------------- draw triangle --------------------
    if (draw_mode & draw_triangle) {
        glBindVertexArray(vaoID[0]);             // bind pyramid VAO
        glDrawArrays(GL_TRIANGLES, 0, stlvec.size());   // render data
    }
    // ---------------- draw line --------------------
    if (draw_mode & draw_line) {
        glBindVertexArray(vaoID[1]);             // bind pyramid VAO
        glDisable (GL_LIGHTING);
        glLineWidth (1);
        glDrawArrays(GL_LINES, 0, stlline.size());   // render data
        glEnable (GL_LIGHTING);
    }
    // ---------------- draw point --------------------
    if (draw_mode & draw_point) {
        glBindVertexArray(vaoID[0]);             // bind pyramid VAO
        glDisable (GL_LIGHTING);
        glPointSize (5);
        glDrawArrays(GL_POINTS, 0, stlvec.size());   // render data
        glEnable (GL_LIGHTING);
    }
    glPopMatrix();                          // MODELVIEW zurück
}

/*************************************************************
 * @brief   uint8_t stlcmd::get_draw_mode ()
 */
uint8_t stlcmd::get_draw_mode ()
{
    return draw_mode;
}

/***************************************************************
 * @brief   void stlcmd::set_draw_mode (uint8_t mode)
 */
void stlcmd::set_draw_mode (uint8_t mode)
{
    draw_mode = mode;
}

/***********************************************************
 * @brief Get the index off vector <allstl>
 */
size_t stlcmd::grep_index (uint32_t surch_id)
{
    size_t ret = SIZE_MAX;
    size_t i = 0;

    while ((i < allstl.size()) && (ret == SIZE_MAX)) {
        if (allstl[i]->id == surch_id) 
            ret = i;
        else 
            i++;
    }

    return ret;
}

/*******************************************************
 * @brief   bool stlcmd::read_stl (std::string fname)
 */
bool stlcmd::read_stl (std::string fname)
{
    FILE *f;
    uint8_t foo[81];
    std::string str;
    const std::string text_kennung = "solid solidname";
    int load_type = 0;  // 0=unbekannter typ;  1=Text STL;  2=Bin STL

    if ((f = fopen(fname.c_str(), "rb")) != NULL) {
        if (fread(foo, text_kennung.length(), 1, f) == 1) {
            foo[text_kennung.length()] = '\0';
            str.assign((char*)&foo[0], text_kennung.length());
            if (str == text_kennung)
                load_type = 1;
            else    
                load_type = 2;
        }
        // cout << str << "|" << str.length() <<  "| " << load_type << endl;        
        fclose(f);
    }
    
    if (load_type == 1) {
        return read_tex_stl (fname);    // STL Textformat einlesen
    } else if (load_type == 2) 
        return read_bin_stl (fname);    // STL Binärformat einlesen

    return false;
}

/*******************************************************************
 * @brief   bool stlcmd::read_tex_stl (std::string fname)
 * 
 *          STL Textformal structur
 * 
 *          facet normal 0.354367 -0.935106 -8.67098e-08
 *              outer loop
 *                  vertex 26.5353 -16.5146 4.41006
 *                  vertex 25.6167 -16.8627 4.12641
 *                  vertex 27.3123 -16.2202 3.95025
 *              endloop
 *          endfacet
 */
bool stlcmd::read_tex_stl (std::string fname)
{
    std::ifstream is(fname);         // open file
    if (!is.is_open()) {
        std::cout << "File: " << filename << " NICHT geoeffnet\n";
        return false;
    }        

    std::size_t pos;
    std::string buffer;
    uint32_t anz_nor = 0;
    uint32_t anz_tri = 0;
    uint8_t vec_counter = 0;
    float n[3];
    float v[3][3];

    while(getline(is, buffer)) {
        // Example: facet normal 0.000000e+000 -1.000000e+000 0.000000e+000
        if ((pos = buffer.find("facet normal")) != std::string::npos) {     // nornal vector gefunden
            vec_counter = 0;
            anz_nor++;
            std::vector<std::string> tokens;
            for (auto i = std::strtok(&buffer[0], " "); i != NULL; i = std::strtok(NULL, " "))
                tokens.push_back(i);    // token finden !

            if (tokens.size() == 5) {   // normal-Zeile besteht aus 5 token
                for (int k=2; k<(int)tokens.size(); k++)
                    n[k-2] = std::stof(tokens[k]);
            }
        }

        // Example: vertex -1.170000e+004 1.008000e+004 2.750000e+003
        if ((pos = buffer.find("vertex")) != std::string::npos) {           // vertex gefunden
            std::vector<std::string> tokens;
            for (auto i = std::strtok(&buffer[0], " "); i != NULL; i = std::strtok(NULL, " "))
                tokens.push_back(i);    // token finden

            if (tokens.size() == 4) {   // vertex-Zeile besteht aud 4 token
                for (int k=1; k<(int)tokens.size(); k++) 
                    v[vec_counter][k-1] = std::stof(tokens[k]);      // vec3 vector
                
                if (++vec_counter == 3) {   // triangle complet; 3 vertexe gefunden
                    anz_tri++;
                    vec_counter = 0;

                    struct _vertex_ vec;
                    vec3copy (n, vec.n);
                    vec4copy (col, vec.c);
                    vec.attrib = 0x0000;
                    for (int i=0; i<3; i++) {
                        vec3copy (v[i], vec.v);
                        stlvec.push_back ( vec );   // vertex speichern
                    }
                }
            } 
        }
    }
    is.close();
    cout << stlvec.size() << " vertexe " << anz_tri << " triangles  Datei:" << fname << endl;

    return true;
}

/****************************************************************
 * @brief   bool stlcmd::read_bin_stl (std::string fname)
 */
bool stlcmd::read_bin_stl (std::string fname)
{
    bool ret = false;
    FILE *f;
    uint8_t modname[81];                // Modelname
    uint32_t anz_tri = 0;               // Anzahl Triangle
    struct _stl_bin_triangle_ sbt;      // Triangle-Struktur binär STL-File

    stlvec.clear();

    if ((f = fopen(fname.c_str(), "rb")) != NULL) {
        if (fread(modname, 80, 1, f) == 1) {                        // Modelname lesen
            // cout << modname << endl;
            if (fread(&anz_tri, sizeof(uint32_t), 1, f) == 1) {     // Anzahl Dreiecke lesen
                uint32_t ist_tri = 0;
                while (!feof(f)) {
                    if (fread (&sbt, sizeof(struct _stl_bin_triangle_), 1, f) == 1) {   // STL-triangle lesen
                        move_bin_stl_to_stlvec (sbt);
                        ist_tri++;
                    }
                }
                if (ist_tri == anz_tri)
                    ret = true;
            }
        }
        fclose (f);
    }
    cout << stlvec.size() << " vertexe " << anz_tri << " triangles  Datei:" << fname << endl;
    return ret;
}

/***********************************************************************************
 * @brief   void stlcmd::move_bin_stl_to_stlvec (struct _stl_bin_triangle_ stb)
 */
void stlcmd::move_bin_stl_to_stlvec (struct _stl_bin_triangle_ stb)
{
    struct _vertex_ vec;

    vec.attrib = 0x00;
    vec4copy (col, vec.c);      // color use by all vertex
    vec3copy (stb.n, vec.n);    // normale use by all 3 vertex

    vec3copy (stb.p0, vec.v);       
    stlvec.push_back ( vec );   // vertex 0 speichern

    vec3copy (stb.p1, vec.v);
    stlvec.push_back ( vec );   // vertex 1 speichern

    vec3copy (stb.p2, vec.v);
    stlvec.push_back ( vec );   // vertex 2 speichern
}

/***********************************************************************
 * @brief   void stlcmd::get_min_max_center()
 */
void stlcmd::get_min_max_center()
{
    if (stlvec.size()) {
        vec3set (std::numeric_limits<float>::max(), min);
        vec3set (std::numeric_limits<float>::lowest(), max);
        
        for (size_t i=0; i<stlvec.size(); i++) {
            for (int k=0; k<3; k++) {
                if (stlvec[i].v[k] < min[k])
                    min[k] = stlvec[i].v[k];
                if (stlvec[i].v[k] > max[k])
                    max[k] = stlvec[i].v[k];
            }
        }
        
        for (int k=0; k<3; k++) 
            center[k] = (min[k] + max[k]) / 2.0f;
    }

    // vec3print_vec ("min: ", min);
    // vec3print_vec ("max: ", max);
    // vec3print_vec ("center: ", center);
}

/****************************************************************
 * @brief   void stlcmd::calc_max_r()
 */
void stlcmd::calc_max_r()
{
    float foo[3];

    vec3sub (max_ges, center_ges, foo);
    stlcmd::obj_radius = vec3bertag (foo);
}

/*********************************************************
 * @brief   void stlcmd::get_min_max_center_ges()
 */
void stlcmd::get_min_max_center_ges()      // berechnet den Gesamt Schwerpunkt.
{
    vec3set (std::numeric_limits<float>::max(), min_ges);
    vec3set (std::numeric_limits<float>::lowest(), max_ges);

    for (size_t i=0; i<allstl.size(); i++) {
        for (int j=0; j<3; j++) {
            if (allstl[i]->min[j] < min_ges[j])
                min_ges[j] = allstl[i]->min[j];
            if (allstl[i]->max[j] > max_ges[j])
                max_ges[j] = allstl[i]->max[j];
        }
    }

    for (int k=0; k<3; k++) 
        center_ges[k] = (min_ges[k] + max_ges[k]) / 2.0f;

    calc_max_r();
}

/*******************************************************************
 * @brief   static void stlcmd::init_stlcmd()
 */
void stlcmd::init_stlcmd()
{
    vec3set (-1, min_ges);
    vec3set (1, max_ges);
    vec3set (0, center_ges);
}

/******************************************************************
 * @brief   void stlcmd::make_line_vertex()
 */
void stlcmd::make_line_vertex()
{
    stlline.clear();
    for (size_t i=0; i<stlvec.size(); i+=3) {
       stlline.push_back (stlvec[i]);
       stlline.push_back (stlvec[i+1]);

       stlline.push_back (stlvec[i+1]);
       stlline.push_back (stlvec[i+2]);

       stlline.push_back (stlvec[i+2]);
       stlline.push_back (stlvec[i]);
    }
}

/*****************************************************************
 * @brief 
 */
void stlcmd::make_triangle_center()
{
    struct _vertex_only_ sum;

    tri_center.clear();
    for (size_t i=0; i<stlvec.size(); i+=3) {
        vec3set (0.0f, sum.v);
        for (int n=0; n<3; n++) 
            vec3add (sum.v, stlvec[i+n].v, sum.v);

        sum.v[0] /= 3.0f;
        sum.v[1] /= 3.0f;
        sum.v[2] /= 3.0f;
        tri_center.push_back(sum);
    }
    // cout << "tri_center.size()= " << tri_center.size() << endl;
}

#endif