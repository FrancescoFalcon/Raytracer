/*
Francesco Falcon Sm3201408
 */
#include "scene.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Legge il "tag" iniziale di una riga (es. VP, BG, OBJ_N, S),
// saltando spazi/break line iniziali e fermandosi al primo separatore.
// Rimette nel buffer di input il primo carattere utile successivo con ungetc,
// così fscanf può proseguire a leggere i valori sulla stessa riga.
static int parse_line_header(FILE* f, char* tag, size_t taglen){
    int c;
    do { c = fgetc(f); if(c==EOF) return -1; } while(c=='\n' || c=='\r' || c==' ' || c=='\t');
    size_t idx=0;
    tag[idx++] = (char)c;
    while(idx < taglen-1){
        c = fgetc(f);
        if(c==EOF || c==' ' || c=='\t' || c=='\n' || c=='\r') break;
        tag[idx++] = (char)c;
    }
    tag[idx] = '\0';
    while(c!='\n' && c!='\r' && c!=EOF){ if(c!=' ' && c!='\t'){ ungetc(c,f); break;} c=fgetc(f);}    
    return 0;
}

// Parser del file scena nel formato richiesto dal compito.
// Popola viewport, background e l'array di sfere (se OBJ_N>0).
// Restituisce 0 su successo, !=0 su errore.
int scene_load(const char* path, scene* out){
    if(!path || !out) return 1;
    memset(out, 0, sizeof(*out));

    FILE* f = fopen(path, "r");
    if(!f){ perror("fopen scena"); return 2; }

    char tag[16]={0};
    int haveVP=0, haveBG=0, haveN=0; 
    size_t expectN=0; size_t nextIdx=0;

    while(1){
        long pos = ftell(f);
        if(parse_line_header(f, tag, sizeof(tag))!=0){ break; }
        if(strcmp(tag, "VP")==0){
            if(fscanf(f, "%f %f %f", &out->vp.x, &out->vp.y, &out->vp.z)!=3){
                fprintf(stderr, "Errore parsing VP\n"); goto fail;
            }
            haveVP=1;
        }else if(strcmp(tag, "BG")==0){
            unsigned r,g,b; 
            if(fscanf(f, "%u %u %u", &r,&g,&b)!=3){ fprintf(stderr, "Errore parsing BG\n"); goto fail; }
            out->background = (rgb24){ (uint8_t)r,(uint8_t)g,(uint8_t)b };
            haveBG=1;
        }else if(strcmp(tag, "OBJ_N")==0){
            if(fscanf(f, "%zu", &expectN)!=1){ fprintf(stderr, "Errore parsing OBJ_N\n"); goto fail; }
            if(expectN==0){ out->objs=NULL; out->n=0; haveN=1; }
            else{
                out->objs = (sphere*)calloc(expectN, sizeof(sphere));
                if(!out->objs){ fprintf(stderr, "Alloc fallita per %zu sfere\n", expectN); goto fail; }
                out->n = expectN; haveN=1;
            }
    }else if(strcmp(tag, "S")==0){
            if(!haveN || (!out->objs && out->n>0)){ fprintf(stderr, "Definisci OBJ_N prima delle sfere\n"); goto fail; }
            sphere s; unsigned r,g,b;
            if(fscanf(f, "%f %f %f %f %u %u %u", &s.c.x, &s.c.y, &s.c.z, &s.r, &r,&g,&b)!=7){
                fprintf(stderr, "Errore parsing S\n"); goto fail;
            }
            s.color = (rgb24){(uint8_t)r,(uint8_t)g,(uint8_t)b};
            if(out->n==0){ /* nessun oggetto atteso */ }
            else {
                if(nextIdx>=out->n){ fprintf(stderr, "Troppe sfere rispetto a OBJ_N\n"); goto fail; }
                out->objs[nextIdx++] = s;
            }
    }else{
            fseek(f, pos, SEEK_SET);
            int c=fgetc(f);
            if(c==EOF) break;
            while(c!='\n' && c!=EOF) c=fgetc(f);
        }
    }

    if(!(haveVP&&haveBG&&haveN)){
        fprintf(stderr, "File scena incompleto (VP/BG/OBJ_N mancanti)\n");
        goto fail;
    }

    fclose(f);
    return 0;
fail:
    if(f) fclose(f);
    scene_free(out);
    return 3;
}

// Rilascio risorse allocate dal parser
void scene_free(scene* s){
    if(!s) return;
    free(s->objs); s->objs=NULL; s->n=0;
}

// Intersezione raggio-sfera con camera all'origine.
// Sfera centrata in sp->c nello spazio camera; raggio con direzione normalizzata V.
// Restituisce il t > 0 più piccolo, o -1 se nessuna intersezione.
static f32 ray_sphere_intersect(vec3 V, const sphere* sp){
    f32 a = v_dot(V,V);
    f32 b = -2.0f * v_dot((vec3){sp->c.x, sp->c.y, sp->c.z}, V);
    f32 c = v_dot((vec3){sp->c.x, sp->c.y, sp->c.z}, (vec3){sp->c.x, sp->c.y, sp->c.z}) - sp->r*sp->r;
    f32 disc = b*b - 4*a*c;
    if(disc < 0) return -1.0f;
    f32 sqrtD = (f32)sqrt(disc);
    f32 t1 = (-b - sqrtD)/(2*a);
    f32 t2 = (-b + sqrtD)/(2*a);
    if(t1>0 && t2>0) return t1 < t2 ? t1 : t2;
    if(t1>0) return t1;
    if(t2>0) return t2;
    return -1.0f;
}

// Calcolo colore del pixel (i,j): proietta il centro del pixel sul viewport,
// costruisce il raggio, trova l'oggetto più vicino e ritorna il colore.
rgb24 scene_trace_pixel(const scene* s, int i, int j, int width, int height){
    f32 x = ((f32)i + 0.5f) - (f32)width*0.5f;
    f32 y = ((f32)j + 0.5f) - (f32)height*0.5f;
    f32 vx = (x / (f32)width) * s->vp.x;
    // Nota: asse Y invertito perché i pixel sono enumerati dall'alto verso il basso
    f32 vy = (-(y) / (f32)height) * s->vp.y;
    f32 vz = s->vp.z;
    vec3 V = v_norm((vec3){vx, vy, vz});

    f32 bestT = -1.0f; size_t bestIdx = (size_t)-1;
    for(size_t k=0;k<s->n;k++){
        f32 t = ray_sphere_intersect(V, &s->objs[k]);
        if(t > 0.0f && (bestT < 0.0f || t < bestT)) { bestT = t; bestIdx = k; }
    }
    if(bestT > 0.0f) return s->objs[bestIdx].color; // hit: colore dell'oggetto più vicino
    return s->background;
}

