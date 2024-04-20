#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define READ_PORT 0
#define WRITE_PORT 1


int main()
{
    int parentToChild[2];
    int childToParent[2];
    pipe(parentToChild);
    pipe(childToParent);

    char buf[64];

    int pid = fork();
    if (pid == 0) {
        int id = getpid();
        write(childToParent[WRITE_PORT], "pong", 4);
        close(childToParent[WRITE_PORT]);
        int n = read(parentToChild[READ_PORT], buf, sizeof(buf));
        close(parentToChild[READ_PORT]);
        if (n < 0) exit(-1);
        printf("%d: received %s\n", id, buf);
        exit(0);
    }
    else {
        write(parentToChild[WRITE_PORT], "ping", 4);
        close(parentToChild[WRITE_PORT]);
        wait(0);
        int n = read(childToParent[READ_PORT], buf, sizeof(buf));
        int id = getpid();
        if (n < 0) exit(-1);
        printf("%d: received %s\n", id, buf);
        close(childToParent[READ_PORT]);
        exit(0);
    }
}