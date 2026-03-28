#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "memory.h"
#include "file_io.h"
#include "transformations.h"
#include "commands.h"

int main(void) {
	image_t img = { .was_loaded = 0 };
	char input[100], cmd[20];

	while (fgets(input, sizeof(input), stdin)) {
		// Strip newline
		input[strcspn(input, "\n")] = 0;

		if (sscanf(input, "%s", cmd) == EOF) break;

		if (strcmp(cmd, "LOAD") == 0) {
			char file_path[100];
			if (sscanf(input + 5, "%s", file_path) == 1) load(file_path, &img);
		} else if (strcmp(cmd, "SAVE") == 0) {
			save(&img, input);
		} else if (strcmp(cmd, "EXIT") == 0) {
			if (!img.was_loaded) printf("No image loaded\n");
			break;
		} else if (strcmp(cmd, "SELECT") == 0) {
			selection(&img, input);
		} else if (strcmp(cmd, "CROP") == 0) {
			crop(&img);
		} else if (strcmp(cmd, "EQUALIZE") == 0) {
			equalize(&img);
		} else if (strcmp(cmd, "HISTOGRAM") == 0) {
			histogram(&img, input);
		} else if (strcmp(cmd, "ROTATE") == 0) {
			rotate(&img, input);
		} else if (strcmp(cmd, "APPLY") == 0) {
			apply_filter(&img, input);
		} else {
			printf("Invalid command\n");
		}
	}

	if (img.was_loaded) free_image(&img);
	return 0;
}
