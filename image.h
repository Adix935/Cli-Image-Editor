#ifndef IMAGE_H
#define IMAGE_H

#include <stdbool.h>

#define GRAY 935
#define RGB 2005
#define NMAX 255

// Stores the selection coordinates
typedef struct {
	int x1, y1, x2, y2;
} coords_t;

// Stores the RGB values of a single pixel
typedef struct {
	int r, g, b;
} colors_t;

// Main structure representing an image and its current state
typedef struct {
	int was_loaded;
	int type;
	int width, height;
	int max_int;      // Maximum intensity value for a pixel
	int **bw;         // Grayscale pixel matrix
	colors_t **rgb;   // RGB pixel matrix
	coords_t select;  // Current active selection
} image_t;

#endif
