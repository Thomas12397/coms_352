#include <stdio.h>

int main() {

  
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
