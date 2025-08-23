/*
Francesco Falcon Sm3201408
 */
#ifndef SCENE_H
#define SCENE_H

#include <stdint.h>
#include <stddef.h>
#include <math.h>

#ifdef __GNUC__
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif

// Tipo float richiesto
// Tutte le operazioni in virgola mobile del progetto utilizzano 'float'
typedef float f32;

// Vettore 3D con float
// Rappresenta coordinate o direzioni nello spazio della camera
typedef struct { f32 x, y, z; } vec3;

// Colore RGB 24-bit packed
// Ordine canali: R, G, B; ciascuno su 8 bit come richiesto dal PPM P6
typedef struct PACKED { uint8_t r, g, b; } rgb24;

// Viewport: larghezza (x), altezza (y), distanza dalla camera (z)
// Rettangolo perpendicolare all'asse Z e centrato sull'origine; la camera è in (0,0,0)
typedef struct { f32 x, y, z; } viewport;

// Sfera: centro, raggio, colore
typedef struct { vec3 c; f32 r; rgb24 color; } sphere;

// Scena
typedef struct {
    viewport vp;
    rgb24 background;
    size_t n;
    sphere* objs; // array di sfere, sola lettura dopo il parsing
} scene;

// Parsing file scena (.txt). Ritorna 0 se ok, !=0 se errore.
// Formato supportato:
//  VP <float x> <float y> <float z>
//  BG <uint8 r> <uint8 g> <uint8 b>
//  OBJ_N <int n>
//  S <cx> <cy> <cz> <r> <R> <G> <B>   (ripetuto n volte)
int scene_load(const char* path, scene* out);

// Libera risorse allocate per la scena.
void scene_free(scene* s);

// Calcola il colore del pixel (i,j) dell'immagine width x height.
// Implementa il ray casting: genera un raggio per pixel, trova l'intersezione
// più vicina e ritorna il colore dell'oggetto o del background.
rgb24 scene_trace_pixel(const scene* s, int i, int j, int width, int height);

// Utility matematiche
static inline vec3 v_add(vec3 a, vec3 b){ return (vec3){a.x+b.x,a.y+b.y,a.z+b.z}; }
static inline vec3 v_sub(vec3 a, vec3 b){ return (vec3){a.x-b.x,a.y-b.y,a.z-b.z}; }
static inline vec3 v_scale(vec3 a, f32 k){ return (vec3){a.x*k,a.y*k,a.z*k}; }
static inline f32  v_dot(vec3 a, vec3 b){ return a.x*b.x + a.y*b.y + a.z*b.z; }
static inline f32  v_len2(vec3 a){ return v_dot(a,a); }
static inline vec3 v_norm(vec3 a){ f32 l2=v_len2(a); if(l2>0){f32 inv=1.0f/(f32)sqrt(l2); return v_scale(a,inv);} return a; }

#endif // SCENE_H
/*
Francesco Falcon Sm3201408
 */
#ifndef SCENE_H
#define SCENE_H

#include <stdint.h>
#include <stddef.h>
#include <math.h>

#ifdef __GNUC__
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif

// Tipo float richiesto
// Tutte le operazioni in virgola mobile del progetto utilizzano 'float'
typedef float f32;

// Vettore 3D con float
// Rappresenta coordinate o direzioni nello spazio della camera
typedef struct { f32 x, y, z; } vec3;

// Colore RGB 24-bit packed
// Ordine canali: R, G, B; ciascuno su 8 bit come richiesto dal PPM P6
typedef struct PACKED { uint8_t r, g, b; } rgb24;

// Viewport: larghezza (x), altezza (y), distanza dalla camera (z)
// Rettangolo perpendicolare all'asse Z e centrato sull'origine; la camera è in (0,0,0)
typedef struct { f32 x, y, z; } viewport;

// Sfera: centro, raggio, colore
typedef struct { vec3 c; f32 r; rgb24 color; } sphere;

// Scena
typedef struct {
    viewport vp;
    rgb24 background;
    size_t n;
    sphere* objs; // array di sfere, sola lettura dopo il parsing
} scene;

// Parsing file scena (.txt). Ritorna 0 se ok, !=0 se errore.
// Formato supportato:
//  VP <float x> <float y> <float z>
//  BG <uint8 r> <uint8 g> <uint8 b>
//  OBJ_N <int n>
//  S <cx> <cy> <cz> <r> <R> <G> <B>   (ripetuto n volte)
int scene_load(const char* path, scene* out);

// Libera risorse allocate per la scena.
void scene_free(scene* s);

// Calcola il colore del pixel (i,j) dell'immagine width x height.
// Implementa il ray casting: genera un raggio per pixel, trova l'intersezione
// più vicina e ritorna il colore dell'oggetto o del background.
rgb24 scene_trace_pixel(const scene* s, int i, int j, int width, int height);

// Utility matematiche
static inline vec3 v_add(vec3 a, vec3 b){ return (vec3){a.x+b.x,a.y+b.y,a.z+b.z}; }
static inline vec3 v_sub(vec3 a, vec3 b){ return (vec3){a.x-b.x,a.y-b.y,a.z-b.z}; }
static inline vec3 v_scale(vec3 a, f32 k){ return (vec3){a.x*k,a.y*k,a.z*k}; }
static inline f32  v_dot(vec3 a, vec3 b){ return a.x*b.x + a.y*b.y + a.z*b.z; }
static inline f32  v_len2(vec3 a){ return v_dot(a,a); }
static inline vec3 v_norm(vec3 a){ f32 l2=v_len2(a); if(l2>0){f32 inv=1.0f/(f32)sqrt(l2); return v_scale(a,inv);} return a; }

#endif // SCENE_H
