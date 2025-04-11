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
    if (in_idx != -1) {
        if (dup2(pipes[in_idx], STDIN_FILENO) == -1) {
            perror("dup2 for STDIN");
            exit(1);
        }
    }
    if (out_idx != -1) {
        if (dup2(pipes[out_idx], STDOUT_FILENO) == -1) {
            perror("dup2 for STDOUT");
            exit(1);
        }
    }
    for (int i = 0; i < 2 * n_pipes; i++) {
        if (close(pipes[i]) == -1) {
            perror("close failure");
        }
    }

    run_command(tokens);
    perror("run_command failed");
    exit(1);

}

int run_pipelined_commands(strvec_t *tokens) {
    int n = strvec_num_occurrences(tokens, "|");
    int *pipe_fds = malloc(2 * n * sizeof(int));
    if (pipe_fds == NULL) {
        perror("malloc");
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

    for (int i = n; i >= 0; i--) {
        //finding the end of a pipe command
        int in_idx;
        int out_idx;
        if(i == 0){
            in_idx = -1;
        } else {
            in_idx = 2 * (i - 1);
        }

        if (i == n) {
            out_idx = -1;
        } else {
            out_idx = 2 * i + 1;
        }

        int pipe_pos = strvec_find_last(tokens, "|");
        int start_idx;
        if (pipe_pos == -1) {
            start_idx = 0;
        } else {
            start_idx = pipe_pos + 1;
        }

        strvec_t cmd;
        if (strvec_slice(tokens, &cmd, start_idx, tokens->length) == -1) {
            perror("strvec_slice");
            free(pipe_fds);
            return -1;
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            strvec_clear(&cmd);
            free(pipe_fds);
            return -1;
        } else if (pid == 0) {
            run_piped_command(&cmd, pipe_fds, n, in_idx, out_idx);
        }

        strvec_take(tokens, pipe_pos);
        strvec_clear(&cmd);
    }

    for (int i = 0; i < 2 * n; i++) {
        if (close(pipe_fds[i]) == -1) {
            perror("close failure");
        }
    }
    free(pipe_fds);


    int status;
    int failure = 0;
    for (int i = 0; i <= n; i++) {
        if (wait(&status) == -1) {
            perror("wait");
            failure = 1;
        } else {
            if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            if (code != 0) {
                // Child exited with non-zero status (like exit(1))
                failure = 1;
            }
        } else {
            if (WIFSIGNALED(status)) {
            // Child was terminated by a signal
            failure = 1;
            }
        }
    }
}

    if (failure != 0) {
        return -1;
    } else {
        return 0;
    }
}
