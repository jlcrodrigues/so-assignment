#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc > 3) return -1;
    if (argc < 2) {
        printf("\nusage: phrases [-l] file\n");
        return EXIT_FAILURE;
    }

    FILE* file;
    
    int txt = 1, l = 0;  // l is if '-l' option is used

    if (strcmp(argv[1], "-l") == 0) {
        l = 1;
        txt = 2;
    }


    file = fopen(argv[txt], "r");
    if (file == NULL) {
        fprintf(stderr, "\nFailed to open file\n");
        exit(EXIT_FAILURE); 
    }

    /* Handle empty files */
    int size;
    fseek (file, 0, SEEK_END);
    size = ftell(file);
    if (0 == size) {
        printf("File is empty!\n");
        return EXIT_FAILURE;
    }
    rewind(file);


    char buffer[BUFFER_SIZE];
    char c;
    int i = 0, line = 0;


    while((c = fgetc(file)) != EOF) {
        buffer[i] = c;
        i++;

        if (c == '.' || c == '?' || c == '!') {
            line++;
            buffer[i] = '\0';
            if (l) {
                printf("[%i]", line);
                printf("%s\n", buffer);
            };
            i = 0;
        }
    }

    if (i != 0) {  // if last phrase doesn't end with punctuation
        line++;
        buffer[i--] = '\0';  // 'i--' because i has to be behind end of file
        if (l) {
            printf("[%i]", line);
            printf("%s\n", buffer);
        };
    }

    if (!l) printf("%i\n", line);

    return EXIT_SUCCESS;
}