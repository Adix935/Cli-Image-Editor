#ifndef MEMORY_H
#define MEMORY_H

#include "image.h"

int **alloc_matrix(int width, int height);
colors_t **matrix_colors_alloc(int width, int height);
void free_gray(int height, int **mat);
void free_rgb(int height, colors_t **mat);
void free_image(image_t *image);

#endif
