/**
 * @file stldev.h
 * @author Ulrich Büttemeier
 * @version v0.0.1
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

#pragma pack()

#endif