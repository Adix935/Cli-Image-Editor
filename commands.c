#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "commands.h"
#include "memory.h"

static inline int clamp(int val) {
	if (val < 0) return 0;
	if (val > 255) return 255;
	return val;
}

static int is_pow2(int n) {
	if (n < 2 || n > 256) return 0;
	return (n & (n - 1)) == 0;
}

void selection(image_t *image, char *input) {
	if (!image->was_loaded) {
		printf("No image loaded\n");
		return;
	}

	char type[20];
	// Extragem primul cuvânt de după "SELECT " ignorând whitespace-ul
	if (sscanf(input + 7, "%s", type) != 1) {
		printf("Invalid command\n");
		return;
	}
	
	// Verificăm dacă este "ALL"
	if (strcmp(type, "ALL") == 0) {
		image->select = (coords_t){0, 0, image->width, image->height};
		printf("Selected ALL\n");
		return;
	}

	// Dacă nu a fost "ALL", înseamnă că așteptăm 4 coordonate
	int x1, y1, x2, y2;
	if (sscanf(input + 7, "%d %d %d %d", &x1, &y1, &x2, &y2) != 4) {
		printf("Invalid command\n");
		return;
	}

	// Interschimbăm dacă e nevoie ca (x1, y1) să fie colțul din stânga sus
	if (x1 > x2) {
		int tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	if (y1 > y2) {
		int tmp = y1;
		y1 = y2;
		y2 = tmp;
	}

	// Validarea încadrării în imagine
	if (x1 < 0 || y1 < 0 || x2 > image->width || y2 > image->height || x1 == x2 || y1 == y2) {
		printf("Invalid set of coordinates\n");
		return;
	}

	image->select = (coords_t){x1, y1, x2, y2};
	printf("Selected %d %d %d %d\n", x1, y1, x2, y2);
}

void histogram(image_t *image, char *input) {
	if (!image->was_loaded) {
		printf("No image loaded\n");
		return;
	}

	int max_stars, bins;
	char extra[100];

	int parsed = sscanf(input, "%*s %d %d %s", &max_stars, &bins, extra);

	if (parsed != 2) {
		printf("Invalid command\n");
		return;
	}

	if (image->type == RGB) {
		printf("Black and white image needed\n");
		return;
	}

	if (!is_pow2(bins)) {
		printf("Invalid set of parameters\n");
		return;
	}

	int *freq = (int *)calloc(bins, sizeof(int));
	int bin_size = 256 / bins;

	for (int i = 0; i < image->height; i++) {
		for (int j = 0; j < image->width; j++) {
			freq[image->bw[i][j] / bin_size]++;
		}
	}

	int max_freq = 0;
	for (int i = 0; i < bins; i++) {
		if (freq[i] > max_freq) max_freq = freq[i];
	}

	for (int i = 0; i < bins; i++) {
		int stars = 0;
		if (max_freq > 0) {
			stars = (freq[i] * max_stars) / max_freq;
		}
		printf("%d\t| ", stars);
		for (int j = 0; j < stars; j++) printf("*");
		printf("\n");
	}

	free(freq);
}

void equalize(image_t *image) {
	if (!image->was_loaded) {
		printf("No image loaded\n");
		return;
	}

	if (image->type == RGB) {
		printf("Black and white image needed\n");
		return;
	}

	int h[256] = {0};
	for (int i = 0; i < image->height; i++) {
		for (int j = 0; j < image->width; j++) {
			h[image->bw[i][j]]++;
		}
	}

	double area = (double)(image->width * image->height);
	for (int i = 0; i < image->height; i++) {
		for (int j = 0; j < image->width; j++) {
			long long sum = 0;
			for (int k = 0; k <= image->bw[i][j]; k++) {
				sum += h[k];
			}
			double val = (255.0 * sum) / area;
			image->bw[i][j] = clamp((int)round(val));
		}
	}

	printf("Equalize done\n");
}

// Internal function to compute weighted sum for BW applying filters
static int get_weighted_sum_bw(image_t *img, int y, int x, double kernel[3][3]) {
	double sum = 0;
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			sum += img->bw[y + i][x + j] * kernel[i + 1][j + 1];
		}
	}
	return clamp((int)round(sum));
}

// Internal function to compute weighted sum for RGB applying filters
static colors_t get_weighted_sum_rgb(image_t *img, int y, int x, double kernel[3][3]) {
	double sum_r = 0, sum_g = 0, sum_b = 0;
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			sum_r += img->rgb[y + i][x + j].r * kernel[i + 1][j + 1];
			sum_g += img->rgb[y + i][x + j].g * kernel[i + 1][j + 1];
			sum_b += img->rgb[y + i][x + j].b * kernel[i + 1][j + 1];
		}
	}
	return (colors_t){clamp((int)round(sum_r)), clamp((int)round(sum_g)), clamp((int)round(sum_b))};
}

void apply_filter(image_t *image, char *input) {
	if (!image->was_loaded) {
		printf("No image loaded\n");
		return;
	}

	char filter_type[100];
	if (sscanf(input, "%*s %s", filter_type) != 1) {
		printf("Invalid command\n");
		return;
	}

	double kernel[3][3];
	bool valid_filter = false;

	if (strcmp(filter_type, "EDGE") == 0) {
		double edge[3][3] = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};
		memcpy(kernel, edge, sizeof(edge));
		valid_filter = true;
	} else if (strcmp(filter_type, "SHARPEN") == 0) {
		double sharpen[3][3] = {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}};
		memcpy(kernel, sharpen, sizeof(sharpen));
		valid_filter = true;
	} else if (strcmp(filter_type, "BLUR") == 0) {
		double blur[3][3] = {{1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0}, {1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0}, {1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0}};
		memcpy(kernel, blur, sizeof(blur));
		valid_filter = true;
	} else if (strcmp(filter_type, "GAUSSIAN_BLUR") == 0) {
		double gaussian[3][3] = {{1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0}, {2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0}, {1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0}};
		memcpy(kernel, gaussian, sizeof(gaussian));
		valid_filter = true;
	}

	if (!valid_filter) {
		printf("APPLY parameter invalid\n");
		return;
	}

	if (image->type == GRAY) {
		printf("Easy, Charlie Chaplin\n");
		return;
	}


	colors_t **temp_rgb = matrix_colors_alloc(image->width, image->height);
	if (!temp_rgb) return;

	for (int i = 0; i < image->height; i++) {
		for (int j = 0; j < image->width; j++) {
			temp_rgb[i][j] = image->rgb[i][j];
		}
	}

	int start_y = (image->select.y1 == 0) ? 1 : image->select.y1;
	int start_x = (image->select.x1 == 0) ? 1 : image->select.x1;
	int end_y = (image->select.y2 == image->height) ? image->height - 1 : image->select.y2;
	int end_x = (image->select.x2 == image->width) ? image->width - 1 : image->select.x2;

	// 3. Aplicăm filtrul doar pe selecția restrânsă
	for (int i = start_y; i < end_y; i++) {
		for (int j = start_x; j < end_x; j++) {
			temp_rgb[i][j] = get_weighted_sum_rgb(image, i, j, kernel);
		}
	}

	free_rgb(image->height, image->rgb);
	image->rgb = temp_rgb;

	printf("APPLY %s done\n", filter_type);
}
