//Accept command-line arguments: strace [args...]
//Fork a child process to run the specified command
//Enable tracing for the child process
//Wait for child completion

//Validate command-line arguments (at least 2 arguments required)
//Use fork() to create child process
//In child: exec() the target program with its arguments
//In parent: call trace() on child PID, then wait() for completion

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int trace(int pid);

static void
usage(void)
{
  fprintf(2, "usage: strace command [arg ...]\n");
  exit(1);
}

int
main(int argc, char *argv[])
{
    if (argc < 2) {
       usage();
    }

    int pfd[2];
    if (pipe(pfd) < 0) {
        fprintf(2, "strace: pipe failed\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "strace: fork failed\n");
        close(pfd[0]); close(pfd[1]);
        exit(1);
    }

    if (pid == 0) {
        // ---- Child ----
        close(pfd[1]);
        char dummy;
        (void)read(pfd[0], &dummy, 1);
        close(pfd[0]);

        if (exec(argv[1], &argv[1]) < 0) {
            fprintf(2, "strace: exec %s failed\n", argv[1]);
            exit(1);
        }
        // not reached
    } else {
        // ---- Parent ----
        close(pfd[0]);
        if (trace(pid) < 0)
            fprintf(2, "strace: trace(%d) failed\n", pid);
        (void)write(pfd[1], "x", 1);
        close(pfd[1]);

        int status = 0;
        (void)wait(&status);
        exit(status);
    }
}


