#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

//#include "hash_table.h"

#define READ_END 0
#define WRITE_END 1
#define LINESIZE 256
#define BUFFER_SIZE (LINESIZE * 5)

int main(int argc, char *argv[])
{

  int nbytes, fd_raw[2], fd_filtered[2];
  pid_t pid;
  char line[LINESIZE];

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
    int i = 0;
    char c;
    char *buff_read = (char *)malloc(BUFFER_SIZE * sizeof(char));
    c = getchar();
    while (c != EOF) {
      buff_read[i] = c;
      i++;
      c = getchar();
    }
    buff_read[i] = '\0';
    if ((nbytes = write(fd_raw[WRITE_END], buff_read, i)) < 0)
    {
      fprintf(stderr, "Unable to write to pipe: %s\n", strerror(errno));
    }
    close(fd_raw[WRITE_END]);

    /* receive the filtered text */
    close(fd_filtered[WRITE_END]);
    while ((nbytes = read(fd_filtered[READ_END], line, LINESIZE)) > 0)
    {
      write(STDOUT_FILENO, line, nbytes);
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
    char *buff = (char *)malloc(BUFFER_SIZE * sizeof(char));
    char *buff_filtered = (char *)malloc(BUFFER_SIZE * sizeof(char));
    buff[0] = '\0';
    buff_filtered[0] = '\0';

    /* receive raw text */
    int size = 0;
    close(fd_raw[WRITE_END]);
    while ((nbytes = read(fd_raw[READ_END], line, BUFFER_SIZE)) > 0)
    {
      strcat(buff, line);
      size += nbytes;
    }
    close(fd_raw[READ_END]);
    buff[size - 1] = '\0';

    /* get the cypher */
    FILE *cypher_file;
    char *line_in = (char *)malloc(LINESIZE * sizeof(char));
    size_t read_n = 0, file_size;
    int cypher_number = 0;

    if ((cypher_file = fopen("cypher.txt", "r")) == NULL)
    {
      printf("Cannot open cypher.txt\n");
      exit(EXIT_FAILURE);
    }

    /* Get the size of the file. */
    fseek(cypher_file, 0L, SEEK_END);
    file_size = ftell(cypher_file);
    rewind(cypher_file);

    char **cypher1 = (char **)malloc(file_size * sizeof(char *));
    char **cypher2 = (char **)malloc(file_size * sizeof(char *));

    while (getline(&line_in, &read_n, cypher_file) != -1)
    {
      cypher1[cypher_number] = (char *)malloc(LINESIZE * sizeof(char));
      cypher2[cypher_number] = (char *)malloc(LINESIZE * sizeof(char));
      strcpy(cypher1[cypher_number], strtok(line_in, " "));
      strcpy(cypher2[cypher_number], strtok(NULL, "\n"));
      cypher2[cypher_number][strlen(cypher2[cypher_number]) - 1] = '\0';
      cypher_number++;
    }

    fclose(cypher_file);
    if (line_in)
      free(line_in);

    /*hash_table_table * cypher = hash_table_new();

    for (int i = 0; i < cypher_number; i++)
    {
      hash_table_insert(cypher, cypher1[i], cypher2[i]);
      hash_table_insert(cypher, cypher2[i], cypher1[i]);
    }*/

    //free(cypher1);
    //free(cypher2);

    char *word = strtok(buff, " ");
    char *filter = (char *)malloc(LINESIZE * sizeof(char));
    char* punctuation = (char *)malloc(10 * sizeof(char));
    while (word != NULL)
    {
      int punc_index = 0;
      punctuation[0] = '\0';
      while (!isalpha(word[strlen(word) - 1 - punc_index])) {
        punctuation[punc_index] = word[strlen(word) - 1 - punc_index];
        ++punc_index;
      }
      punctuation[punc_index] = '\0';
      word[strlen(word) - punc_index] = '\0';

      int found = 0;
      for (int i = 0; i < cypher_number && !found; i++) {
        if (strcmp(word, cypher1[i]) == 0) {
          strcpy(filter, cypher2[i]);
          found = 1;
        }
        if (strcmp(word, cypher2[i]) == 0) {
          strcpy(filter, cypher1[i]);
          found = 1;
        }
      }

      if (found) {
        for (int i = strlen(punctuation) - 1; i >= 0; i--) 
          strncat(filter, &punctuation[i], 1);
        strcat(buff_filtered, filter);
      }
      else {
        for (int i = strlen(punctuation) - 1; i >= 0; i--) {
          strncat(word, &punctuation[i], 1);
        }
        strcat(buff_filtered, word);
      }

      strcat(buff_filtered, " ");
      word = strtok(NULL, " ");
    }
    strcat(buff_filtered, "\n");

    /* send filtered text */
    close(fd_filtered[READ_END]);
    if ((nbytes = write(fd_filtered[WRITE_END], buff_filtered, strlen(buff_filtered))) < 0)
    {
      fprintf(stderr, "Unable to write to pipe: %s\n", strerror(errno));
    }

    close(fd_filtered[WRITE_END]);

    /* exit gracefully */
    exit(EXIT_SUCCESS);
  }
  return 0;
}
