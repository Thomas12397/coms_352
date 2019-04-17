#include <stdio.h>
#include <stdlib.h>

#include "alloc.h"

int main() {

  char resp_y_n[2];
  char resp_res_num_val[4];

  //readFile();
  mapFile();
  
  printf("Allocate more resources(y/n)? ");
  scanf("%c", resp_y_n);
  if (*resp_y_n == 'y') {
    
  }
  else if (*resp_y_n == 'n') {
    return 0;
  }
  else {
    printf("\nYou didn't type (y/n). Exiting...\n");
    return 0;
  }
  
  
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

void mapFile() {

  FILE *file;
  int size = 0;
  file = fopen("res.txt", "r");
  
  fseek(file, 0, SEEK_END);
  size = ftell(file);
  rewind(file);
  printf("%d\n", size);

  fclose(file);
}
