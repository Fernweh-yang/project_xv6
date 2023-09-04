// xargs.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// 带参数列表，执行某个程序
void run(char *program, char **args) {
	if(fork() == 0) { // 在子进程中，fork返回0；
		exec(program, args);	// 将参数args的指针传递给程序program，并执行
		exit(0);				// 终止当前进程，0表示程序正常终止
	}
	return; 		  // 父进程就直接返回
}

int main(int argc, char *argv[]){
	char buf[2048]; 				// 读入时使用的内存池
	char *p = buf, *last_p = buf; 	// 当前参数的结束、开始指针
	// 定义了一个长度为 128 的字符指针数组。用于保存argv 传进来的参数和 stdin 读入的参数
	char *argsbuf[128]; 		
	// 指向指针的指针，初始化为argsbuf数组第一个元素的指针。
	char **args = argsbuf; 	

	// ! 考虑一个命令 $ echo hello world | xargs echo bye
	// ********* 开始从命令行中读取参数: echo bye *********
	// 将所有的命令行参数的指针，保存到指针数组argsbuf中去
	for(int i=1;i<argc;i++) {
		*args = argv[i];	// 将参数argv[i]的指针复制给指针args，即给argsbuf数组的某一元素
		args++;				// 指针指向argsbuf数组的下一个元素
	}

	// ********* 开始从标准输入stdin中读取参数: echo hello world命令的结果 *********
	// 此时argsbuf存了echo和bye2个字符串，args指向bye后面一个字符串的指针
	char **pa = args; // pa用于指向每一个从stdin读取到的参数的开始位置
	// 系统调用read，从标准输入0读取数据，将数据存储到p指针所指向的内存位置，每次读取1个字节的数据
	while(read(0, p, 1) != 0) {
		// 读入一个参数完成（以空格分隔，如 `echo hello world`，则 hello 和 world 各为一个参数）
		if(*p == ' ' || *p == '\n') {
			// 将空格替换为 \0 分割开各个参数，这样可以直接使用内存池中的字符串作为参数字符串，而不用额外开辟空间
			// \0是终止符，标志字符串的结束
			*p = '\0';
			// 将读取完的参数hello存储到argsbuf数组中去
			// 此时p指向了hello\0的\0，last_p还指向开头的字符h
			*(pa++) = last_p;
			// 将last_p指向下一个参数,world的开头字符w
			last_p = p+1;

			// 如果遇到了换行符号
			if(*p == '\n') {
				*pa = 0; // 参数列表末尾用 null 标识列表结束
				run(argv[1], argsbuf); // 执行最后一行指令，这里是echo bye hello world
				pa = args; // 重置读入参数指针，准备读入下一行
			}
		}
		p++;
	}
	// 如果最后一行不是空行，即最后没有遇到换行符\n
	// 此时args还是指向bye后面一个字符串的位置,即hello
	// 此时pa指向world后面一个字符串的位置
	if(pa != args) { 
		*p = '\0';
		*(pa++) = last_p;
		*pa = 0; // 参数列表末尾用 null 标识列表结束
		run(argv[1], argsbuf);	// 子进程：执行最后一行指令，这里是echo bye hello world
	}
	while(wait(0) != -1) {}; // 循环等待所有子进程完成，每一次 wait(0) 等待一个
	exit(0);
}