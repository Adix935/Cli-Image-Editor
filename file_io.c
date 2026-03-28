#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_io.h"
#include "memory.h"

// Skips comment lines starting with '#' in the Netpbm file
static int ignore_comments(FILE *file, char *chur) {
	int byte_count = 0;
	while (fread(chur, sizeof(char), 1, file) == 1) {
		byte_count++;
		if (*chur == '#') {
			do {
				if (fread(chur, sizeof(char), 1, file) != 1) break;
				byte_count++;
			} while (*chur != '\n');
		} else {
			break;
		}
	}
	return byte_count;
}

// Parses the Netpbm header (Magic Number, dimensions, max value)
static int parse_header(FILE *file, int *width, int *height, int *type, int *max_int) {
	char current_char;
	ignore_comments(file, &current_char);

	if (current_char != 'P') return -1;
	if (fread(&current_char, sizeof(char), 1, file) != 1) return -1;
	*type = current_char - '0';
	
	fread(&current_char, sizeof(char), 1, file); /* Skip whitespace */
	ignore_comments(file, &current_char);

	*width = 0;
	while (current_char != ' ' && current_char != '\n') {
		*width = (*width) * 10 + (current_char - '0');
		fread(&current_char, sizeof(char), 1, file);
	}

	*height = 0;
	fread(&current_char, sizeof(char), 1, file);
	while (current_char != ' ' && current_char != '\n') {
		*height = (*height) * 10 + (current_char - '0');
		fread(&current_char, sizeof(char), 1, file);
	}

	if (*type == 2 || *type == 3 || *type == 5 || *type == 6) {
		ignore_comments(file, &current_char);
		*max_int = 0;
		while (current_char != ' ' && current_char != '\n') {
			*max_int = (*max_int) * 10 + (current_char - '0');
			fread(&current_char, sizeof(char), 1, file);
		}
	}
	// Return offset where pixel data begins
	return ftell(file); 
}

//Top-level load function
void load(const char *filename, image_t *image) {

	if (image->was_loaded) {
		free_image(image);
		image->was_loaded = 0;
	}
	FILE *file = fopen(filename, "rb");
	if (!file) {
		printf("Failed to load %s\n", filename);
		return;
	}

	if (image->was_loaded) {
		free_image(image);
	}

	int type, width, height, max_int;
	int offset = parse_header(file, &width, &height, &type, &max_int);
	
	if (offset < 0) {
		printf("Failed to load %s\n", filename);
		fclose(file);
		return;
	}

	image->width = width;
	image->height = height;
	image->max_int = max_int;
	image->type = (type == 2 || type == 5) ? GRAY : RGB;
	
	if (image->type == GRAY) {
		image->bw = alloc_matrix(width, height);
		if (type == 2) { // ASCII Grayscale
			for (int i = 0; i < height; i++) {
				for (int j = 0; j < width; j++) fscanf(file, "%d", &image->bw[i][j]);
			}
		} else { //Binary Grayscale
			fseek(file, offset, SEEK_SET);
			unsigned char *buf = malloc(width * height);
			fread(buf, 1, width * height, file);
			for (int i = 0; i < height; i++) {
				for (int j = 0; j < width; j++) image->bw[i][j] = buf[i * width + j];
			}
			free(buf);
		}
	} else {
		image->rgb = matrix_colors_alloc(width, height);
		// ASCII RGB
		if (type == 3) {
			for (int i = 0; i < height; i++) {
				for (int j = 0; j < width; j++) {
					fscanf(file, "%d %d %d", &image->rgb[i][j].r, &image->rgb[i][j].g, &image->rgb[i][j].b);
				}
			}
		} else {
			// Binary RGB
			fseek(file, offset, SEEK_SET);
			for (int i = 0; i < height; i++) {
				for (int j = 0; j < width; j++) {
					unsigned char rgb[3];
					fread(rgb, 1, 3, file);
					image->rgb[i][j].r = rgb[0];
					image->rgb[i][j].g = rgb[1];
					image->rgb[i][j].b = rgb[2];
				}
			}
		}
	}

	image->was_loaded = 1;
	image->select = (coords_t){0, 0, width, height};
	printf("Loaded %s\n", filename);
	fclose(file);
}

void save(image_t *image, char *line) {
	if (!image->was_loaded) {
		printf("No image loaded\n");
		return;
	}

	char filename[100];
	char format[20] = "";
	int is_ascii = 0;

	int args_read = sscanf(line + 5, "%s %s", filename, format);
	
	if (args_read < 1) {
		printf("Invalid command\n");
		return;
	}

	if (args_read == 2 && strcmp(format, "ascii") == 0) {
		is_ascii = 1;
	}

	FILE *file = fopen(filename, is_ascii ? "w" : "wb");
	if (!file) {
		printf("Failed to save\n");
		return;
	}

	int magic = (image->type == GRAY) ? (is_ascii ? 2 : 5) : (is_ascii ? 3 : 6);
	fprintf(file, "P%d\n%d %d\n%d\n", magic, image->width, image->height, image->max_int);

	for (int i = 0; i < image->height; i++) {
		for (int j = 0; j < image->width; j++) {
			if (image->type == GRAY) {
				if (is_ascii) fprintf(file, "%d ", image->bw[i][j]);
				else {
					unsigned char p = image->bw[i][j];
					fwrite(&p, 1, 1, file);
				}
			} else {
				if (is_ascii) fprintf(file, "%d %d %d ", image->rgb[i][j].r, image->rgb[i][j].g, image->rgb[i][j].b);
				else {
					unsigned char p[3] = {image->rgb[i][j].r, image->rgb[i][j].g, image->rgb[i][j].b};
					fwrite(p, 1, 3, file);
				}
			}
		}
		if (is_ascii) fprintf(file, "\n");
	}

	fclose(file);
	printf("Saved %s\n", filename);
}
