/*
	Author: Thomas Haddy
	Date:	4/23/19
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>

void part_b(char *in_path, char *out_path);

int main() {

	int length, count;
	char *in_path, *out_path;
	char ch;

	length = 20; //Initial path size

	in_path = malloc(length * sizeof(char));
	printf("Please enter the path of the file you wish to uppercase its contents: \n");
	scanf("%s", in_path);

	//Allocate more mem to in_path if needed
	while((ch = getc(stdin)) != '\n') {
		if (count >= length) {
			in_path = realloc(in_path, (length += 10) * sizeof(char));
		}
		in_path[count++] = ch;
	}
	count = 0, length = 20;

	out_path = malloc(length * sizeof(char));
	printf("Please enter the path of the output file: \n");
	scanf("%s", out_path);

	//Allocate more mem to out_path if needed
	while((ch = getc(stdin)) != '\n') {
		if (count >= length) {
			out_path = realloc(in_path, (length += 10) * sizeof(char));
		}
		out_path[count++] = ch;
	}

	part_b(in_path, out_path);

	free(in_path);
	free(out_path);
	return 0;
}

void part_b(char *in_path, char *out_path) {

	FILE *file_in = fopen(in_path, "r");
	if (!file_in) {
		fprintf(stderr, "Input file with path \"%s\" could not be opened. The path starts from the current terminal directory you are in.\n", in_path);
		return;
	}

	FILE *file_out = fopen(out_path, "w");
	if (!file_out) {
		fprintf(stderr, "Output file with path \"%s\" could not be opened. The path starts from the current terminal directory you are in.\n", out_path);
		return;
	}

	//Actual problem
	clock_t start, stop;
	double time_spent;
	char ch;

	start = clock();
	while (fread(&ch ,sizeof(char), 1, file_in)) {
		ch = toupper(ch);
		fwrite(&ch ,sizeof(char), 1, file_out);
	}
	stop = clock();
	time_spent = (double)(stop - start) / CLOCKS_PER_SEC;
	printf("Runtime: %f seconds\n", time_spent);

	fclose(file_in);
	fclose(file_out);
}