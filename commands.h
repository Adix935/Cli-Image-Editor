#ifndef COMMANDS_H
#define COMMANDS_H

#include "image.h"

void selection(image_t *image, char *input);
void histogram(image_t *image, char *input);
void equalize(image_t *image);
void apply_filter(image_t *image, char *input);

#endif