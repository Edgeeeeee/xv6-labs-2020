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
        // 子进程从p2c读，向c2p写，所以不需要p2c写和c2p读，关闭端口。
        close(parentToChild[WRITE_PORT]);
        close(childToParent[READ_PORT]);

        write(childToParent[WRITE_PORT], "pong", 4);
        
        int n = read(parentToChild[READ_PORT], buf, sizeof(buf));
        
        if (n < 0) exit(-1);
        printf("%d: received %s\n", id, buf);

        close(childToParent[WRITE_PORT]);
        close(parentToChild[READ_PORT]);
        exit(0);
    }
    else {
        close(parentToChild[READ_PORT]);
        close(childToParent[WRITE_PORT]);

        write(parentToChild[WRITE_PORT], "ping", 4); 
        wait(0);
        int n = read(childToParent[READ_PORT], buf, sizeof(buf));
        int id = getpid();
        if (n < 0) exit(-1);
        printf("%d: received %s\n", id, buf);

        close(parentToChild[WRITE_PORT]);
        close(childToParent[READ_PORT]);
        exit(0);
    }
}