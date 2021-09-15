/**
 * @file basicelement.hpp
 * @author Ulrich Büttemeier
 * @version v0.0.1
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
    void display();
private:
    vector <struct _vertex_small_> ursprung;

    GLuint vaoID[1] = {0};    // VAO einrichten; triangle, line, ???
    GLuint vboID[1] = {0};    // VBO einrichten; triangle, line, ???
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
    glGenVertexArrays(1, vaoID);  // create the Vertex Array Objects
    glGenBuffers(1, vboID);       // generating Vertex Buffer Objects (VBO)

    glBindVertexArray(vaoID[0]);                    // VAO 0
    glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);        // VBO 0
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

void basics::display()
{
    glMatrixMode(GL_MODELVIEW);
    glBindVertexArray(vaoID[0]);             // bind pyramid VAO
    glDisable (GL_LIGHTING);
    glLineWidth (3);
    glDrawArrays(GL_LINES, 0, ursprung.size());   // render data
    glEnable (GL_LIGHTING);

}

#endif
