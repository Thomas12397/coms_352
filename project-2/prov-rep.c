/* 
   Author: Thomas Haddy
   Date:   4/19/19
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/sem.h>

#include "prov-rep.h"
#include "alloc.h"

int sem;

int main() {

  char resp_y_n;
  int res_type, res_val;
  int filedesc, size;
  char *map_region;
  FILE *file;
  struct sembuf sem_op;

  /*Opening the file*/
  filedesc = open_file();

  /*Get the file size*/
  size = get_file_size(filedesc);

   /*Initially map the file*/
  map_region = init_mem_map_file(filedesc, size);

  /* Create the semaphore for mutual exclusion */
  sem = semget(1, 1, IPC_CREAT | 0600);
  if (sem == -1) {
    fprintf(stderr, "Semaphore was not created correctly.\n");
    return 1;
  }

  int rc;
  rc = semctl(sem, 0, SETVAL, 1);

  /* Child Process */
  if(fork() == 0) {
    while(1) {

      usleep(10000000);
      
      printf("\nReport:\n");
      printf("Page size is: %d\n\n", getpagesize());

      printf("Current state of resources:\n\n");

      /* Semaphore waits */
      sem_op.sem_num = 0;
      sem_op.sem_op = -1;   
      sem_op.sem_flg = 0;
      semop(sem, &sem_op, 1);
      
      file = fdopen(filedesc, "r");
      int ch;
      if (file) {
	while ((ch = fgetc(file)) != EOF) {
	  printf("%c", ch);
	}
	fclose(file);

	/* Semaphore signal */
	sem_op.sem_num = 0;
	sem_op.sem_op = 1;   
	sem_op.sem_flg = 0;
	semop(sem, &sem_op, 1);
	
	filedesc = open_file();
      }
      
      unsigned char *vec;
      vec = calloc(1, (size + getpagesize() - 1)/getpagesize());
      mincore(map_region, size, vec);
      
      printf("Cached blocks: \n");
      for (size_t i = 0; i <= size / getpagesize(); ++i){
        if (vec[i] & 1){
	  printf("Page %ld: Resident\n", i);
	}
      }
      
      fputc('\n', stdout);
      free(vec); 
      fclose(file);

      /* Semaphore signal */
      sem_op.sem_num = 0;
      sem_op.sem_op = 1;   
      sem_op.sem_flg = 0;
      semop(sem, &sem_op, 1);
      
      fflush(stdout);
    }
  }
  /* Parent Process */
  else {
    do {
      printf("Allocate more resources(y/n)? ");
      scanf(" %c", &resp_y_n);
      
      if (resp_y_n == 'y') {
	printf("Enter the resource number type: ");
	scanf("%d", &res_type);
	printf("Enter the number of resources needed: ");
	scanf("%d", &res_val);
	new_alloc_mem_map_file(map_region, size, filedesc, res_type, res_val);
      }
      
    } while (resp_y_n != 'n');
    
    /* Deallocate everything */
    unmap_mem_map_file(map_region, size);
    close(filedesc);
  }
 
  return 0;
}

/* Opens the input file.
   
   Return: returns the file descriptor of the input file

 */
int open_file() {

  int fd;
  fd = open("res.txt", O_RDWR);
  if (fd == -1) {
    fprintf(stderr, "Could not open \"res.txt\"\n");
    return -1;
  }
  return fd;
}

/* Gets the size of the input file.
   int fd: The file descriptor of the input file
   
   Return: returns the size of the input file
 */
int get_file_size(int fd) {

  if (fd == -1) {
    fprintf(stderr, "\"res.txt\" is not open. Couldn't get size\n");
    return -1;
  }
  struct stat buf;
  fstat(fd, &buf);
  return buf.st_size;
}

/* Initializes the memory mapped file.
   int fd: The file descriptor of the input file
   int size: The size of the input file

   Return: returns the memory mapped file as string
 */
char* init_mem_map_file(int fd, int size) {

  char *map = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (map == MAP_FAILED) {
    close(fd);
    fprintf(stderr, "Error mapping the file");
    return NULL;
  }

  return map;
}

/* Simulates adding resources to the text file. The reosurce type's value is updated.
   char *map: The memory mapped file
   int size: The size of the input file
   int fd: The file descriptor of the input file
   int rt: The resource type passed in from the user
   int rv: The resource value passed in from the user
 */
void new_alloc_mem_map_file(char *map, int size, int fd, int rt, int rv) {

  char rv_str[2];
  struct sembuf sem_op;

  if (rt > 9 || rt < 0 || rv > 9 || rv < 1) {
    fprintf(stderr, "There was an invalid number typed. Must be between 0 and 9\n");
    return;
  }

  /* Semaphore waits */
  sem_op.sem_num = 0;
  sem_op.sem_op = -1;   
  sem_op.sem_flg = 0;
  semop(sem, &sem_op, 1);
  
  FILE *file = fdopen(fd, "r+");

  int i;
  int res_type_read_in, res_val_read_in;
  for (i = 2; i < size; i = i + 8) {
    fseek(file, i, SEEK_SET);
    fscanf(file, "%d", &res_type_read_in);
    if (res_type_read_in == rt) {
      fseek(file, i + 4, SEEK_SET);
      fscanf(file, "%d", &res_val_read_in);
      if (res_val_read_in + rv < 10) {
	res_val_read_in += rv;
	sprintf(rv_str, "%d", res_val_read_in);
	fseek(file, i + 4, SEEK_SET);
	fwrite(rv_str, 1, 2, file);
	break;
      }
    }
  }

  fclose(file);

  /* Semaphore signal */
  sem_op.sem_num = 0;
  sem_op.sem_op = 1;   
  sem_op.sem_flg = 0;
  semop(sem, &sem_op, 1);
  
  fd = open_file();
  sync_mem_map_file(map, size);
}

/* Synchronizes the memory mapped file with the physical file on the disk
   char *map: The memory mapped file
   int size: The size of the input file
 */
void sync_mem_map_file(char *map, int size) {

  if (msync(map, size, MS_SYNC) == -1) {
    fprintf(stderr, "Error syncing the file");
    return;
  }
  printf("Synced successfully.\n");
}

/* Unmaps the memory mapped file.
   char *map: The memory mapped file
   int size: The size of the input file
 */
void unmap_mem_map_file(char *map, int size) {

  if (munmap(map, size) == -1) {
    fprintf(stderr, "Error un-mapping the file");
  }
}
