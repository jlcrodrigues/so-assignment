#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

typedef struct matrixInfo{
    int n;
    int m;
    int size;
    int * matrix;
} matrixinfo;

matrixinfo createMatrix (char* infile){
  FILE *fp;
  if((fp = fopen(infile,"r")) == NULL){
    perror("cannot open file");
    exit(EXIT_FAILURE);
  }
  size_t str_size = 0;
  char* str = NULL;
  getline(&str, &str_size, fp);
  matrixinfo matrixinfo;
  matrixinfo.n = atoi(&str[0]);
  matrixinfo.m = atoi(&str[2]);
  matrixinfo.size = matrixinfo.n*matrixinfo.m;
  matrixinfo.matrix = (int*)malloc(matrixinfo.size*sizeof(int));
  int i = 0, j = 0;
  while (getline(&str, &str_size, fp) > 0 ) {
    char* token = strtok(str," ");
    while( token != NULL ) {
      *(matrixinfo.matrix+(i*matrixinfo.m)+j)=atoi(token);
      token=strtok(NULL," ");
      j++;
    }
    i++;
    j=0;
  }
  fclose(fp);
  return matrixinfo;
}

int main(int argc, char *argv[]) {
  
  char* infile1 = argv[1];
  char* infile2 = argv[2];
  matrixinfo matrixinfo1= createMatrix(infile1);
  matrixinfo matrixinfo2= createMatrix(infile2);
  if (matrixinfo1.size!=matrixinfo2.size){
      perror("Matrix with different sizes");
      exit(EXIT_FAILURE);
  }

  int *final_matrix = mmap(NULL, matrixinfo1.size*sizeof(int), PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if(final_matrix == MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }
  for(int i = 0; i < matrixinfo1.size; i++) {
      final_matrix[i] = 0;
  }
  for(int j = 0; j < matrixinfo1.m; j++) {
    pid_t pid;
    if ((pid = fork()) < 0) {
      perror("fork");
      exit(EXIT_FAILURE);
    }
    if(pid == 0) {
      for(int i = 0; i < matrixinfo1.n ; i++){
        int a = *(matrixinfo1.matrix+(i*matrixinfo1.m)+j);
        int b = *(matrixinfo2.matrix+(i*matrixinfo2.m)+j);
        *(final_matrix+(i*matrixinfo1.m)+j) = a + b;
      }
      exit(EXIT_SUCCESS);
    }
  }
  for(int i = 0; i < matrixinfo1.m; i++) {
    if (waitpid(-1, NULL, 0) < 0) {
      perror("waitpid");
      exit(EXIT_FAILURE);
    }
  }
  printf("%dx%d\n",matrixinfo1.n,matrixinfo1.m);
  for (int i=0; i < matrixinfo1.n;i++){
    for(int j=0; j<matrixinfo1.m;j++){
      printf("%d ", *(final_matrix+(i*matrixinfo1.m)+j));
    }
    printf("\n");
  }
  if (munmap(final_matrix, sizeof(final_matrix)) < 0) {
    perror("munmap");
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
}
