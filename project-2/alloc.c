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
  int filedesc, size;
  char *map_region;

  /*Opening the file*/
  filedesc = open_file();

  /*Get the file size*/
  size = get_file_size(filedesc);

   /*Initially map the file*/
  map_region = init_mem_map_file(filedesc, size);
  
  /* User input for program */
  do {
    printf("Allocate more resources(y/n)? ");
    scanf(" %c", &resp_y_n);
  
    if (resp_y_n == 'y') {
      printf("Enter the resource number type: ");
      scanf("%d", &res_type);
      printf("Enter the number of resources needed: ");
      scanf("%d", &res_val);
      alloc_mem_map_file(map_region, size, filedesc, res_type, res_val);
    }

  } while (resp_y_n != 'n');

  /* Deallocate everything */
  unmap_mem_map_file(map_region, size);
  close(filedesc);
  return 0;
}

int open_file() {

  int fd;
  fd = open("res.txt", O_RDWR);
  if (fd == -1) {
    fprintf(stderr, "Could not open \"res.txt\"\n");
    return -1;
  }
  return fd;
}

int get_file_size(int fd) {

  struct stat buf;
  fstat(fd, &buf);
  return buf.st_size;
}

char* init_mem_map_file(int fd, int size) {

  char *map = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (map == MAP_FAILED) {
    close(fd);
    fprintf(stderr, "Error mapping the file");
    return NULL;
  }

  return map;
}

void alloc_mem_map_file(char *map, int size, int fd, int rt, int rv) {
  
  if (rt > 9 || rt < 0 || rv > 9 || rv < 1) {
    fprintf(stderr, "There was an invalid number typed. Must be between 0 and 9\n");
    return;
  }
  
  FILE *file = fdopen(fd, "r+");
  char rv_str[2];
  int i, res_type_read_in, res_val_read_in;
  /*Go through the file */
  for (i = 2; i < size; i = i + 8) {
    fseek(file, i, SEEK_SET);
    fscanf(file, "%d", &res_type_read_in);
    /* Check if the resource types match */
    if (res_type_read_in == rt) {
      fseek(file, i + 4, SEEK_SET);
      fscanf(file, "%d", &res_val_read_in);
      /* Check if the resource value can be subtracted from the corresponding type */
      if (res_val_read_in - rv >= 0) {
	res_val_read_in -= rv;
	sprintf(rv_str, "%d", res_val_read_in);
	fseek(file, i + 4, SEEK_SET);
	fwrite(rv_str, 1, 2, file);
	break;
      }
    }
  }
  
  
  fclose(file);
  fd = open_file();
  sync_mem_map_file(map, size);
}

void sync_mem_map_file(char *map, int size) {

  if (msync(map, size, MS_SYNC) == -1) {
    fprintf(stderr, "Error syncing the file");
    return;
  }
  printf("Synchronized\n");
}

void unmap_mem_map_file(char *map, int size) {

  if (munmap(map, size) == -1) {
    fprintf(stderr, "Error un-mapping the file");
  }
}


