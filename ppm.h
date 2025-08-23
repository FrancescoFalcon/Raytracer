/*
Francesco Falcon Sm3201408
 */
#ifndef PPM_H
#define PPM_H

#include <stdint.h>
#include <stddef.h>
#include "scene.h"

int ppm_write_mmap(const char* path, int width, int height, const rgb24* data);

#endif // PPM_H
/*
Francesco Falcon Sm3201408
 */
#ifndef PPM_H
#define PPM_H

#include <stdint.h>
#include <stddef.h>
#include "scene.h"

// Scrive immagine PPM (P6) usando mmap in modo portabile (POSIX e Windows).
// Formato P6: header ASCII "P6\n<width> <height>\n255\n" seguito da width*height pixel
// in ordine riga-per-riga dall'alto verso il basso, ciascuno come 3 byte RGB.
// Ritorna 0 se ok, !=0 su errore.
int ppm_write_mmap(const char* path, int width, int height, const rgb24* data);

#endif // PPM_H
