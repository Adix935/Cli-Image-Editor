#include <stdio.h>
#include <stdlib.h>
#include "memory.h"

// Allocates a 2D array for grayscale images
int **alloc_matrix(int width, int height) {
	int **mat = (int **)malloc(height * sizeof(int *));
	if (!mat) {
		fprintf(stderr, "Dynamic Allocation failed!\n");
		return NULL;
	}

	for (int i = 0; i < height; i++) {
		mat[i] = (int *)malloc(width * sizeof(int));
		if (!mat[i]) {
			fprintf(stderr, "Dynamic Allocation failed!\n");
			for (int j = i - 1; j >= 0; j--) {
				free(mat[j]);
			}
			free(mat);
			return NULL;
		}
	}
	return mat;
}

// Allocates a 2D array for RGB images
colors_t **matrix_colors_alloc(int width, int height) {
	colors_t **mat = (colors_t **)malloc(height * sizeof(colors_t *));
	if (!mat) {
		fprintf(stderr, "Dynamic Allocation failed!\n");
		return NULL;
	}

	for (int i = height - 1; i >= 0; i--) {
		mat[i] = (colors_t *)malloc(width * sizeof(colors_t));
		if (!mat[i]) {
			fprintf(stderr, "Dynamic Allocation failed!\n");
			for (int j = height - 1; j > i; j--) {
				free(mat[j]);
			}
			free(mat);
			return NULL;
		}
	}
	return mat;
}

// Frees memory for a grayscale matrix
void free_gray(int height, int **mat) {
	for (int i = height - 1; i >= 0; i--) {
		free(mat[i]);
	}
	free(mat);
}

// Frees memory for an RGB matrix
void free_rgb(int height, colors_t **mat) {
	for (int i = 0; i < height; i++) {
		free(mat[i]);
	}
	free(mat);
}

// Safely frees the loaded image memory and resets its state
void free_image(image_t *image) {
	if (image->was_loaded == 0) {
		return;
	}

	if (image->type == RGB) {
		free_rgb(image->height, image->rgb);
	} else {
		free_gray(image->height, image->bw);
	}
	image->was_loaded = false;
}
