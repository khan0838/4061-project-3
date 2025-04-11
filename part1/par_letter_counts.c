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
    if(counts == NULL){
        fprintf(stderr, "counts array is NULL\n");
        fclose(file_read);
        return -1;
    }
    //read every indavidual character in the file
    while((current = fgetc(file_read)) != EOF){
        if(isalpha(current)){
            if(isupper(current)){
                current = tolower(current);
            }
            //add the letter into the current array in the right index
            counts[current - 'a']++;
        }

    }
    if(fclose(file_read) != 0){
        perror("something went wrong with closing file.\n");
        return -1;
    }
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
    memset(letter_count, 0, sizeof(letter_count));

    if(count_letters(file_name, letter_count) == -1){
        return -1;

    }
    ssize_t bytes_written = write(out_fd, letter_count, sizeof(letter_count));
    if (bytes_written == -1 || bytes_written != sizeof(letter_count)) {
        perror("write");
        return -1;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 1) {
        // No files to consume, return immediately
        return 0;
    }
    int pipe_fds[2];
    if (pipe(pipe_fds) == -1) {
        perror("pipe");
        return -1;
    }
    //forking argc-1 children (one for each file)
    for(int i = 1; i<argc; i++){
        pid_t child_pid = fork();
        if(child_pid == -1){
            perror("fork");
            close(pipe_fds[0]);
            close(pipe_fds[1]);
            return 1;
        }
        if(child_pid == 0){
            if (close(pipe_fds[0]) == -1) {
                perror("close");
                close(pipe_fds[0]);
                exit(1);
            }

            if(process_file(argv[i], pipe_fds[1]) == -1){
                exit(1);
            }
            if (close(pipe_fds[1]) == -1) {
                perror("close");
                exit(1);
            }
            exit(0);

        }
    }
    //first wait for all children to finish
    for(int i = 0; i<argc; i++){
        wait(NULL);
    }
    if (close(pipe_fds[1]) == -1) {
        perror("close");
        close(pipe_fds[0]);
        return 1;
    }

    int bytes_read;
    int total_counts[ALPHABET_LEN] = {0};
    int temp[ALPHABET_LEN];

    //reads each file input by reasding a list of ALPABET_LEN length
    while ((bytes_read = read(pipe_fds[0], temp, sizeof(temp))) > 0) {
        //if it does not read the a list of ALPHABET_LEN ints something went wrong
        if(bytes_read != sizeof(temp) || bytes_read == -1){
            perror("reading from pipe");
            return 1;
        }
        for(int j = 0; j<ALPHABET_LEN; j++){
            total_counts[j] += temp[j];
        }
    }
    //close the read pipe after finishing reading each array
    if (close(pipe_fds[0]) == -1) {
        perror("close");
        return 1;
    }


    for (int i = 0; i < ALPHABET_LEN; i++) {
        printf("%c Count: %d\n", 'a' + i, total_counts[i]);
    }
    return 0;
}
