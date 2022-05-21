#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define READ_END 0
#define WRITE_END 1
#define LINESIZE 256
#define BUFFER_SIZE (LINESIZE * 5)

int main(int argc, char *argv[])
{
  FILE* cypher;
  char* line_in = malloc(LINESIZE * sizeof(char));
  size_t read_n = 0, file_size;
  int cypher_number = 0;

  if ((cypher = fopen("cypher.txt", "r")) == NULL) {
    printf("Cannot open cypher.txt\n");
    exit(EXIT_FAILURE);
  }

  /* Get the size of the file. */
  fseek(cypher, 0L, SEEK_END);
  file_size = ftell(cypher);
  rewind(cypher);

  char** cypher1 = malloc(file_size * sizeof(char*));
  char** cypher2 = malloc(file_size * sizeof(char*));

  while (getline(&line_in, &read_n, cypher) != -1) {
    cypher1[cypher_number] = malloc(LINESIZE * sizeof(char));
    cypher2[cypher_number] = malloc(LINESIZE * sizeof(char));
    strcpy(cypher1[cypher_number], strtok(line, " "));
    strcpy(cypher2[cypher_number], strtok(NULL, " "));
    cypher2[cypher_number][strlen(cypher2[cypher_number]) - 1] = '\0'; 
    cypher_number++;
  }

  fclose(cypher);
  if (line_in)
    free(line_in);
  
  int nbytes, fd_raw[2], fd_filtered[2];
  pid_t pid;
  char line[LINESIZE];
  char* buff = (char *) malloc(BUFFER_SIZE * sizeof(char));

  if (pipe(fd_raw) < 0 || pipe(fd_filtered) < 0)
  {
    perror("pipe error");
    exit(EXIT_FAILURE);
  }

  if ((pid = fork()) < 0)
  {
    perror("fork error");
    exit(EXIT_FAILURE);
  }

  else if (pid > 0)
  {
    /* parent */

    /* send raw text */
    close(fd_raw[READ_END]);
    int n;
    while ((n = read(STDIN_FILENO, line, LINESIZE)) > 0) {
      if ((nbytes = write(fd_raw[WRITE_END], line, n)) < 0)
      {
        fprintf(stderr, "Unable to write to pipe: %s\n", strerror(errno));
      }
      if (line[n - 1] == '\n') break;
    }
    close(fd_raw[WRITE_END]);

    /* receive the filtered text */
    close(fd_filtered[WRITE_END]);
    while ((nbytes = read(fd_filtered[READ_END], line, LINESIZE)) > 0)
    {
      write(STDOUT_FILENO, line, size);
    }
    close(fd_filtered[READ_END]);

    /* wait for child and exit */
    if (waitpid(pid, NULL, 0) < 0)
    {
      fprintf(stderr, "Cannot wait for child: %s\n", strerror(errno));
    }
    exit(EXIT_SUCCESS);
  }
  else
  {
    /* child  */

    /* receive raw text */
    size_t size = 0;
    close(fd_raw[WRITE_END]);
    while ((nbytes = read(fd_raw[READ_END], line, LINESIZE)) > 0)
    {
      strcat(buff, line);
      size += nbytes;
    }
    write(STDOUT_FILENO, buff, size);
    close(fd_raw[READ_END]);

    /* send filtered text */
    close(fd_filtered[READ_END]);
    int n;
    while (1) {
      if ((nbytes = write(fd_filtered[WRITE_END], line, n)) < 0)
      {
        fprintf(stderr, "Unable to write to pipe: %s\n", strerror(errno));
      }
      if (line[n - 1] == '\n') break;
    }
    close(fd_filtered[WRITE_END]);

    /* exit gracefully */
    exit(EXIT_SUCCESS);
  }
  return 0;
}
