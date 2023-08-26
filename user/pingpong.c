#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"  // 系统调用和ulib.c中定义的函数

int main(int argc, char* artv[]){
    /*
        ! 创建2个用于存储管道的文件描述符，分别用于父进程到子进程和子进程到父进程
        这样的设计保证了父进程和子进程的交替进行通信，实现了一种简单的同步机制。
        通过不同的管道进行读写操作，可以确保不同的进程不会发生竞争条件。
    */
    int p2c[2],c2p[2];   
    
    // 通过系统调用pipe()创建管道，read/write file descriptor分别存在p[0]/p[1]
    pipe(p2c);  // 父进程 -> 子进程
    pipe(c2p);  // 子进程 -> 父进程

    if(fork()!=0){              // 在父进程中，fork返回新创建的子进程pid
        write(p2c[1],"x",1);    // 把x通过管道发送给子进程
        char buf;
        read(c2p[0],&buf,1);    // 从子进程的通道读取字符,存到buf中
        printf("%d: received pong\n",getpid());
        // printf("parent buf:%c \n",buf);
    }else{                      // 在子进程中，fork返回0；
        char buf;
        read(p2c[0],&buf,1);    // 从父进程的通道读取字符,存到buf中
        printf("%d: received ping\n",getpid());
        // printf("child buf:%c \n",buf);
        write(c2p[1],&buf,1);    // 把x通过管道发送给父进程
    }
    exit(0);
}