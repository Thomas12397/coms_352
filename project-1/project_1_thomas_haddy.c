/* 
   Author: Thomas Haddy
   Date: 3/15/19
   
   This file contains all of the functions necessary to
   complete shearsort on a file called "input.txt".
   The "input.txt" will contain a n x n 2D integer array
   formatted like:
   1 2 3
   4 5 6
   7 8 9
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <pthread.h>

typedef struct {
  int **arr;
  int length;
  int threadNum;
} args_t;

int getSize(char *fName);
void readFile(char *fName, int **arr);
void print2DArr(int **arr);
void **allocate2DArr();
void *shearesort(void *ptr);
void readInArgs(void *ptr);
void swap(int *x, int *y);
void sortCol(int **arr, int col);
void sortRow(int **arr, int row);
int figureOutHowToSort(int row);
void startNext(int **arr, pthread_mutex_t mutex);

/*
  Counter made to print the 2D array
 */
int readyToPrintArray;
pthread_mutex_t global_mutex;
pthread_cond_t next;
/* 
   size of the 2D array 
*/
int n;

/*
  The main function will conduct shearsort on a 2D array
  Then free the allocated memory
 */
int main(void) {

  int i;
  n = getSize("input.txt");
  pthread_mutex_init(&global_mutex, NULL);
  pthread_cond_init(&next, NULL);
  int **arr = (int **) allocate2DArr();
  readFile("input.txt", arr);
  print2DArr(arr);

  args_t *args;
  readyToPrintArray = 0;
  pthread_t *threads = (pthread_t *) malloc(n * sizeof(pthread_t));
  if (threads == NULL) {
    fprintf(stderr, "Out of memory\n");
    return -1;
  }
  
  for(i = 0; i < n; i++) {
    args = (args_t *) malloc(sizeof(args_t));
    args->arr = arr;
    args->length = n;
    args->threadNum = i;
    /* Calls shearsort on the threads */
    pthread_create(&threads[i], NULL, shearesort, (void *) args);
  }

  /* Waits for all of the threads to exit */
  for(i = 0; i < n; i++) {
    pthread_join(threads[i], NULL);
  }
  
  /* Cleaning up allocation */
  for(i = 0; i < n; i++) {
    free(arr[i]);
  }
  free(arr);
  pthread_mutex_destroy(&global_mutex);
  pthread_cond_destroy(&next);
}

/*
  Gets the size by counting the number of '\n' in the "input.txt".
  
  char *fName: the given file name of the 2D array.
  
  3 x 3 array will return 3
  6 x 6 array will return 6
 */
int getSize(char *fName) {

  int size = 0;
  FILE *file = fopen(fName, "r");
  if (file == NULL) {
    fprintf(stderr, "File not found from getSize()\n");
    return -1;
  }

  char ch;
  while(fread(&ch, sizeof(ch), 1, file)) {
    if (ch == '\n') {
      size++;
    }
  }
  fclose(file);
   
  return size;
}

/*
  Reads from the "input.txt" file and puts the integer
  values into an allocated 2D array.
  
  char *fName: the given  file name of the 2D array.
  int **arr: the 2D array.
 */
void readFile(char *fName, int **arr) {
  
  FILE *file = fopen(fName, "r");
  if (file == NULL) {
    fprintf(stderr, "File not found from readFromFile()\n");
    return;
  }
  
  int row;
  int col;
  for(row = 0; row < n; row++) {
    for(col = 0; col < n; col++) {
      fscanf(file, "%d", &(arr[row][col]));
    }
  }
  fclose(file);
}

/*
  Prints a 'n' x 'n' array to the terminal
  
  int **arr: the 2D array
 */
void print2DArr(int **arr) {

  int row;
  int col;
  for(row = 0; row < n; row++) {
    for(col = 0; col < n; col++) {
      printf("%d ", arr[row][col]);
    }
    printf("\n");
  }
  printf("\n");
}

/*
  Allocates memory to a dynamic 2D array
*/
void **allocate2DArr() {

  int i;
  void **arr = malloc(n * sizeof(int *));
  if (arr == NULL) {
    fprintf(stderr, "Out of memory\n");
    return NULL;
  }
  
  for(i = 0; i < n; i++) {
    arr[i] = malloc(n * sizeof(int));
    if (arr[i] == NULL) {
      fprintf(stderr, "Out of memory\n");
      return NULL;
    }
  }
  return arr;
}

/*
  Starts shearsort by getting the numbber of phases. 
  Then it bubblesorts the rows
  on the even phases and bubblesorts the columns on the odd phases.
  
  void *ptr: Void pointer used for getting the shearsort arguments
 */
void *shearesort(void *ptr) {

  int i;
  pthread_mutex_t mutex;
  pthread_mutex_init(&mutex, NULL);
  
  /* Read in all info from the void pointer and then free it */
  int **arr = ((args_t *) ptr) -> arr; 
  int length = ((args_t *) ptr) -> length;
  int threadNum = ((args_t *) ptr) -> threadNum;
  free(ptr);
  int numPhases = ceil(2 * (log10((double) length) / log10(2.0))) + 1;

  for(i = 0; i < numPhases; i++) {
    /* Odd Phases --> sort columns */
    if (i % 2 == 1) {
      sortCol(arr, threadNum);
    }
    /* Even Phases --> sort rows */
    else {
      sortRow(arr, threadNum);
    }
    /* Start the next phase */
    startNext(arr, mutex);
  }

  /* Free mutex */
  pthread_mutex_destroy(&mutex);
}

/*
  Sorts a column of the 2D array. It normal sorts it no matter if it's even or odd

  int **arr: the 2D array
  int col: the column to be sorted
 */
void sortCol(int **arr, int col) {

  int i;
  int j;
  for(i = 0; i < n - 1; i++) {
    for(j = 0; j < n - i -1; j++) {
      if(arr[j][col] > arr[j+1][col]) {
        swap(&arr[j][col], &arr[j+1][col]);
      }
    }
  }
}

/*
  Sorts a row of the 2D array.
  if the row is odd, it reverse sorts it.
  else the row is even, so it normal sorts it
  
  int **arr: the 2D array
  int row: the row to sort
 */
void sortRow(int **arr, int row) {

  int i;
  int j;
  int comparator;

  comparator = figureOutHowToSort(row);

  for(i = 0; i < n - 1; i++) {
    for(j = 0; j < n - i - 1; j++) {
      if(comparator * arr[row][j] > comparator * arr[row][j+1]) {
        swap(&arr[row][j], &arr[row][j+1]);
      }
    }
  }
}

/*
  Checks to see how to sort the row. If it's an odd row, reverse sort
  Otherwise it's even, normal sort it
 */
int figureOutHowToSort(int row) {
  /* Odd --> true --> -1 --> Reverse sort*/
  if (row % 2 == 1) {
    return -1;
  }
  /* Even --> false --> 1 --> Normal sort*/
  else {
    return 1;
  }
}

/* 
   Swaps x and y

   int *x: the first element
   int *y: the second element
*/
void swap(int *x, int *y) {

  int temp = *x;
  *x = *y;
  *y = temp;
}

/*
  Starts the next phase, whether it is sorting a new row or column

  int **arr: the 2D array
  pthread_mutex_t mutex: the given mutex for a pthread
 */
void startNext(int **arr, pthread_mutex_t mutex) {

  /* Announce that threads are finished */
  pthread_mutex_lock(&mutex);
  readyToPrintArray++;
  if(readyToPrintArray == n) {
    print2DArr(arr);
    readyToPrintArray = 0;
  }
  pthread_mutex_unlock(&mutex);

  /* Wait for all of the threads to finish, then move on */
  pthread_mutex_lock(&mutex);
  if(readyToPrintArray == 0) {
    pthread_cond_broadcast(&next);
    pthread_mutex_unlock(&mutex);
  }
  else {
    pthread_cond_wait(&next, &mutex);
  }
}
