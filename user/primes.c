#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define READ_PORT 0
#define WRITE_PORT 1

void prime(int* left_pipe)  // 接受一个管道，执行算法，生成另一个管道，调用递归
{
    // printf("pid: %d\n", getpid());
    close(left_pipe[WRITE_PORT]);  // 不需要向左管道写。

    int num;
    if (read(left_pipe[READ_PORT], &num, sizeof(num)) != sizeof(int)) exit(-1);  // 从管道中读取一个数字
    printf("prime %d\n", num);

    //创建右侧管道
    int right_pipe[2];
    pipe(right_pipe);
    int temp_num;
    while (read(left_pipe[READ_PORT], &temp_num, sizeof(temp_num)) == sizeof(temp_num)) {
        if (temp_num % num != 0)
            write(right_pipe[WRITE_PORT], &temp_num, sizeof(temp_num));  //去除能够被num整除的数字，将其他数字写入右侧管道。 
    }

    if (fork() == 0) {
        prime(right_pipe);
    }
    else {
        close(left_pipe[READ_PORT]);
        close(right_pipe[READ_PORT]);
        close(right_pipe[WRITE_PORT]);
        wait(0);
    }

}


int main()
{
    int p[2];
    pipe(p);
    for (int i = 2; i < 34; ++i) {
        write(p[WRITE_PORT], &i, sizeof i);  // 将所有数字写入第一个管道。
    }
    if (fork() == 0) {
        prime(p);
    }
    else {
        close(p[READ_PORT]);
        close(p[WRITE_PORT]);
        wait(0);
    }
    exit(0);
}