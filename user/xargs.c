#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define MAXSZ 512
// 定义状态机
enum state
{
    S_WAIT,         // 等待状态，该状态设置为起始状态，并根据输入字符类型（char_type）判断下一个状态
    S_ARG,          // 正在输入参数
    S_ARG_END,      // 当前输入参数结束
    S_ARG_LINE_END, // 输入参数后遇到换行
    S_LINE_END,     // 行结束
    S_END,          // 结束状态
};
// 定义字符类型
enum char_type
{
    C_SPACE,
    C_CHAR,
    C_LINE_END,
};

// 获得字符对应的类型，对空格，换行特殊处理
enum char_type get_char_type(char c)
{
    switch (c)
    {
    case ' ':
        return C_SPACE;
        break;
    case '\n':
        return C_LINE_END;
        break;
    default:
        return C_CHAR;
        break;
    }
};

// 状态机转换规则
enum state transform_state(enum state cur_state, enum char_type ct)
{
    switch (cur_state)
    {
    case S_WAIT:
        if (ct == C_SPACE)
            return S_WAIT;
        if (ct == C_LINE_END)
            return S_LINE_END;
        if (ct == C_CHAR)
            return S_ARG;
        break;
    case S_ARG:
        if (ct == C_SPACE)
            return S_ARG_END;
        if (ct == C_LINE_END)
            return S_ARG_LINE_END;
        if (ct == C_CHAR)
            return S_ARG;
        break;
    case S_ARG_END:
    case S_ARG_LINE_END:
    case S_LINE_END:
        if (ct == C_SPACE)
            return S_WAIT;
        if (ct == C_LINE_END)
            return S_LINE_END;
        if (ct == C_CHAR)
            return S_ARG;
        break;
    default:
        break;
    }
    return S_END;
};

// 去除\n
void clearArgv(char *x_argv[MAXARG], int beg)
{
    for (int i = beg; i < MAXARG; i++)
    {
        x_argv[i] = 0;
    }
}

// main
int main(int argc, char *argv[])
{

    if (argc - 1 >= MAXARG) // 最后一个必须是0，这是判断参数结束的方法，类似于字符串判断方法。
    {
        fprintf(2, "xargs: too many arguments\n");
        exit(1);
    }
    char lines[MAXSZ];
    char *p = lines;
    char *x_argv[MAXARG] = {0}; // xargc参数列表，通过将管道左端输出加入到参数列表中，之后在子进程中执行exec()系统调用来实现xargs

    // 保存原有参数
    //  输入为 echo a | xargs echo b 时
    //  argc = 3, argv分别为 xargs echo b
    //  不考虑管道情况下，args[1]为实际执行命令，所以从args[1]开始保存。

    for (int i = 1; i < argc; i++)
    {
        x_argv[i - 1] = argv[i];
    }
    // begin index
    int arg_beg = 0;        // 指向line中起始位置
    int arg_end = 0;        // 指向line中参数结束位置
    int arg_cnt = argc - 1; // 当前参数数量

    enum state st = S_WAIT; // 初始状态

    while (st != S_END)
    {
        if (read(0, p, sizeof(char)) != sizeof(char)) // 将管道输出结果通过char*p ,逐字符输入到line中
        {
            st = S_END; // 读取完成则设置为END状态。
        }
        else
        {
            st = transform_state(st, get_char_type(*p)); // 根据当前状态和当前读入字符做转换
        }
        // if end index bigger than maxsz
        if (++arg_end >= MAXSZ)
        {
            fprintf(2, "xargs: arguments too long\n");
            exit(1);
        }
        switch (st)
        {
        case S_WAIT: // 等待状态，即读取到结果，但是是个没什么用的结果，多个空格等。
            ++arg_beg;
            break;
        case S_ARG_END:                          // 一个参数输入结束
            x_argv[arg_cnt++] = &lines[arg_beg]; // lines是一个大的字符数组，内部包含多个字符串参数，x_argv中的char*指向各个字符串参数的起始位置。
            arg_beg = arg_end;
            *p = '\0';
            break;
        case S_ARG_LINE_END: // 先保存参数，之后按照遇到换行处理
            x_argv[arg_cnt++] = &lines[arg_beg];
            // no break
        case S_LINE_END: // 遇到换行符表示输入结束
            arg_beg = arg_end;
            *p = '\0';
            if (fork() == 0)
            {
                exec(argv[1], x_argv); // int exec(char *path, char **argv)  路径，参数列表，列表中第一个元素为执行命令（echo等）。
            }
            arg_cnt = argc - 1;
            clearArgv(x_argv, arg_cnt); // 每次执行之后，将已经执行的参数清空
            // 在linux系统下
            // 执行 echo "1\n2\n3" | echo line
            // line 1
            // line 2
            // line 3
            // 即遇到换行就执行一次，最终可能执行多次echo line
            wait(0);
            break;
        default:
            break;
        }
        ++p;
    }
    exit(0);
}
