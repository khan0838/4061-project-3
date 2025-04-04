#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define ALPHABET_LEN 26
/*
 * Counts the number of occurrences of each letter (case insensitive) in a text
 * file and stores the results in an array.
 * file_name: The name of the text file in which to count letter occurrences
 * counts: An array of integers storing the number of occurrences of each letter.
 *     counts[0] is the number of 'a' or 'A' characters, counts [1] is the number
 *     of 'b' or 'B' characters, and so on.
 * Returns 0 on success or -1 on error.
 */
int count_letters(const char *file_name, int *counts) {

    FILE *file_read = fopen(file_name, "r");
    char current;
    if(file_read == NULL){
        perror("fopen");
        return -1;
    }
    while((current = fgetc(file_read)) != EOF){
        if(isalpha(current)){
            if(isupper(current)){
                current = tolower(current);
            }
            counts[current - 'a']++;
        }

    }
    fclose(file_read);
    return 0;
}

/*
 * Processes a particular file(counting occurrences of each letter)
 *     and writes the results to a file descriptor.
 * This function should be called in child processes.
 * file_name: The name of the file to analyze.
 * out_fd: The file descriptor to which results are written
 * Returns 0 on success or -1 on error
 */
int process_file(const char *file_name, int out_fd) {
    int letter_count[ALPHABET_LEN];
    for(int i = 0; i<ALPHABET_LEN; i++){
        letter_count[i] = 0;
    }

    if(count_letters(file_name, letter_count) == -1){
        return -1;

    }
    if(write(out_fd, letter_count, sizeof(int)*ALPHABET_LEN) == -1){
        perror("write to pipe");
        return -1;

    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 1) {
        // No files to consume, return immediately
        return 0;
    }

    // TODO Create a pipe for child processes to write their results
    // TODO Fork a child to analyze each specified file (names are argv[1], argv[2], ...)
    // TODO Aggregate all the results together by reading from the pipe in the parent

    // TODO Change this code to print out the total count of each letter (case insensitive)
    for (int i = 0; i < ALPHABET_LEN; i++) {
        printf("%c Count: %d\n", 'a' + i, -1);
    }
    return 0;
}
