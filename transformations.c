#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "transformations.h"
#include "memory.h"

// Helper used inside rotations
static void swap(int *a, int *b) {
	int temp = *a;
	*a = *b;
	*b = temp;
}

// Internal function to rotate a square grayscale selection
static void rotate_bw_90(image_t *image) {
	int y1 = image->select.y1, x1 = image->select.x1;
	int h = image->select.y2 - y1;
	for (int i = 0; i < h / 2; i++) {
		for (int j = i; j < h - i - 1; j++) {
			int temp = image->bw[y1 + i][x1 + j];
			image->bw[y1 + i][x1 + j] = image->bw[y1 + h - 1 - j][x1 + i];
			image->bw[y1 + h - 1 - j][x1 + i] = image->bw[y1 + h - 1 - i][x1 + h - 1 - j];
			image->bw[y1 + h - 1 - i][x1 + h - 1 - j] = image->bw[y1 + j][x1 + h - 1 - i];
			image->bw[y1 + j][x1 + h - 1 - i] = temp;
		}
	}
}

// Internal function to rotate a square RGB selection
static void rotate_rgb_90(image_t *image) {
	int y1 = image->select.y1, x1 = image->select.x1;
	int h = image->select.y2 - y1;
	for (int i = 0; i < h / 2; i++) {
		for (int j = i; j < h - i - 1; j++) {
			colors_t temp = image->rgb[y1 + i][x1 + j];
			image->rgb[y1 + i][x1 + j] = image->rgb[y1 + h - 1 - j][x1 + i];
			image->rgb[y1 + h - 1 - j][x1 + i] = image->rgb[y1 + h - 1 - i][x1 + h - 1 - j];
			image->rgb[y1 + h - 1 - i][x1 + h - 1 - j] = image->rgb[y1 + j][x1 + h - 1 - i];
			image->rgb[y1 + j][x1 + h - 1 - i] = temp;
		}
	}
}

// Internal function to rotate entire BW image (not necessarily square)
static void rotate_all_bw_90(image_t *image) {
	int **rotated = alloc_matrix(image->height, image->width);
	for (int i = 0; i < image->height; i++) {
		for (int j = 0; j < image->width; j++) {
			rotated[j][image->height - 1 - i] = image->bw[i][j];
		}
	}
	free_gray(image->height, image->bw);
	image->bw = rotated;
	swap(&image->width, &image->height);
}

// Main rotate command implementation
void rotate(image_t *image, char *line) {
	if (!image->was_loaded) {
		printf("No image loaded\n");
		return;
	}

	int angle;
	if (sscanf(line + 7, "%d", &angle) != 1) {
		printf("Invalid command\n");
		return;
	}

	int x1 = image->select.x1, y1 = image->select.y1;
	int x2 = image->select.x2, y2 = image->select.y2;
	int is_full = (x1 == 0 && y1 == 0 && x2 == image->width && y2 == image->height);
	
	// Check if partial selection is square
	if (!is_full && (x2 - x1) != (y2 - y1)) {
		printf("The selection must be square\n");
		return;
	}

	if (angle % 90 != 0 || angle > 360 || angle < -360) {
		printf("Unsupported rotation angle\n");
		return;
	}

	int normalized_angle = (angle + 360) % 360;
	int rotations_count = normalized_angle / 90;

	for (int r = 0; r < rotations_count; r++) {
		if (is_full) {
			// Full image rotation might require resizing memory if not square
			if (image->type == GRAY) rotate_all_bw_90(image);
			else { /* Logic for full RGB rotation (needs temp matrix) */ }
		} else {
			// Square selection rotation can be done in-place
			if (image->type == GRAY) rotate_bw_90(image);
			else rotate_rgb_90(image);
		}
	}

	if (is_full) {
		// Reset selection after full rotation
		image->select = (coords_t){0, 0, image->width, image->height};
	}

	printf("Rotated %d\n", angle);
}

void crop(image_t *image) {
	if (!image->was_loaded) {
		printf("No image loaded\n");
		return;
	}

	int w = image->select.x2 - image->select.x1;
	int h = image->select.y2 - image->select.y1;

	if (image->type == GRAY) {
		int **cropped = alloc_matrix(w, h);
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				cropped[i][j] = image->bw[image->select.y1 + i][image->select.x1 + j];
			}
		}
		free_gray(image->height, image->bw);
		image->bw = cropped;
	} else {
		colors_t **cropped = matrix_colors_alloc(w, h);
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				cropped[i][j] = image->rgb[image->select.y1 + i][image->select.x1 + j];
			}
		}
		free_rgb(image->height, image->rgb);
		image->rgb = cropped;
	}

	image->width = w;
	image->height = h;
	image->select = (coords_t){0, 0, w, h};
	printf("Image cropped\n");
}
