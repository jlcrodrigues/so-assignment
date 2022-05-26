#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "addmx.h"

struct matrixDimensions{
    int n;
    int m;
    int size;
};
matrixDimensions createMatrix (int * matrix, char* infile){
  FILE *fp;
  if((fp = fopen(infile,"r")) == NULL){
    perror("cannot open file");
    exit(EXIT_FAILURE);
  }
  size_t str_size = 0;
  char* str = NULL;
  getline(&str, &str_size, fp);
  matrixDimensions dimensions;
  dimensions.n = atoi(&str[0]);
  dimensions.m = atoi(&str[2]);
  dimensions.size = n*m;
  matrix = (int*)malloc(dimensions.size*sizeof(int));
  int i = 0, j = 0;
  while (getline(&str, &str_size, fp) > 0 ) {
    char* token = strtok(str," ");
    while( token != NULL ) {
      *(matrix+(i*dimensions.m)+j)=atoi(token);
      token=strtok(NULL," ");
      j++;
    }
    i++;
    j=0;
  }
  fclose(fp);
  return dimensions;
}

/*int readMatrix (int m, int i, int j, int value, int * partials){
    return *(partials+(i-1)*m+j);
}
*/
int main(int argc, char *argv[]) {
  
  char* infile1 = argv[1];
  char* infile2 = argv[2];
  int * matrix1;
  int * matrix2;
  matrixDimensions dimensions1 = createMatrix(matrix1,infile1);
  matrixDimensions dimensions2 = createMatrix(matrix2,infile2);
  if (dimensions1.size!=dimensions2.size){
      perror("Matrix with different sizes");
      exit(EXIT_FAILURE);
  }

  int *final_matrix = mmap(NULL, dimensions1.size*sizeof(int), PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if(final_matrix == MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }
  for(int i = 0; i < dimensions1.size; i++) {
      final_matrix[i] = 0;
  }
  for(int j = 0; j < dimensions1.m; j++) {
    pid_t pid;
    if ((pid = fork()) < 0) {
      perror("fork");
      exit(EXIT_FAILURE);
    }
    if(pid == 0) {
      for(int i = 0; i < dimensions1.n ; i++){
        *(final_matrix+(i*dimensions1.m)+j) = *(matrix1+(i*dimensions1.m)+j) + *(matrix2+(i*dimensions1.m)+j);
      }
      exit(EXIT_SUCCESS);
    }
  }
  for(i = 0; i < nprocs; i++) {
    if ( waitpid(-1, NULL, 0) < 0) {
      perror("waitpid");
      exit(EXIT_FAILURE);
    }
  }
  printf("%dx%d\n",dimensions1.n,dimensions1.m);
  for (int i=0; i < dimensions1.n;i++){
    for(int j=0; j<dimensions1.m;j++){
      printf("%d ", *(final_matrix+(i*dimensions1.m)+j);
    }
    printf("\n");
  }
  if (munmap(final_matrix, sizeof(final_matrix)) < 0) {
    perror("munmap");
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
}
