#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"


// 从路径中提取文件名
// 输入 “a/b/c”
// 输出 “c"
char*
myFmtName(char *path)  
{
    char *p;
     // Find first character after last slash.
    for(p=path+strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    return p;
}


void find(char* path, char* findName)  // 路径，文件名
{
    // 首先判断path是文件夹还是文件，如果是文件，则比对文件名字，如果是文件夹，则递归调用find


    int fd;  // 文件描述符
    struct stat st;  // int dev(文件系统的磁盘设备)， uint ino(id), short type(T_DIR = 1, T_FILE = 2, T_DEVICE = 3)， short nlink(指向文件的链接数), uint64 size 字节数
    char buf[512], *p;
    struct dirent de;
    // dirent格式如下：
    // struct dirent {
    //     ushort inum;  // id
    //     char name[DIRSIZ];  // 名字
    // }; 


    if((fd = open(path, 0)) < 0){  
        fprintf(2, "ls: cannot open %s\n", path);
        return;
    }

    if(fstat(fd, &st) < 0){   // 获得文件stat信息。
        fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type) {
    case T_FILE:
        if (strcmp(myFmtName(path), findName) == 0) {
            printf("%s\n",path);
        }
        else {
            // printf("%s is file\n", path);
            // printf("(%d)%s != %s(%d)\n",strlen(myFmtName(path)), myFmtName(path), findName, strlen(findName));
        }
        
        break;

    case T_DIR:
        // printf("%s is dir\n", path);
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
            printf("ls: path too long\n");
            break;
        }
        strcpy(buf, path);  // path 复制到 buf
        p = buf+strlen(buf);
        *p++ = '/';
        while(read(fd, &de, sizeof(de)) == sizeof(de)){  // 循环获取文件夹中所有文件，fd描述符为T_DIR类型，则内部保存多个文件，每个文件格式为struct dirent
            if(de.inum == 0)
                continue;   
            if (!strcmp(de.name, ".") || !strcmp(de.name, "..")) continue;  // 去掉. 和 ..

            memmove(p, de.name, DIRSIZ);  // void *memmove(void *dest, const void *src, size_t n);
            p[DIRSIZ] = 0;
            // printf("a|%s|a\n", buf);
            if(stat(buf, &st) < 0){
                printf("ls: cannot stat %s(%d)\n", buf, strlen(buf));
                continue;
            }
            find(buf, findName);
        }
        break;
    }
    close(fd);   // 如果不关闭描述符，会导致一些文件无法读取, 类似资源泄露!!!!!!!!!!!!!!!
    return;
    
}


int main(int argc, char* argv[])
{
    if (argc != 3) {
        printf("input error");
        exit(-1);
    }
    find(argv[1], argv[2]);
    exit(0);
}