/* 
   Author: Thomas Haddy
   Date: 3/14/19
   
   This file contains all of the functions necessary to
   complete shearsort on a file called "input.txt".
   The "input.txt" can contain a n x n 2D integer array
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
  int size;
  int threadNum;
} shearsort_args_t;

int getSize(char *fileName);
void readFile(char *fileName, int **arr, int size);
void print2DArr(int **arr, int size);
void **allocate2DArr(int row, int col);
void startShearesortThreads(int **arr, int size);
void *shearesort(void *ptr);
void swap(int *x, int *y);
void bubbleSortRow(int **arr, int size, int row);
void bubbleSortCol(int **arr, int size, int col);
void startNextPhase(int **arr, int size, pthread_mutex_t my_mutex);
void free2DArr(void **arr, int size);

pthread_mutex_t mutex;
pthread_cond_t nextPhase;
unsigned numReady;

int main(void) {
  
  int size = getSize("input.txt");
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&nextPhase, NULL);
  int **arr = (int **) allocate2DArr(size, size);
  readFile("input.txt", arr, size);
  print2DArr(arr, size);
  startShearesortThreads(arr, size);
  
  /* Cleaning up allocation */
  free2DArr((void **) arr, size);
  pthread_cond_destroy(&nextPhase);
  pthread_mutex_destroy(&mutex);
}

/*
  Gets the size by counting the number of '\n' in the "input.txt".
  
  char *filename is the given file name of the 2D array.
  
  3 x 3 array will return 3
  6 x 6 array will return 6
 */
int getSize(char *fileName) {

  int size = 0;
  FILE *file = fopen(fileName, "r");
  if (file == NULL) {
    printf("File not found from getSize()\n");
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
  
  char *fileName: the given  file name of the 2D array.
  int **arr: the 2D array.
  int size: the size of the 2D array 
 */
void readFile(char *fileName, int **arr, int size) {
  
  FILE *file = fopen(fileName, "r");
  if (file == NULL) {
    printf("File not found from readFromFile()\n");
  }
  
  int i, j;
  for(i = 0; i < size; i++) {
    for(j = 0; j < size; j++) {
      fscanf(file, "%d", &(arr[i][j]));
    }
  }
  fclose(file);
}

/*
  Prints a 'size' x 'size' array to the terminal
  
  int **arr: the 2D array
  int size: the size of the 2D array
 */
void print2DArr(int **arr, int size) {
  int i, j;
  for(i = 0; i < size; i++) {
    for(j = 0; j < size; j++) {
      printf("%d ", arr[i][j]);
    }
    printf("\n");
  }
  printf("\n");
}

/*
  Allocates memory to a dynamic 2D array
  
  int row: number of rows
  int col: number of columns
*/
void **allocate2DArr(int row, int col) {
  int i;
  void **arr = malloc(row * sizeof(int *));
  if (arr == NULL) {
    fprintf(stderr, "Out of memory\n");
    return NULL;
  }
  
  for(i = 0; i < row; i++) {
    arr[i] = malloc(col * sizeof(int));
    if (arr[i] == NULL) {
      fprintf(stderr, "Out of memory\n");
      return NULL;
    }
  }
  return arr;
}

/* 
   Creates the threads needed to complete shearsort and calls shearsort
   inside pthread_create()
   
   int **arr: the 2D array
   int size: the size of the 2D array
*/
void startShearesortThreads(int **arr, int size) {
  int i;
  shearsort_args_t *shearsort_args;
  numReady = 0;
  pthread_t *threads = (pthread_t *) malloc(size * sizeof(pthread_t));
  
  for(i = 0; i < size; i++) {
    shearsort_args = (shearsort_args_t *) malloc(sizeof(shearsort_args_t));
    shearsort_args->arr = arr;
    shearsort_args->size = size;
    shearsort_args->threadNum = i;
    pthread_create(&threads[i], NULL, shearesort, (void *) shearsort_args);
  }

  /* Waits for all of the threads to exit */
  for(i = 0; i < size; i++) {
    pthread_join(threads[i], NULL);
  }
}

/*
  Starts shearsort by getting the numbber of phases. 
  Then it bubblesorts the rows
  on the even phases and bubblesorts the columns on the odd phases.
  
  void *ptr: Void pointer used for getting the shearsort arguments
 */
void *shearesort(void *ptr) {
  int i;
  pthread_mutex_t my_mutex;
  pthread_mutex_init(&my_mutex, NULL);

  /* Read in all info from the pointer and then free it */
  int **arr = ((shearsort_args_t *) ptr) -> arr; 
  int size = ((shearsort_args_t *) ptr) -> size;
  int threadNum = ((shearsort_args_t *) ptr) -> threadNum;
  free(ptr);

  int numPhases = ceil(2 * (log10((double) size) / log10(2.0))) + 1;
  for(i = 0; i < numPhases; i++) {
    /* Even Phases --> sort rows */
    if(i % 2 == 0) {
      bubbleSortRow(arr, size, threadNum);
    }
    /* Odd Phases --> sort columns */
    else {
      bubbleSortCol(arr, size, threadNum);
    }
    /* Start the next phase */
    startNextPhase(arr, size, my_mutex);
  }

  /* Free my_mutex */
  pthread_mutex_destroy(&my_mutex);
}

/*
  Sorts a row of the 2D array.
  if the row is odd, it reverse sorts it.
  else the row is even, so it normal sorts it
  
  int **arr: the 2D array
  int size: the size of the 2D array
  int row: the row to sort
 */
void bubbleSortRow(int **arr, int size, int row) {
  int i, j, comparator;
  
  /* Odd --> true --> -1 --> Reverse sort*/
  /* Even --> false --> 1 --> Normal sort*/
  comparator = (row % 2 == 1 ? -1 : 1);

  for(i = 0; i < size - 1; i++) {
    for(j = 0; j < size - i - 1; j++) {
      if(comparator * arr[row][j] > comparator * arr[row][j+1]) {
        swap(&arr[row][j], &arr[row][j+1]);
      }
    }
  }
}

/*
  Sorts a column of the 2D array. It normal sorts it no matter if it's even or odd

  int **arr: the 2D array
  int size: the size of the 2D array
  int col: the column to be sorted
 */
void bubbleSortCol(int **arr, int size, int col) {
  int i, j;
  for(i = 0; i < size - 1; i++) {
    for(j = 0; j < size - i -1; j++) {
      if(arr[j][col] > arr[j+1][col]) {
        swap(&arr[j][col], &arr[j+1][col]);
      }
    }
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
  int size: the size of the 2D array
  pthread_mutex_t my_mutex: the given mutex for a pthread
 */
void startNextPhase(int **arr, int size, pthread_mutex_t my_mutex) {

  /* Announce that you're finished */
  pthread_mutex_lock(&mutex);
  numReady++;
  if(numReady == size) {
    print2DArr(arr, size);
    numReady = 0;
  }
  pthread_mutex_unlock(&mutex);

  /* Wait for all threads to finish their work, then move on */
  pthread_mutex_lock(&my_mutex);
  if(numReady == 0) {
    pthread_cond_broadcast(&nextPhase);
    pthread_mutex_unlock(&my_mutex);
  }
  else {
    pthread_cond_wait(&nextPhase, &my_mutex);
  }
}

/*
  Frees the allocation space of the given 'size' x 'size' arr

  int **arr: the 2D array
  int size: the sizze of the 2D array
*/
void free2DArr(void **arr, int size) {
  int i;
  for(i = 0; i < size; i++) {
    free(arr[i]);
  }
  free(arr);
}
