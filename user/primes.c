#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"  // 系统调用和ulib.c中定义的函数

// 一次 sieve 调用是一个筛子阶段，会从 old_pipe 获取并输出一个素数 pime，筛除 prime 的所有倍数
// 同时创建下一 stage 的进程以及相应输入管道 new_pipe，然后将剩下的数传到下一 stage 处理
void sieve(int old_pipe[2]){
    int prime;
    read(old_pipe[0], &prime, sizeof(prime));   // 一个stage(子进程)读到的第一个数，必然是素数
    if(prime == 36) exit(0);                    // 如果读到的数是36，则代表所有数字处理完毕，退出程序
    printf("prime %d\n",prime);

    int new_pipe[2];
    pipe(new_pipe);                             // 创建用于输出到下一 stage 的进程的输出管道 new_pipe

    if(fork()!=0){  
        // 父进程（当前stage）
        close(new_pipe[0]);     // 父进程(当前stage)只需要将筛选完的数字write写入子进程，不需要从子进程读取read，所以关闭read
        int num;
        while(read(old_pipe[0], &num, 1) && num!=36){   // 从当前进程的父进程那读取数字
            if(num%prime!=0){   // 筛掉能被该进程筛掉的数字
                write(new_pipe[1], &num, sizeof(num));  // 将剩余的数字写到子进程(新的stage)
            }
        }
        num = 36;               // 补写最后的 36，标示输入完成。
        write(new_pipe[1], &num, sizeof(num));
        wait(0);                // 等待该进程的子进程完成，也就是下一 stage
        exit(0);
    }else{
        // 子进程（下一个stage）
        close(new_pipe[1]);     // 子进程(新stage)只需要从父进程read它筛选完的数字,没有写东西给父进程需求，所以关闭write
        close(old_pipe[0]);     // 这里的 old_pipe 在上一个stage中write已经关了，现在关掉read
        sieve(new_pipe);        // 子进程以父进程的输出管道作为输入，开始进行下一个 stage 的处理。
    }
}

int main(int argc, char* artv[]){
    int input_pipe[2];
    pipe(input_pipe);           // 输入管道，输入2-35之间的所有整数

    if(fork()!=0){  
        // 父进程
        close(input_pipe[0]);   // 父进程只需要write输入管道，而不需要读，关掉子进程的管道read文件描述符
        for(int i=2;i<=36;i++){ // 将数字依次写入stage（传给子进程）
            write(input_pipe[1], &i, sizeof(i));
        }
    }else{                      
        // 第一个stage的子进程
        close(input_pipe[1]);   // 子进程只需要read输入管道，而不需要写，关掉子进程的管道write文件描述符
        sieve(input_pipe);
        exit(0);
    }

    // 等待第一个stage完成。注意：这里无法等待子进程的子进程，只能等待直接子进程，无法等待间接子进程。
    // 在 sieve() 中会为每个 stage 再各自执行 wait(0)，形成等待链。
    wait(0);    
    exit(0);
}