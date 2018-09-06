#include <stdio.h>
#include <fcntl.h>
#include "file.h"

void File_writeIntToFile(char *filepath, int input) {
	FILE *file = fopen(filepath, "w");
	if (file == NULL) {
		printf("writeIntToFile: Error opening %s\n", filepath);
	}
	fprintf(file, "%d", input);
	fclose(file);
}

void File_writeStringToFile(char *filepath, const char *input) {
	FILE *file = fopen(filepath, "w");
	if (file == NULL) {
		printf("Error opening %s", filepath);
	}
	int charWritten = fprintf(file, "%s", input);
	if (charWritten <= 0) {
		printf("Error writing data: %s\n", input);
	}
	fclose(file);
}