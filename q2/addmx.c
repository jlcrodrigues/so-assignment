#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

//Struct with informations about the matrix and the matrix itself
typedef struct matrixInfo{
    int n;
    int m;
    int size;
    int * matrix;
} matrixinfo;

//Function to create the struct with matrix from the inputted file
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
  if(str[1]!='x'){
    perror("invalid matrix");
    exit(EXIT_FAILURE);
  }
  matrixinfo.n = atoi(&str[0]);
  matrixinfo.m = atoi(&str[2]);

//Checks if size of matrix is invalid
  if(matrixinfo.n == 0 || matrixinfo.m == 0){
    perror("invalid size");
    exit(EXIT_FAILURE);
  }
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
  if(argc!= 3){
    perror("Wrong amount of matrix");
    exit(EXIT_FAILURE);
  }
  char* infile1 = argv[1];
  char* infile2 = argv[2];
  matrixinfo matrixinfo1= createMatrix(infile1);
  matrixinfo matrixinfo2= createMatrix(infile2);

//Confirms that both matrixes have the same dimensions
  if (matrixinfo1.size!=matrixinfo2.size){
      perror("Matrix with different sizes");
      exit(EXIT_FAILURE);
  }

//Memory share for the final matrix
  int *final_matrix = mmap(NULL, matrixinfo1.size*sizeof(int), PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if(final_matrix == MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }
  for(int i = 0; i < matrixinfo1.size; i++) {
      final_matrix[i] = 0;
  }

//Creates a process for each column
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

//Creates the matrix for stdout 
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
