#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

int handle_file(const char *file_path, const int flags, const mode_t permissions) {
    int fd = -1;
    struct stat fd_stat;

    if ((fd = open(file_path, flags)) == -1) {
        if (errno == EACCES) {
            if (stat(file_path, &fd_stat) != 0) {
                printf("Error access to %s file", file_path);
            };

            if ((fd_stat.st_mode & S_IRWXU) != S_IRWXU && chmod(file_path, permissions) != 0) {
                printf("Error access to %s file", file_path);
            }

            // if dont close file permissions doesnt change
            close(fd);

            // reopen for read file with new permissions
            if ((fd = open(file_path, flags)) == -1) {
                printf("Error open %s file", file_path);
            }
        } else {
            printf("Error open %s file", file_path);
        }
    }

    return fd;
}

int main(int argc, char *argv[]) {
    char *cmd = argv[1];

    if (!cmd) {
        puts("You should pass the command as a first argument");
        exit(EXIT_FAILURE);
    }

    pid_t pid;
    int *status;

    if ((pid = fork()) == -1) {
        perror("Error fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        size_t execv_args_length = sizeof(&argv);
        char *execv_programm = argv[1];
        char *execv_args[execv_args_length - 1];

        for (size_t i = 1; i < execv_args_length; i++) {
            execv_args[i - 1] = argv[i];
        }

        const char *output_file_path = "./out.txt";
        const char *error_file_path = "./err.txt";
        const mode_t file_permissions = S_IRWXU;
        const int file_flags = O_RDWR | O_CREAT | O_APPEND;

        int output_fd = handle_file(output_file_path, file_flags, file_permissions);
        int error_fd = handle_file(error_file_path, file_flags, file_permissions);

        if (output_fd == -1 || error_fd == -1) {
            perror("Error open file");
            exit(EXIT_FAILURE);
        }

        dup2(output_fd, STDOUT_FILENO);
        dup2(error_fd, STDERR_FILENO);

        close(output_fd);
        close(error_fd);

        if (execvp(execv_programm, execv_args)) {
            perror("Error exec");
            exit(EXIT_FAILURE);
        }
    } else if (pid > 0) {
        wait(status);
    }

    return 0;
};