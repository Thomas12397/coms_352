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

void part_a(char *in_path, char *out_path);

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

	part_a(in_path, out_path);

	free(in_path);
	free(out_path);
	return 0;
}

void part_a(char *in_path, char *out_path) {

	int file_desc_in = open(in_path, O_RDONLY);
	if (file_desc_in < 0) {
		fprintf(stderr, "Input file with path \"%s\" could not be opened. The path starts from the current terminal directory you are in.\n", in_path);
		return;
	}

	int file_desc_out = open(out_path, O_WRONLY);
	if (file_desc_in < 0) {
		fprintf(stderr, "Output file with path \"%s\" could not be opened. The path starts from the current terminal directory you are in.\n", out_path);
		return;
	}

	//Actual problem
	clock_t start, stop;
	double time_spent;
	char ch;

	start = clock();
	while (read(file_desc_in, &ch, sizeof(char)) == 1) {
		ch = toupper(ch);
		write(file_desc_out, &ch, sizeof(char));
	}
	stop = clock();
	time_spent = (double)(stop - start) / CLOCKS_PER_SEC;
	printf("Runtime: %f seconds\n", time_spent);

	close(file_desc_in);
	close(file_desc_out);
}