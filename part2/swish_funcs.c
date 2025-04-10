#include "swish_funcs.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "string_vector.h"

#define MAX_ARGS 10

/*
 * Helper function to run a single command within a pipeline. You should make
 * make use of the provided 'run_command' function here.
 * tokens: String vector containing the tokens representing the command to be
 * executed, possible redirection, and the command's arguments.
 * pipes: An array of pipe file descriptors.
 * n_pipes: Length of the 'pipes' array
 * in_idx: Index of the file descriptor in the array from which the program
 *         should read its input, or -1 if input should not be read from a pipe.
 * out_idx: Index of the file descriptor in the array to which the program
 *          should write its output, or -1 if output should not be written to
 *          a pipe.
 * Returns 0 on success or -1 on error.
 */
int run_piped_command(strvec_t *tokens, int *pipes, int n_pipes, int in_idx, int out_idx) {
    // TODO Complete this function's implementation
    return 0;
}

int run_pipelined_commands(strvec_t *tokens) {
    // TODO Complete this function's implementation

    int n = strvec_num_occurrences(tokens, "|");
    int *pipe_fds = malloc(2 * n * sizeof(int));
    if (pipe_fds == NULL) {
        fprintf(stderr, "malloc failed\n");
        return -1;
    }

    // Set up all pipes
    for (int i = 0; i < n; i++) {
        if (pipe(pipe_fds + 2 * i) == -1) {
            perror("pipe");
            for (int j = 0; j < i; j++) {
                close(pipe_fds[2 * j]);
                close(pipe_fds[2 * j + 1]);
            }
            free(pipe_fds);
            return -1;
        }
    }
    int end_index = 0;
    int start_index;

    for (int i = 0; i < n; i++) {
        //finding the end of a pipe command
        start_index = end_index;
        if(strvec_find(tokens, "|") != -1){
            end_index = strvec_find(tokens, "|");
        }
        strvec_t *new_tok;
        strvec_slice(tokens, new_tok, start_index, end_index);
        pid_t child_pid = fork();
        if (child_pid == -1) {
            perror("fork");
            for (int j = 0; j < n; j++) {
                close(pipe_fds[2 * j]);
                close(pipe_fds[2 * j + 1]);
            }
            free(pipe_fds);
            return -1;
        } else if (child_pid == 0) {
            // Close pipes for all other children
            // And close read end of own pipe
            int in_index;
            int out_index;
            if(i == 0){
                in_index = -1;

            }
            else{
                in_index = 2 * (i-1);
            }
            if(i = n-1){
                out_index = -1;
            }
            else{
                out_index = 2 * (i+1);
            }
            run_piped_command(new_tok, pipe_fds, n, in_index, out_index);
            for (int j = 0; j < n; j++) {
                if (close(pipe_fds[2 * j]) == -1) {
                    perror("close");
                }
                if (j != i && close(pipe_fds[2 * j + 1]) == -1) {
                    perror("close");
                }
            }
        }
    }

    return 0;
}
