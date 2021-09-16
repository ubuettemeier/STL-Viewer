/**
 * @file mat4.hpp
 * @author Ulrich Buettemeier
 * @version v0.0.5
 * @date 2021-08-26
 */

#ifndef MAT4HPP
#define MAT4HPP

#include <iostream>
#include <math.h>
#include <cstring>

using namespace std;

void vec3print_vec (float *p, char endline = '\n');
void vec3print_vec (const char *s, float *p);
void vec4print_vec (float *p);
void vec3set (float x, float y, float z, float *res);
void vec4set (float x, float y, float z, float u, float *res);
void vec3copy (float *a, float *b);                     // a => b
void vec4copy (float *a, float *b);
void vec3change (float *a, float *b);
void vec3rot_vec (float *rsp, float *rep,
                    float *ve,
                    float a,          // a = Rotationswinkel [rad]
                    float *res);
void vec3rot_point_um_achse (float sx, float sy, float sz,
                               float ex, float ey, float ez,
                               float alpha,         // alpha in rad
                               float &px, float &py, float &pz);
void vec3rot_point_um_achse_II (float *s, float *e,
                                float alpha,        // alpha in rad
                                float *p);
void vec3richtungs_cos (float *a, float *res);
float vec3bertag ( float *a );                      // Vektor Betrag
float vec3dist (float *a, float *b);
float vec3Dot( float *a, float *b);
void vec3add ( float *a, float *b, float *res);      // Addition
void vec3sub( float *a, float *b, float *res );      // subtraktion
void vec3mul_faktor (float *a, float f, float *res); // res = a * f
void vec3add_vec_mul_fakt ( float *a, float *b, float f, float *res );      // res = a + b*f
void vec3Cross( float *a, float *b, float *res);   // Kreuzprodukt
float vec3scalar( float *a, float *b);             // Scalarprodukt
float vec3alpha (float *a, float *b);              // Winkel zwische 2 Vectoren
void vec3Normalize(float *a);                      // Einsvektor

void mat4print (float *m);
void mat4Identity( float *a);                      // Einheitsmatrix
void mat4Multiply(float *a, float *b, float *res);   // Matrix multiplikation
void mat4Perspective(float *a, float fov, float aspect, float zNear, float zFar);
void mat4LookAt(float *viewMatrix,
                  float eyeX, float eyeY, float eyeZ,
                  float centerX, float centerY, float centerZ,
                  float upX, float upY, float upZ);
void mat4Transpose(float* a, float *transposed);
bool mat4Invert(float* m, float *inverse);
void mat4mulvec3 (float *m, float *v, float *res);    // m[16] * v[3] = res[3]

void mat4translate(float *m, float *tv);              // m[16], tv[3]; Funktion erstellt eine Translations-Matrix
void mat4scale(float *m, float *sf);                  // m[16]; sf[3]; Funktion erstellt die Skalierungs-Matrix
void mat4Roate(float *m, float angle, float *r);      // m[16], angle in rad, r[3]; Rotationsachse geht durch 0 und r[3].
void mat4Rot_um_Achse (float *m, float *sp, float *ep, float alpha);  // Erstellt Rotationsmatrix um eine belibiege Achse. alpha in [rad]

void get_max_min (float *dat, uint32_t anz_ele, uint32_t offset, float *res);
void move_vec3 (float *dat, uint32_t anz_ele, uint16_t offset, float *v);

bool vec3_equal (float *a, float *b);   // wie sicher ist die Funktion ??? Delta < 0.00001 dann Vektoren gleich !!!
float grad_to_rad (float grad);         // return rad 
float rad_to_grad (float rad);          // return grad

float vec3dist_point_vec (float *p, float *a, float *r);  // Normalabstand von p^ bis zur Gerade a^+x*r^

/// ---------------------------------------------------------------------------------------------
void get_max_min (float *dat, uint32_t anz_ele, uint32_t offset, float *res)
{
    res[0] = res[2] = res[4] = 1000000.0;    // min
    res[1] = res[3] = res[5] = -1000000.0;   // max

    for (uint32_t n=0; n<anz_ele; n++) {
        uint32_t basis = n*offset;
        for (int j=0; j<3; j++) {
            if (dat[basis+j] < res[j*2])
                res[j*2] = dat[basis+j];

            if (dat[basis+j] > res[j*2+1])
                res[j*2+1] = dat[basis+j];
        }
    }
}
/// ---------------------------------------------------------------------------------------------
void move_vec3 (float *dat, uint32_t anz_ele, uint16_t offset, float *v)
{
    for (uint32_t n=0; n<anz_ele; n++) {
        vec3add ( &dat[n*offset], v, &dat[n*offset] );
    }
}
/// ---------------------------------------------------------------------------------------------
float vec3dist (float *a, float *b)
{
    float res[3];
    vec3sub (a, b, res);
    return ( vec3bertag(res) );
}
/// ---------------------------------------------------------------------------------------------
void vec3print_vec (float *p, char endline)
{
    std::cout << p[0] << ", " << p[1] << ", " << p[2] << endline;
}
/// ---------------------------------------------------------------------------------------------
void vec3print_vec (const char *s, float *p)
{
    std::cout << s << p[0] << ", " << p[1] << ", " << p[2] << std::endl;
    // std::cout << "vec3print_vec() funktioniert nicht\n";
}
/// ---------------------------------------------------------------------------------------------
void vec4print_vec (float *p)
{
    std::cout << p[0] << ", " << p[1] << ", " << p[2] << ", " << p[3] << std::endl;
}
/// ---------------------------------------------------------------------------------------------
void vec3set (float x, float y, float z, float *res)
{
  res[0] = x;
  res[1] = y;
  res[2] = z;
}
/// ---------------------------------------------------------------------------------------------
void vec4set (float x, float y, float z, float u, float *res)
{
  res[0] = x;
  res[1] = y;
  res[2] = z;
  res[3] = u;
}
/// ---------------------------------------------------------------------------------------------
void vec3change (float *a, float *b)
{
  float foo[3];
  vec3copy (a, foo);
  vec3copy (b, a);
  vec3copy (foo, b);
}
/// ---------------------------------------------------------------------------------------------
void vec3copy (float *a, float *b)    // a => b
{
    for (int i=0; i<3; i++)
      b[i] = a[i];
}
/// ---------------------------------------------------------------------------------------------
void vec4copy (float *a, float *b)
{
  for (int i=0; i<4; i++)
      b[i] = a[i];
}
/// ---------------------------------------------------------------------------------------------
void vec3rot_vec (float *rsp, float *rep,
                  float *ve,
                  float a,          // a = Rotationswinkel [rad]
                  float *res)
{
  // float delta = 0.01;        // wird zur Zeit nicht benötigt.

  float xx[3], yy[3], zz[3];
  float w1[3], w2[3], w3[3];
  float p01[3], p02[3];

  vec3sub ( rep, rsp, xx );
  vec3Normalize ( xx );
  vec3Cross ( xx, ve, zz );
  
  if (vec3bertag(zz) == 0.0f) {
    // cout << "betrag = 0\n";
  }
  vec3Normalize ( zz );
  vec3Cross (zz, xx, yy);
  vec3Normalize ( yy );

  vec3richtungs_cos (xx, w1 );
  vec3richtungs_cos (yy, w2 );
  vec3richtungs_cos (zz, w3 );
  // Den zu drehenden Vector in Ursprung drehen -> Der Vectoren v wird in Z gedreht.
  p01[0] = ve[0] * w1[0] + ve[1] * w1[1] + ve[2] * w1[2];
  p01[1] = ve[0] * w2[0] + ve[1] * w2[1] + ve[2] * w2[2];
  p01[2] = ve[0] * w3[0] + ve[1] * w3[1] + ve[2] * w3[2];

  // Rotation um X
  p02[0] = p01[0];
  // p02.a[1] = p01.a[1]*_fm_cosl(a) - p01.a[2]*_fm_sinl(a);
  p02[1] = p01[1]*cosl(a) - p01[2]*sinl(a);
  // p02.a[2] = p01.a[1]*_fm_sinl(a) + p01.a[2]*_fm_cosl(a);
  p02[2] = p01[1]*sinl(a) + p01[2]*cosl(a);

  res[0] = p02[0] * w1[0] + p02[1] * w2[0] + p02[2] * w3[0];
  res[1] = p02[0] * w1[1] + p02[1] * w2[1] + p02[2] * w3[1];
  res[2] = p02[0] * w1[2] + p02[1] * w2[2] + p02[2] * w3[2];
}
/// ---------------------------------------------------------------------------------------------
void vec3rot_point_um_achse (float sx, float sy, float sz,
                              float ex, float ey, float ez,
                              float alpha,                       // alpha in rad
                              float &px, float &py, float &pz)
{
  float sp[3] = {sx, sy, sz};
  float ep[3] = {ex, ey, ez};
  float p[3] = {px, py, pz};
  
  vec3rot_point_um_achse_II (sp, ep, alpha, p);

  px = p[0];
  py = p[1];
  pz = p[2];
}
/// ---------------------------------------------------------------------------------------------
void vec3rot_point_um_achse_II (float *s, float *e,   // s=Rotationsachse Startpkt. e=Endpkt.
                                float alpha,          // alpha in [rad]
                                float *p)             // der zu drehende Punkt
{
  float m[16];
  mat4Rot_um_Achse (m, s, e, alpha);
  mat4mulvec3 (m, p, p);    // center rotieren
}                                
/// ---------------------------------------------------------------------------------------------
void vec3richtungs_cos (float *a, float *res) 
{
  float bet = vec3bertag( a );
  for (int i=0; i<3; i++) 
    res[i] = a[i] / bet;
}

/// ---------------------------------------------------------------------------------------------
float vec3bertag ( float *a ) {                     // Vektor Betrag
  return sqrt( a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
}

/// ---------------------------------------------------------------------------------------------
float vec3Dot( float *a, float *b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];         // skalares Produkt.
}

/// ---------------------------------------------------------------------------------------------
void vec3add ( float *a, float *b, float *res)      // Addition
{
  for (int i=0; i<3; i++)
    res[i] = a[i] + b[i];
}

/// ---------------------------------------------------------------------------------------------
void vec3sub( float *a, float *b, float *res )      // subtraktion
{
  for (int i=0; i<3; i++) {
    res[i] = a[i] - b[i];
  }
}

/// ---------------------------------------------------------------------------------------------
void vec3mul_faktor (float *a, float f, float *res)
{
    for (int i=0; i<3; i++)
      res[i] = a[i] * f;
}
/// ---------------------------------------------------------------------------------------------
void vec3add_vec_mul_fakt ( float *a, float *b, float f, float *res )      // res = a + b*f
{
  for (int i=0; i<3; i++)
    res[i] = a[i] + b[i]*f;
}

/// ---------------------------------------------------------------------------------------------
void vec3Cross( float *a, float *b, float *res) {   // Kreuzprodukt
  res[0] = a[1] * b[2]  -  b[1] * a[2];
  res[1] = a[2] * b[0]  -  b[2] * a[0];
  res[2] = a[0] * b[1]  -  b[0] * a[1];
}
/// ---------------------------------------------------------------------------------------------
float vec3scalar( float *a, float *b) {   // Scalarprodukt
  return (a[0]*b[0] + a[1]*b[1] + a[2]*b[2]);
}  
/// ---------------------------------------------------------------------------------------------
float vec3alpha (float *a, float *b) {           // Winkel zwische 2 Vectoren
  float nenner = vec3bertag(a) * vec3bertag(b);
  float zaehler = vec3scalar (a, b);

  if (zaehler == 0.0f)
    return (M_PI/2.0f);
  
  float ret = 0.0f;
  if (nenner == 0.0f)
    std::cout << "ERROR vec3alpha\n";

  ret = acos(zaehler / nenner);

  return ret;
}
/// ---------------------------------------------------------------------------------------------
void vec3Normalize(float *a) {                      // Einsvektor
  float mag = vec3bertag( a );
  a[0] /= mag; a[1] /= mag; a[2] /= mag;
}

/// ---------------------------------------------------------------------------------------------
void mat4print (float *m)
{
  for (int x=0; x<4; x++) {
    for (int y=0; y<4; y++) 
      cout << m[y*4+x] << " ";
    cout << endl;
  }
}

/// ---------------------------------------------------------------------------------------------
void mat4Identity( float *m) {                      // Einheitsmatrix    
  m[0] = m[5] = m[10] = m[15] = 1.0f;
  m[1] = m[2] = m[3] = m[4] = m[6] = m[7] = m[8] = m[9] = m[11] = m[12] = m[13] = m[14] = 0.0f;
}

/// ---------------------------------------------------------------------------------------------
void mat4Multiply(float *a, float *b, float *res)   // Matrix multiplikation <float[4] * float[4]>
{
  float result[16];

  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      result[j*4 + i] = 0.0f;
      for (int k = 0; k < 4; ++k) {
        result[j*4 + i] += a[k*4 + i] * b[j*4 + k];
      }
    }
  }

  memcpy (res, result, sizeof(float)*16);
}

/// -------------------------------------------------------------------------------------------------------
void mat4Perspective(float *a, float fov, float aspect, float zNear, float zFar) {
  float f = 1.0f / float(tan (fov/2.0f * (M_PI / 180.0f)));
  mat4Identity(a);
  a[0] = f / aspect;
  a[1 * 4 + 1] = f;
  a[2 * 4 + 2] = (zFar + zNear)  / (zNear - zFar);
  a[3 * 4 + 2] = (2.0f * zFar * zNear) / (zNear - zFar);
  a[2 * 4 + 3] = -1.0f;
  a[3 * 4 + 3] = 0.0f;
}
/// -------------------------------------------------------------------------------------------------------
void mat4LookAt(float *viewMatrix,
                float eyeX, float eyeY, float eyeZ,
                float centerX, float centerY, float centerZ,
                float upX, float upY, float upZ) {

  float right[3];
  float up[3] = { upX, upY, upZ };
  float eye[3] = { eyeX, eyeY, eyeZ };
  float center[3] = { centerX, centerY, centerZ };
  float dir[3];

  vec3sub ( center, eye, dir );   // dir = center - eye

  vec3Normalize(dir);       // Richtung einsvector
  vec3Cross(dir,up,right);  // right = dir X up
  vec3Normalize(right);     // right einsvector
  vec3Cross(right,dir,up);
  vec3Normalize(up);
  // first row
  viewMatrix[0]  = right[0];
  viewMatrix[4]  = right[1];
  viewMatrix[8]  = right[2];
  viewMatrix[12] = -vec3Dot(right, eye);
  // second row
  viewMatrix[1]  = up[0];
  viewMatrix[5]  = up[1];
  viewMatrix[9]  = up[2];
  viewMatrix[13] = -vec3Dot(up, eye);
  // third row
  viewMatrix[2]  = -dir[0];
  viewMatrix[6]  = -dir[1];
  viewMatrix[10] = -dir[2];
  viewMatrix[14] =  vec3Dot(dir, eye);
  // forth row
  viewMatrix[3]  = 0.0f;
  viewMatrix[7]  = 0.0f;
  viewMatrix[11] = 0.0f;
  viewMatrix[15] = 1.0f;
}
/// ------------------------------------------------------------------------------------
void mat4Print(float* a) {
  // opengl uses column major order
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      cout << a[j * 4 + i] << " ";
    }
    cout << endl;
  }
}
/// -------------------------------------------------------------------------------------------------------
void mat4Transpose(float* a, float *transposed)
{
    int t = 0;
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        transposed[t++] = a[j * 4 + i];
      }
    }
}
/// -------------------------------------------------------------------------------------------------------
void mat4mulvec3 (float *m, float *v, float *res)   // m[16] * v[3] = res[3]
{
  float foo[3];
  
  foo[0] = v[0]*m[0] + v[1]*m[4] + v[2]*m[8] + 1.0f*m[12];
  foo[1] = v[0]*m[1] + v[1]*m[5] + v[2]*m[9] + 1.0f*m[13];  
  foo[2] = v[0]*m[2] + v[1]*m[6] + v[2]*m[10] + 1.0f*m[14];

  memcpy (res, foo, sizeof(float)*3);
}
/// -------------------------------------------------------------------------------------------------------
bool mat4Invert(float* m, float *inverse)
{
    float inv[16];
    inv[0] = m[5]*m[10]*m[15]-m[5]*m[11]*m[14]-m[9]*m[6]*m[15]+
             m[9]*m[7]*m[14]+m[13]*m[6]*m[11]-m[13]*m[7]*m[10];
    inv[4] = -m[4]*m[10]*m[15]+m[4]*m[11]*m[14]+m[8]*m[6]*m[15]-
             m[8]*m[7]*m[14]-m[12]*m[6]*m[11]+m[12]*m[7]*m[10];
    inv[8] = m[4]*m[9]*m[15]-m[4]*m[11]*m[13]-m[8]*m[5]*m[15]+
             m[8]*m[7]*m[13]+m[12]*m[5]*m[11]-m[12]*m[7]*m[9];
    inv[12]= -m[4]*m[9]*m[14]+m[4]*m[10]*m[13]+m[8]*m[5]*m[14]-
             m[8]*m[6]*m[13]-m[12]*m[5]*m[10]+m[12]*m[6]*m[9];
    inv[1] = -m[1]*m[10]*m[15]+m[1]*m[11]*m[14]+m[9]*m[2]*m[15]-
             m[9]*m[3]*m[14]-m[13]*m[2]*m[11]+m[13]*m[3]*m[10];
    inv[5] = m[0]*m[10]*m[15]-m[0]*m[11]*m[14]-m[8]*m[2]*m[15]+
             m[8]*m[3]*m[14]+m[12]*m[2]*m[11]-m[12]*m[3]*m[10];
    inv[9] = -m[0]*m[9]*m[15]+m[0]*m[11]*m[13]+m[8]*m[1]*m[15]-
             m[8]*m[3]*m[13]-m[12]*m[1]*m[11]+m[12]*m[3]*m[9];
    inv[13]= m[0]*m[9]*m[14]-m[0]*m[10]*m[13]-m[8]*m[1]*m[14]+
             m[8]*m[2]*m[13]+m[12]*m[1]*m[10]-m[12]*m[2]*m[9];
    inv[2] = m[1]*m[6]*m[15]-m[1]*m[7]*m[14]-m[5]*m[2]*m[15]+
             m[5]*m[3]*m[14]+m[13]*m[2]*m[7]-m[13]*m[3]*m[6];
    inv[6] = -m[0]*m[6]*m[15]+m[0]*m[7]*m[14]+m[4]*m[2]*m[15]-
             m[4]*m[3]*m[14]-m[12]*m[2]*m[7]+m[12]*m[3]*m[6];
    inv[10]= m[0]*m[5]*m[15]-m[0]*m[7]*m[13]-m[4]*m[1]*m[15]+
             m[4]*m[3]*m[13]+m[12]*m[1]*m[7]-m[12]*m[3]*m[5];
    inv[14]= -m[0]*m[5]*m[14]+m[0]*m[6]*m[13]+m[4]*m[1]*m[14]-
             m[4]*m[2]*m[13]-m[12]*m[1]*m[6]+m[12]*m[2]*m[5];
    inv[3] = -m[1]*m[6]*m[11]+m[1]*m[7]*m[10]+m[5]*m[2]*m[11]-
             m[5]*m[3]*m[10]-m[9]*m[2]*m[7]+m[9]*m[3]*m[6];
    inv[7] = m[0]*m[6]*m[11]-m[0]*m[7]*m[10]-m[4]*m[2]*m[11]+
             m[4]*m[3]*m[10]+m[8]*m[2]*m[7]-m[8]*m[3]*m[6];
    inv[11]= -m[0]*m[5]*m[11]+m[0]*m[7]*m[9]+m[4]*m[1]*m[11]-
             m[4]*m[3]*m[9]-m[8]*m[1]*m[7]+m[8]*m[3]*m[5];
    inv[15]= m[0]*m[5]*m[10]-m[0]*m[6]*m[9]-m[4]*m[1]*m[10]+
             m[4]*m[2]*m[9]+m[8]*m[1]*m[6]-m[8]*m[2]*m[5];

    float det = m[0]*inv[0]+m[1]*inv[4]+m[2]*inv[8]+m[3]*inv[12];
    if (det == 0) return false;
    det = 1.0f / det;
    for (int i = 0; i < 16; i++) inverse[i] = inv[i] * det;
    return true;
}
/// -------------------------------------------------------------------------------------------------------
bool vec3_equal (float *a, float *b)
{
  bool ret = false;

  if (vec3dist(a, b) < 0.00001) ret = true; 

  return ret;
}
/// -------------------------------------------------------------------------------------------------------
float grad_to_rad (float grad)
{
  return (grad * M_PI / 180.0f);
}
/// -------------------------------------------------------------------------------------------------------
float rad_to_grad (float rad)           // return grad
{
  return rad / M_PI * 180.0f;
}

/** -----------------------------------------------------------------------------
 * @brief Abstand Punkt / Gerade
 *        g: a^ + x*r^ 
 *        https://de.serlo.org/mathe/2137/abstand-eines-punktes-zu-einer-geraden-berechnen-analytische-geometrie
 * @param p: Punkt
 * @param a: Startpunkt Gerade
 * @param r: Richtungsvector der Geraden
 * 
 * @return  Normalabstand von p^ bis zur Gerade a^+x*r^
 */
float vec3dist_point_vec (float *p, float *a, float *r)  // g: a^ + x*r^
{
  float ret = 0.0;
  float foo[3], kp[3];

  vec3sub (p, a, foo);
  vec3Cross (foo, r, kp);
  float w = vec3bertag(r);

  if (w != 0.0)
    ret = vec3bertag(kp) / w;

  return ret;   // Normalabstand
}

/***************************************************************************
 * @brief     Funktion erstellt eine Translations Matrix
 * 
 * @param m   float m[16]
 * @param tv  float tv[3]; Translate-Vector
 */
void mat4translate(float *m, float *tv)   // 
{
  m[0]=1.0f; m[4]=0.0f; m[8]=0.0f;  m[12]=tv[0];
  m[1]=0.0f; m[5]=1.0f; m[9]=0.0f;  m[13]=tv[1];
  m[2]=0.0f; m[6]=0.0f; m[10]=1.0f; m[14]=tv[2];
  m[3]=0.0f; m[7]=0.0f; m[11]=0.0f; m[15]=1.0f;
}

/***************************************************************************
 * @brief     Funktion erstellt die Skalierungs-Matrix
 * 
 * @param m   float m[16]
 * @param sf  float sv[3]; Skaltierungsfaktoren in x, y, z
 */
void mat4scale(float *m, float *sf)   // Funktion erstellt die Skalierungs-Matrix
{
  m[0]=sf[0]; m[4]=0.0f;  m[8]=0.0f;   m[12]=0.0f;
  m[1]=0.0f;  m[5]=sf[1]; m[9]=0.0f;   m[13]=0.0f;
  m[2]=0.0f;  m[6]=0.0f;  m[10]=sf[2]; m[14]=0.0f;
  m[3]=0.0f;  m[7]=0.0f;  m[11]=0.0f;  m[15]=1.0f;
}

/****************************************************************************************
 * @brief         Funktion erstellt die Rotations-Matrix 
 * 
 * @param m       matrix[16]
 * @param angle   in rad
 * @param r       rotations-Achse; geht durch den 0-Punkt
 */
void mat4Roate(float *m, float angle, float *r)
{
  float c = cosf (angle);
  float s = sinf (angle);

  m[0] = r[0]*r[0]*(1-c)+c;      m[4] = r[0]*r[1]*(1-c)-r[2]*s;   m[8] = r[0]*r[2]*(1-c)+r[1]*s;    m[12] = 0.0f;
  m[1] = r[0]*r[1]*(1-c)+r[2]*s; m[5] = r[1]*r[1]*(1-c)+c;        m[9] = r[1]*r[2]*(1-c)-r[0]*s;    m[13] = 0.0f;
  m[2] = r[0]*r[2]*(1-c)-r[1]*s; m[6] = r[1]*r[2]*(1-c)+r[0]*s;   m[10] = r[2]*r[2]*(1-c)+c;        m[14] = 0.0f;
  m[3] = 0.0f;                   m[7] = 0.0f;                     m[11] = 0.0f;                     m[15] = 1.0f;
}

/*********************************************************************************************
 * @brief   Erstellt Rotationsmatrix für eine Drehhung um eine belibige Achse.
 * 
 * @param m     matrix[16]
 * @param sp    Achse Startpunkt sp[3]
 * @param ep    Achse Endpunkt ep[3]
 * @param alpha in [rad]
 */
void mat4Rot_um_Achse (float *m, float *sp, float *ep, float alpha)
{
  float n[3], s_p[3];
  float mt[16], m_t[16], mr[16];

  vec3sub (ep, sp, n);
  vec3Normalize (n);
  vec3mul_faktor (sp, -1.0f, s_p);

  mat4translate (mt, sp);         // translations-matrix bilden
  mat4Roate (mr, alpha, n);       // rotations-matix bilden
  mat4translate (m_t, s_p);       // negative translation

  mat4Multiply (mt, mr, m);
  mat4Multiply (m, m_t, m);
}

#endif