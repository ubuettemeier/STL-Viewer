/**
 * @file stldev.h
 * @author Ulrich Büttemeier
 * @version v0.0.4
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

struct _attribute_ {
    unsigned is_sel : 1;       
    unsigned rest : 15;
};

struct _vertex_ {           // Basistructur für VBO's
    float v[3];
    float n[3];
    float c[4];
    struct _attribute_ attribute;     // default: 0x0000
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

/*****************************************************************
 * @brief   struct _hilfs_container_ wird für 
 *          bool stlcmd::grep_stl_triangle (float *v)
 *          benötigt.
 */
struct _hilfs_container_ {
    long unsigned int i;
    double f0;
};

#pragma pack()

#endif