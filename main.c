/*
Francesco Falcon Sm3201408
 *
 * Descrizione:
 *   Punto di ingresso del programma di ray casting (Parte 1 - C).
 *   Flusso:
 *     - Parsing argomenti CLI.
 *     - Caricamento scena da file testuale con fscanf (scene_load).
 *     - Rendering parallelo via OpenMP (#pragma omp parallel for):
 *       un raggio per pixel, intersezione con sfere e scelta colore.
 *     - Salvataggio immagine in formato PPM P6 usando mmap (ppm_write_mmap).
 *   Errori:
 *     stampa su stderr e ritorna codice > 0 senza generare crash.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <omp.h>

#include "scene.h"
#include "ppm.h"

// Funzione main:
// - Input: argv[1]=file scena, argv[2]=file PPM uscita, argv[3]=larghezza, argv[4]=altezza
// - Output: 0 se successo; >0 se errore (formato file, allocazione, I/O)
// - Note: rilascia risorse allocate e non causa segmentation fault.
int main(int argc, char** argv){
    if(argc != 5){
        fprintf(stderr, "Uso: %s <scene.txt> <out.ppm> <width> <height>\n", argv[0]);
        return 1;
    }
    const char* scenePath = argv[1];
    const char* outPath = argv[2];
    // Parsing robusto di width/height con strtol:
    // - intercetta overflow/underflow (errno == ERANGE)
    // - rifiuta caratteri extra (controllo end pointer)
    // - accetta solo interi positivi nel range di int
    char *end1=NULL, *end2=NULL; errno = 0;
    long w = strtol(argv[3], &end1, 10);
    while(end1 && *end1 && isspace((unsigned char)*end1)) end1++;
    long h = strtol(argv[4], &end2, 10);
    while(end2 && *end2 && isspace((unsigned char)*end2)) end2++;
    if (errno==ERANGE || w<=0 || h<=0 || w>INT_MAX || h>INT_MAX || (end1 && *end1!='\0') || (end2 && *end2!='\0')){
        fprintf(stderr, "Dimensioni invalide\n");
        return 2;
    }
    int width = (int)w;
    int height = (int)h;

    scene S; int rc = scene_load(scenePath, &S);
    if(rc!=0){ fprintf(stderr, "Errore caricando la scena (%d)\n", rc); return 3; }

    // Buffer immagine RGB8 in layout row-major (riga per riga, dall'alto verso il basso)
    size_t npx = (size_t)width * (size_t)height;
    rgb24* img = (rgb24*)malloc(npx * sizeof(rgb24));
    if(!img){ fprintf(stderr, "Alloc immagine fallita\n"); scene_free(&S); return 4; }

    // Parallelizza sulle righe: ogni thread calcola un sottoinsieme di righe.
    // L'indice lineare è j*width + i (row-major).
    #pragma omp parallel for schedule(static)
    for(int j=0; j<height; ++j){
        for(int i=0; i<width; ++i){
            img[(size_t)j*width + i] = scene_trace_pixel(&S, i, j, width, height);
        }
    }

    // Scrive su disco in formato PPM P6 usando memoria mappata (Windows/Posix)
    int wrc = ppm_write_mmap(outPath, width, height, img);
    if(wrc!=0){ fprintf(stderr, "Errore scrittura PPM (%d)\n", wrc); free(img); scene_free(&S); return 5; }

    free(img);
    scene_free(&S);
    // Codice 0 = successo
    return 0;
}
