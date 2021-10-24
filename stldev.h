/**
 * @file stldev.h
 * @author Ulrich Büttemeier
 * @version v0.0.3
 * @date 2021-09-15
 */

#ifndef STLDEV_H
#define STLDEV_H

#include <stdint.h>

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

struct _vertex_small_ {
    float v[3];
    float c[4];
};

struct _vertex_only_ {
    float v[3];
};

struct _pick_buf_ {     // pv[] wird mit <get_3D_from_view()> ermittelt. S.auch <mouse_func()>
    bool ist_aktiv;     // false: mouse-pick hat keine Geometrie getroffen. true: mouse-pick hat Geometrie getroffen
    float pv[3];        // Enthält die 3D-Koordinaten vom Pick-Punkt !!!
};

struct _select_buf_ {   // Liste der gepickten Flächen ! S.auch: std::vector<struct _select_buf_> sel_buf
    void *p_to_stlcmd;
    long unsigned int index;
};

#pragma pack()

#endif