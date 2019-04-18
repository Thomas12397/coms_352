#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "alloc.h"

int main() {

  char resp_y_n;
  int res_type, res_val;

  /*Opening the file*/
  int filedesc = open("res.txt", O_RDWR);
  if (filedesc == -1) {
    fprintf(stderr, "Could not open \"res.txt\"\n");
    exit(EXIT_FAILURE);
  }

  /*Get the file size*/
  struct stat buf;
  fstat(filedesc, &buf);
  off_t size =  buf.st_size;

   /*Map the file*/
  char *map_region = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, filedesc, 0);
  if (map_region == MAP_FAILED) {
    close(filedesc);
    fprintf(stderr, "Error mapping the file");
    exit(EXIT_FAILURE);
  }

  msync(map_region, size, MS_SYNC);
  printf("Synchronized\n");
  
  do {
    printf("\nAllocate more resources(y/n)? ");
    scanf("%c", &resp_y_n);
    
    if (resp_y_n == 'y') {
      printf("Enter the resource number and number of resources needed: ");
      scanf("%d %d", &res_type, &res_val);
    }
    
  } while (resp_y_n != 'n');

   /*Un-map the file*/
  if (munmap(map_region, size) == -1) {
    fprintf(stderr, "Error un-mapping the file");
    exit(EXIT_FAILURE);
  }

  munmap(map_region, size);
  close(filedesc);
  return 0;
}

void readFile() {

  int counter = 0;
  int numEntries = 0;
  
  FILE *file;
  file = fopen("res.txt", "r");

  int size = 0;
  fseek(file, 0, SEEK_END);
  size = ftell(file);
  rewind(file);
  numEntries = size / 4;
  
  int resources[numEntries][2];
  
  if (file) {
    
    int i, j;
    for (i = 0; i < numEntries; i++) {
      for (j = 0; j < 2; j++) {
	fscanf (file, "%d", &resources[i][j]);
	printf("%d ", resources[i][j]);
      }
      printf("\n");
    }
  }
  
  fclose(file);
}
