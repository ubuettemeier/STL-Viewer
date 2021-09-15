/**
 * @file basicelement.hpp
 * @author Ulrich Büttemeier
 * @brief basics zeigt Koordiatenkreuz, Max-Quader, ....
 * @version v0.0.2
 * @date 2021-09-15
 */

#ifndef BASICELEMENT_HPP
#define BASICELEMENT_HPP

#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GL/glut.h>

#include "stldev.h"
#include "mat4.hpp"

using namespace std;

class basics {
public:
    basics(float axeslength = 1.0f);
    ~basics();
    void set_max_quader (float *min, float *max);
    void display();
private:
    vector <struct _vertex_small_> ursprung;
    vector <struct _vertex_small_> max_quader;

    GLuint koor_vaoID[1] = {0};    // VAO einrichten; triangle, line, ???
    GLuint koor_vboID[1] = {0};    // VBO einrichten; triangle, line, ???

    GLuint quad_vaoID[1] = {0};    // VAO einrichten; triangle, line, ???
    GLuint quad_vboID[1] = {0};    // VBO einrichten; triangle, line, ???
};

basics::basics(float axeslength)
{
    struct _vertex_small_ foo;

    // X-Achse
    vec4set (1, 0, 0, 1, foo.c);    
    vec3set (0, 0, 0, foo.v);
    ursprung.push_back(foo);
    vec3set (axeslength, 0, 0, foo.v);
    ursprung.push_back(foo);

    // Y-Achse
    vec4set (0, 1, 0, 1, foo.c);
    vec3set (0, 0, 0, foo.v);
    ursprung.push_back(foo);
    vec3set (0, axeslength, 0, foo.v);
    ursprung.push_back(foo);

    // Z-Achse
    vec4set (0.2, 0.2, 0.9, 1, foo.c);
    vec3set (0, 0, 0, foo.v);
    ursprung.push_back(foo);
    vec3set (0, 0, axeslength, foo.v);
    ursprung.push_back(foo);
    
    // ------------------------------------------------------------------------
    glGenVertexArrays(1, koor_vaoID);  // create the Vertex Array Objects
    glGenBuffers(1, koor_vboID);       // generating Vertex Buffer Objects (VBO)

    glBindVertexArray(koor_vaoID[0]);                    // VAO 0
    glBindBuffer(GL_ARRAY_BUFFER, koor_vboID[0]);        // VBO 0
    glBufferData(GL_ARRAY_BUFFER, ursprung.size() *sizeof(struct _vertex_small_), ursprung.data(), GL_STATIC_DRAW);

    int stride = sizeof(struct _vertex_small_);           // int stride = sizeof(Vertex);
    char *offset = (char*)NULL;

    glVertexPointer(3, GL_FLOAT, stride, offset);   // position  3*float
    glEnableClientState(GL_VERTEX_ARRAY);

    offset = (char*)NULL + 3*sizeof(float);         // color
    glColorPointer(4, GL_FLOAT, stride, offset);    // 4*float
    glEnableClientState(GL_COLOR_ARRAY);
}

basics::~basics()
{

}

/**************************************************************************
 * @brief   void basics::set_max_quader (float *min, float *max)
 */
void basics::set_max_quader (float *min, float *max)
{
    if (quad_vaoID[0] == 0) {
        struct _vertex_small_ foo;

        vec4set (1, 1, 1, 1, foo.c);    // Farbe: weiss
        vec3copy (min, foo.v);
        max_quader.push_back (foo);
        vec3copy (max, foo.v);
        max_quader.push_back (foo);

        // ------------------------------------------------------------------------
        glGenVertexArrays(1, quad_vaoID);  // create the Vertex Array Objects
        glGenBuffers(1, quad_vboID);       // generating Vertex Buffer Objects (VBO)

        glBindVertexArray(quad_vaoID[0]);                    // VAO 0
        glBindBuffer(GL_ARRAY_BUFFER, quad_vboID[0]);        // VBO 0
        glBufferData(GL_ARRAY_BUFFER, max_quader.size() *sizeof(struct _vertex_small_), max_quader.data(), GL_STATIC_DRAW);

        int stride = sizeof(struct _vertex_small_);           // int stride = sizeof(Vertex);
        char *offset = (char*)NULL;

        glVertexPointer(3, GL_FLOAT, stride, offset);   // position  3*float
        glEnableClientState(GL_VERTEX_ARRAY);

        offset = (char*)NULL + 3*sizeof(float);         // color
        glColorPointer(4, GL_FLOAT, stride, offset);    // 4*float
        glEnableClientState(GL_COLOR_ARRAY);
    }
}

void basics::display()
{
    glMatrixMode(GL_MODELVIEW);
    glDisable (GL_LIGHTING);

    glBindVertexArray(koor_vaoID[0]);             // bind pyramid VAO
    glLineWidth (3);
    glDrawArrays(GL_LINES, 0, ursprung.size());   // render data

    if (quad_vaoID[0] != 0) {
        glBindVertexArray(quad_vaoID[0]);             // bind pyramid VAO
        glLineWidth (2);
        glDrawArrays(GL_LINES, 0, max_quader.size());   // render data
    }
    glEnable (GL_LIGHTING);
}

#endif
