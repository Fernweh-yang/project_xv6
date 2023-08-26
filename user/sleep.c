#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"  // 系统调用和ulib.c中定义的函数

// argc: 表示命令行参数的数量，包括程序本身。 比如./sleep arg1 arg2 此时argc=3
// argv: 一个指向指针数组的指针，每个指针指向一个字符串，代表一个命令行参数。
// argv[0] 指向字符串./sleep，argv[1]指向字符串arg1,argv[2]指向字符串arg2
int main(int argc, char*argv[]){
    if(argc <= 1){
        // fprintf():C语言中的一个标准库函数，用于格式化输出数据到文件流。
        fprintf(2,"usage: sleep [seconds]\n");  // 参数 2 表示输出到标准错误流（stderr）
        exit(1);    // 表示程序异常终止
    }
    int tick = atoi(argv[1]);  // 使用的user/ulib.c的atoi()将字符串转为数字
    sleep(tick);    // 睡眠tick/10秒
    exit(0);        // 表示程序正常退出
}