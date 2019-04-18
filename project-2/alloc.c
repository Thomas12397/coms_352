#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
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
    scanf("%c", &resp_y_n);
  
    if (resp_y_n == 'y') {
      printf("Enter the resource number type: ");
      scanf("%d", &res_type);
      printf("Enter the number of resources needed: ");
      scanf("%d", &res_val);
      alloc_mem_map_file(map_region, res_type, res_val);
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

void alloc_mem_map_file(char *map, int rt, int rv) {

  if (rt > 9 || rt < 0 || rv > 9 || rv < 0) {
    fprintf(stderr, "There was an invalid number typed. Must be between 0 and 9\n");
  }

  char *text;
  sprintf(text, "%d", rt);
  memcpy(map, text, sizeof(char));
}

void sync_mem_map_file(char *map, int size) {

  if (msync(map, size, MS_SYNC) == -1) {
    fprintf(stderr, "Error syncing the file");
    return;
  }
  printf("Synchronized");
}

void unmap_mem_map_file(char *map, int size) {

  if (munmap(map, size) == -1) {
    fprintf(stderr, "Error un-mapping the file");
  }
}
