#ifndef FILE_IO_H
#define FILE_IO_H

#include "image.h"

void load(const char *filename, image_t *image);
void save(image_t *image, char *line);

#endif