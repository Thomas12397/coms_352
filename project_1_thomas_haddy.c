/* Author: Thomas Haddy */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <pthread.h>

typedef struct {
  int **arr;
  int threadNum;
  int size;
} shearsort_args_t;

void readFromFile(char *fileName, int **arr, int size);
void printArray(int **arr, int size);
void **create2DArray(int row, int column);
void free2DArray(void **arr, int size);
void startShearesort(int **arr, int size);
void *shearesort(void *ptr);
void swap(int *first, int *second);
void bubbleSortRow(int **arr, int size, int row);
void bubbleSortColumn(int **arr, int size, int column);
void startNextPhase(int **arr, int size, pthread_mutex_t my_mutex);

/* Don't know why this has to be global but ... */
pthread_mutex_t mutex;
pthread_cond_t nextPhase;
unsigned numReady;

int main(void) {
  
  int i, j;
  int size = 4;
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&nextPhase, NULL);
  int **arr = (int **) create2DArray(size, size);
  readFromFile("input.txt", arr, size);
  printArray(arr, size);
  startShearesort(arr, size);

  /* end of program */
  free2DArray((void **) arr, size);
  pthread_cond_destroy(&nextPhase);
  pthread_mutex_destroy(&mutex);
}

/* Reads an entire file and files the given array with the data
  fileName: string containing name of file to open and read
  arr: size x size array that will hold given data
  size: size of given array
*/
void readFromFile(char *fileName, int **arr, int size) {
  int i, j;
  FILE *fp = fopen(fileName, "r");
  for(i = 0; i < size; i++) {
    for(j = 0; j < size; j++) {
      fscanf(fp, "%d", &(arr[i][j]));
    }
  }
}

/*
  prints the given array
  arr: array to print
  size: size of array
 */
void printArray(int **arr, int size) {
  int i, j;
  for(i = 0; i < size; i++) {
    for(j = 0; j < size; j++) {
      printf("%d ", arr[i][j]);
    }
    printf("\n");
  }
  printf("\n");
}

/* creates 2D array
  row: number of rows
  column: number of columns
*/
void **create2DArray(int row, int column) {
  int i;
  void **arr = malloc(row * sizeof(int *));
  for(i = 0; i < row; i++) {
    arr[i] = malloc(column * sizeof(int));
  }
  return arr;
}

/*frees the given 2D array
  size: number of rows
  arr: name of array to free
*/
void free2DArray(void **arr, int size) {
  int i;
  for(i = 0; i < size; i++){
    free(arr[i]);
  }
  free(arr);
}

/* Creates the threads and has them start on the shearesort algorithm */
void startShearesort(int **arr, int size) {
  int i;
  shearsort_args_t *args;
  numReady = 0;
  pthread_t *threads = (pthread_t *) malloc(size * sizeof(pthread_t));
  for(i = 0; i < size; i++) {
    /* shearesort function will free this structure */
    args = (shearsort_args_t *) malloc(sizeof(shearsort_args_t));
    args->arr = arr;
    args->size = size;
    args->threadNum = i;
    pthread_create(&threads[i], NULL, shearesort, (void *) args);
  }

  /* Wait for all the threads to exit */
  for(i = 0; i < size; i++) {
    pthread_join(threads[i], NULL);
  }
}

void *shearesort(void *ptr) {
  int i;
  pthread_mutex_t my_mutex;
  pthread_mutex_init(&my_mutex, NULL);

  /* Read in all info from the pointer and then free it */
  int threadNum = ((shearsort_args_t *) ptr) -> threadNum;
  int size = ((shearsort_args_t *) ptr) -> size;
  int **arr = ((shearsort_args_t *) ptr) -> arr;
  free(ptr);

  /* Equation given in assignment */
  int steps = ceil(2 * (log10((double) size) / log10(2.0))) + 1;
  for(i = 0; i < steps; i++) {
    if(i % 2 == 0) { /* Even Phases */
      bubbleSortRow(arr, size, threadNum);
    }
    else { /* Odd Phases */
      bubbleSortColumn(arr, size, threadNum);
    }
    startNextPhase(arr, size, my_mutex);
  }
  pthread_mutex_destroy(&my_mutex);
}

void bubbleSortRow(int **arr, int size, int row) {
  int i, j, mult;
  mult = (row % 2 == 1 ? -1 : 1);

  for(i = 0; i < size - 1; i++) {
    for(j = 0; j < size - i - 1; j++) {
      if(mult * arr[row][j] > mult * arr[row][j+1]) {
        swap(&arr[row][j], &arr[row][j+1]);
      }
    }
  }
}

void bubbleSortColumn(int **arr, int size, int column) {
  int i, j;
  for(i = 0; i < size - 1; i++) {
    for(j = 0; j < size - i -1; j++) {
      if(arr[j][column] > arr[j+1][column]) {
        swap(&arr[j][column], &arr[j+1][column]);
      }
    }
  }
}

/* Swaps the integer values of first and second */
void swap(int *first, int *second) {
  int temp = *first;
  *first = *second;
  *second = temp;
}

void startNextPhase(int **arr, int size, pthread_mutex_t my_mutex) {
  /* Announce that you're finished */
  pthread_mutex_lock(&mutex);
  numReady++;
  if(numReady == size) {
    printArray(arr, size);
    numReady = 0;
  }
  pthread_mutex_unlock(&mutex);

  /* Wait for all threads to finish there work, then move on */
  pthread_mutex_lock(&my_mutex);
  if(numReady == 0) {
    pthread_cond_broadcast(&nextPhase);
    pthread_mutex_unlock(&my_mutex);
  }
  else {
    pthread_cond_wait(&nextPhase, &my_mutex);
  }
}
