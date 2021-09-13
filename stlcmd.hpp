/**
 * @file stlcmd.cpp
 * @author Ulrich Buettemeier
 * @version v0.0.2
 * @date 2021-09-12
 */

#ifndef STLCMD_HPP
#define STLCMD_HPP

#define SHOW_CONTROL_TEXT_

#define MEM(x) ((float*) malloc (sizeof(float)*(x)))
#define ANZ_OBJ 2
#define MAX_FLOAT 3.402823466e+30
#define MIN_FLOAT 1.175494351e-30

#include <iostream>
#include <vector>
#include <fstream>
#include <cstring>
#include <cfloat>

#include <GL/glew.h>
#include <GL/glut.h>

#include "utilities.hpp"
#include "mat4.hpp"

using namespace std;

//! ----------------------------------------------------------
//! @brief  Struktur binär STL-File 
//!         uint8_t[80]     - Dateikopf (Header)
//!         uint32_t        - Anzahl Dreiecke
//!
//!         struct _stl_bin_triangle_   - Triangle Struktur
//! ----------------------------------------------------------
#pragma pack(1)

struct _stl_bin_triangle_ {
    float n[3];
    float p0[3];
    float p1[3];
    float p2[3];
    uint16_t attribute;
};

struct _vertex_ {
    float v[3];
    float n[3];
    float c[4];
};

#pragma pack()

class stlcmd {
public:
    stlcmd (string filename);
    ~stlcmd ();
    stlcmd(stlcmd&) = delete;               // Kopierkonstruktor unterbinden
    void operator=(stlcmd&) = delete;       // Zuweisungsoperator verbieten

    void display ();

    static void *operator new (std::size_t size);
    static vector<stlcmd *> allstl;
    static void clear_allstl();

private:
    size_t grep_index (uint32_t surch_id);
    bool read_stl (std::string fname);
    bool read_tex_stl (std::string fname);
    bool read_bin_stl (std::string fname);
    void move_bin_stl_to_stlvec (struct _stl_bin_triangle_ stb);
    void get_min_max_center();
    void get_min_max_center_ges();      // berechnet den Gesamt Schwerpunkt.
    void set_color (float *c);

    uint32_t id;    
    string filename;
    bool init_by_new;

    vector <struct _vertex_> stlvec;
    float *col = MEM(4);
    float *center = MEM(3);
    float *min = MEM(3);
    float *max = MEM(3);
    float *stl_m = MEM(16);

    GLuint vaoID[ANZ_OBJ] = {0, 0};    // VAO einrichten
    GLuint vboID[ANZ_OBJ] = {0, 0};    // VBO einrichten

    // ------------ static's ------------------------
    static uint32_t id_counter;         // used for <id>
    static bool use_new;                // true: new stlcmd("xxx"); false: stlcmd a("xxx");
    static float *center_ges;
    static float *min_ges, *max_ges;
};

// ---------- init static's ----------------
float *stlcmd::center_ges = MEM(3);
float *stlcmd::min_ges = MEM(3);
float *stlcmd::max_ges = MEM(3);

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
        get_min_max_center();
        set_color ( col );

        glGenVertexArrays(ANZ_OBJ, vaoID);  // create the Vertex Array Objects
        glGenBuffers(ANZ_OBJ, vboID);       // generating Vertex Buffer Objects (VBO)

        glBindVertexArray(vaoID[0]);                    // VAO 0
        glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);        // VBO 0
        glBufferData(GL_ARRAY_BUFFER, stlvec.size() *sizeof(struct _vertex_), stlvec.data(), GL_STATIC_DRAW);

        int stride = sizeof(struct _vertex_);           // int stride = sizeof(Vertex);
        char *offset = (char*)NULL;

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
    // get_min_max_center_ges();
    use_new = false;
}

/********************************************************************
 * @brief   void stlcmd::set_color (float *c) 
 */
void stlcmd::set_color (float *c) 
{
    vec4copy (c, col);

    if (vboID[0] != 0) {
    for (size_t i=0; i<stlvec.size(); i++) 
        vec4copy (c, stlvec[i].c);

    glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);       // VBO
    glBufferData(GL_ARRAY_BUFFER, stlvec.size() *sizeof(struct _vertex_), stlvec.data(), GL_STATIC_DRAW);
    }

    /*
    if (vboID[1] != 0) {
        for (size_t i=0; i<stlline.size(); i++)
            vec4copy (c, stlline[i].c);
        glBindBuffer(GL_ARRAY_BUFFER, vboID[1]);       // VBO
        glBufferData(GL_ARRAY_BUFFER, stlline.size() *sizeof(struct _vertex_), stlline.data(), GL_STATIC_DRAW);        
    }
    */
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

/*********************************************************
 * @brief 
 */
void *stlcmd::operator new (std::size_t size)
{
    use_new = true;
    return ::operator new(size);
}

/********************************************************
 * @brief 
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

/**********************************************************
 * @brief   draw OpenGL VBA's
 */
void stlcmd::display ()
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();                         // MODELVIEW sichern

    glMultMatrixf ( stl_m );                // MODELVIEW mit Bauteil-Matrix (stl_m)  multiplizieren 
    
    glBindVertexArray(vaoID[0]);             // bind pyramid VAO
    glDrawArrays(GL_TRIANGLES, 0, stlvec.size());   // render data

    glPopMatrix();                          // MODELVIEW zurück
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
    int load_type = 0;  // 0=unbekannter typ;  1=Text STL;  2=Bin STL

    if ((f = fopen(fname.c_str(), "rb")) != NULL) {
        if (fread(foo, 80, 1, f) == 1) {
            foo[80] = '\0';
            str.assign((char*)&foo[0], 80);
            str = util::rtrim (str);

            if (str.length() >= 80)
                load_type = 1;
            else if ((str.length() > 0) && (str.length() < 80))
                load_type = 2;
        }
        fclose(f);
    }
    
    if (load_type == 1) {
        return read_tex_stl (fname);
    } else if (load_type == 2) 
        return read_bin_stl (fname);

    return false;
}

/*******************************************************************
 * @brief   bool stlcmd::read_tex_stl (std::string fname)
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
                    for (int i=0; i<3; i++) {
                        vec3copy (v[i], vec.v);
                        stlvec.push_back ( vec );   // vertex speichern
                    }
                }
            } 
        }
    }
    is.close();
    cout << fname << ": " << stlvec.size() << " Dreiecke " << anz_tri << " triangles\n";

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
    uint32_t anz_tri;                   // Anzahl Triangle
    struct _stl_bin_triangle_ sbt;      // Triangle-Struktur binär STL-File

    stlvec.clear();

    if ((f = fopen(fname.c_str(), "rb")) != NULL) {
        if (fread(modname, 80, 1, f) == 1) {                        // Modelname lesen
            if (fread(&anz_tri, sizeof(uint32_t), 1, f) == 1) {     // Anzahl Dreiecke lesen
                uint32_t ist_tri = 0;
                while (!feof(f)) {
                    if (fread (&sbt, sizeof(struct _stl_bin_triangle_), 1, f) == 1) {   // STL-triangle lesen
                        move_bin_stl_to_stlvec (sbt);
                        ist_tri++;
                    }
                }
                cout << fname << ": " << stlvec.size() << " vertexe " << anz_tri << " triangles\n";
                if (ist_tri == anz_tri)
                    ret = true;
            }
        }
        fclose (f);
    }

    return ret;
}

/***********************************************************************************
 * @brief   void stlcmd::move_bin_stl_to_stlvec (struct _stl_bin_triangle_ stb)
 */
void stlcmd::move_bin_stl_to_stlvec (struct _stl_bin_triangle_ stb)
{
    struct _vertex_ vec;

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
        vec3set (FLT_MAX, FLT_MAX, FLT_MAX, min);
        vec3set (FLT_MIN, FLT_MIN, FLT_MIN, max);
        
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

    vec3print_vec ("min: ", min);
    vec3print_vec ("max: ", max);
    vec3print_vec ("center: ", center);
}

/*********************************************************
 * @brief   void stlcmd::get_min_max_center_ges()
 */
void stlcmd::get_min_max_center_ges()      // berechnet den Gesamt Schwerpunkt.
{
    vec3set (FLT_MAX, FLT_MAX, FLT_MAX, min_ges);
    vec3set (FLT_MIN, FLT_MIN, FLT_MIN, max_ges);

    for (size_t i=0; i<allstl.size(); i++) {
        for (int j=0; j<3; j++) {
            if (allstl[i]->min[j] < min_ges[j])
                min_ges[j] = allstl[i]->min[j];
            if (allstl[i]->max[j] < max_ges[j])
                max_ges[j] = allstl[i]->max[j];
        }
    }

    for (int k=0; k<3; k++) 
        center_ges[k] = (min_ges[k] + max_ges[k]) / 2.0f;
}
#endif