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
