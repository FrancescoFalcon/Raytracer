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
    int width = atoi(argv[3]);
    int height = atoi(argv[4]);
    if(width<=0 || height<=0){ fprintf(stderr, "Dimensioni invalide\n"); return 2; }

    scene S; int rc = scene_load(scenePath, &S);
    if(rc!=0){ fprintf(stderr, "Errore caricando la scena (%d)\n", rc); return 3; }

    size_t npx = (size_t)width * (size_t)height;
    rgb24* img = (rgb24*)malloc(npx * sizeof(rgb24));
    if(!img){ fprintf(stderr, "Alloc immagine fallita\n"); scene_free(&S); return 4; }

    // Parallelizza sulle righe
    #pragma omp parallel for schedule(static)
    for(int j=0; j<height; ++j){
        for(int i=0; i<width; ++i){
            img[(size_t)j*width + i] = scene_trace_pixel(&S, i, j, width, height);
        }
    }

    int wrc = ppm_write_mmap(outPath, width, height, img);
    if(wrc!=0){ fprintf(stderr, "Errore scrittura PPM (%d)\n", wrc); free(img); scene_free(&S); return 5; }

    free(img);
    scene_free(&S);
    return 0;
}
