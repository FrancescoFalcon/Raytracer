/*
Francesco Falcon Sm3201408
 */
#include "scene.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

void scene_free(scene* s){
    if(!s) return;
    free(s->objs); s->objs=NULL; s->n=0;
}

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

rgb24 scene_trace_pixel(const scene* s, int i, int j, int width, int height){
    f32 x = ((f32)i + 0.5f) - (f32)width*0.5f;
    f32 y = ((f32)j + 0.5f) - (f32)height*0.5f;
    f32 vx = (x / (f32)width) * s->vp.x;
    f32 vy = (-(y) / (f32)height) * s->vp.y;
    f32 vz = s->vp.z;
    vec3 V = v_norm((vec3){vx, vy, vz});

    f32 bestT = -1.0f; size_t bestIdx = (size_t)-1;
    for(size_t k=0;k<s->n;k++){
        f32 t = ray_sphere_intersect(V, &s->objs[k]);
        if(t > 0.0f && (bestT < 0.0f || t < bestT)) { bestT = t; bestIdx = k; }
    }
    if(bestT > 0.0f) return s->objs[bestIdx].color;
    return s->background;
}
/*
Francesco Falcon Sm3201408
 */
#include "scene.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Contratto funzioni principali:
// - scene_load(path, out):
//     Legge il file di scena nel formato del PDF (VP/BG/OBJ_N/S) usando fscanf.
//     Alloca l'array di sfere e popola 'out'. Ritorna 0 se ok, >0 in caso di errori
//     (I/O, formato, allocazione). In caso di fallimento libera le risorse.
// - scene_trace_pixel(s, i, j, width, height):
//     Genera il raggio dal centro camera (0,0,0) verso il punto del viewport
//     corrispondente al pixel (i,j), risolve l'intersezione con tutte le sfere
//     e restituisce il colore dell'intersezione più vicina o il background.

static int parse_line_header(FILE* f, char* tag, size_t taglen){
    int c;
    // salta whitespace e newline
    do { c = fgetc(f); if(c==EOF) return -1; } while(c=='\n' || c=='\r' || c==' ' || c=='\t');
    // leggi tag fino a spazio o newline
    size_t idx=0;
    tag[idx++] = (char)c;
    while(idx < taglen-1){
        c = fgetc(f);
        if(c==EOF || c==' ' || c=='\t' || c=='\n' || c=='\r') break;
        tag[idx++] = (char)c;
    }
    tag[idx] = '\0';
    // Consuma eventuali spazi extra
    while(c!='\n' && c!='\r' && c!=EOF){ if(c!=' ' && c!='\t'){ ungetc(c,f); break;} c=fgetc(f);}    
    return 0;
}

// Funzione: scene_load
// Input:  path -> percorso del file di scena
//         out  -> puntatore a struct da riempire
// Output: 0 se successo, >0 se errore
// Note:   usa fscanf per leggere VP/BG/OBJ_N/S; in caso di errore libera 'out'
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
            // tag sconosciuto: torna indietro e prova a leggere come whitespace
            fseek(f, pos, SEEK_SET);
            int c=fgetc(f);
            if(c==EOF) break; // fine file
            // ignora riga
            while(c!='\n' && c!=EOF) c=fgetc(f);
        }
    }

    if(!(haveVP&&haveBG&&haveN)){
        fprintf(stderr, "File scena incompleto (VP/BG/OBJ_N mancanti)\n");
        goto fail;
    }
    if(out->n>0){
        // Verifica che tutte le sfere siano state lette
        size_t count=0; for(size_t k=0;k<out->n;k++){ if(out->objs[k].r>0.0f || out->objs[k].color.r||out->objs[k].color.g||out->objs[k].color.b) count++; }
        if(count<out->n){ fprintf(stderr, "Numero di sfere inferiore a OBJ_N\n"); /* non fatal, ma segnaliamo */ }
    }

    fclose(f);
    return 0;
fail:
    if(f) fclose(f);
    scene_free(out);
    return 3;
}

// Funzione: scene_free
// Scopo:    liberare l'array dinamico di sfere e azzerare i campi
void scene_free(scene* s){
    if(!s) return;
    free(s->objs); s->objs=NULL; s->n=0;
}

// Intersezione raggio-sfera: raggio da C=(0,0,0) lungo direzione V normalizzata.
// Ritorna t>0 della prima intersezione, oppure -1 se nessuna.
// Funzione: ray_sphere_intersect
// Input:  V  -> direzione del raggio (normalizzata), origine in C=(0,0,0)
//         sp -> puntatore alla sfera
// Output: distanza t>0 dell'intersezione più vicina lungo V, oppure -1 se nessuna
// Note:   risolve l'equazione quadratica per l'intersezione raggio-sfera.
static f32 ray_sphere_intersect(vec3 V, const sphere* sp){
    // Equazione: (V*t - s)·(V*t - s) = r^2
    // a = V·V, b = -2(s·V), c = s·s - r^2
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

// Funzione: scene_trace_pixel
// Input:  s -> scena; (i,j) -> coordinate pixel; width/height dimensioni immagine
// Output: colore RGB del pixel secondo il modello di ray casting del PDF
rgb24 scene_trace_pixel(const scene* s, int i, int j, int width, int height){
    // Calcolo del vettore direzione del raggio per pixel (i,j)
    // viewport corners: x in [-vp.x/2, vp.x/2], y in [-vp.y/2, vp.y/2], z = vp.z
    // map i in [0,width-1] -> x, j in [0,height-1] -> y (origine top-left)
    f32 x = ((f32)i + 0.5f) - (f32)width*0.5f;    // offset in pixel
    f32 y = ((f32)j + 0.5f) - (f32)height*0.5f;
    // scala a coordinate viewport mantenendo aspect ratio: vp.x corrisponde a width, vp.y a height
    f32 vx = (x / (f32)width) * s->vp.x;
    f32 vy = (-(y) / (f32)height) * s->vp.y; // y crescente verso il basso nell'immagine
    f32 vz = s->vp.z;
    vec3 V = v_norm((vec3){vx, vy, vz});

    // Trova intersezione più vicina
    f32 bestT = -1.0f; size_t bestIdx = (size_t)-1;
    for(size_t k=0;k<s->n;k++){
        f32 t = ray_sphere_intersect(V, &s->objs[k]);
        if(t > 0.0f && (bestT < 0.0f || t < bestT)) { bestT = t; bestIdx = k; }
    }
    if(bestT > 0.0f) return s->objs[bestIdx].color;
    return s->background;
}
