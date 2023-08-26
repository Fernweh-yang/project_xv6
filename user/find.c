#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"  // 系统调用和ulib.c中定义的函数
#include "kernel/fs.h"  // 文件状态相关

void find(char *path, char *filename){
    char buf[512], *p;  
    struct dirent de;   // 文件信息包括inode号和文件名
    struct stat st;   // 用于描述文件的状态信息

    // ! ******************** 打开传进来的path,注意!!!这个path可能是目录也可能是普通文件 ********************
    // open(,0)：0代表读，1代表写.如果打开成功返回一个file descriptor:fd
    int fd;
    if((fd=open(path,0))<0){    // 不论是普通文件、目录、设备文件，都可以被打开
        fprintf(2,"can't open the path %s\n",path);
        return;
    }
    // 获得与文件描述符fd关联的文件信息，并保存到结构体st中
    // st可以保存所有类型的文件信息(设备,文件,目录), 因此下面靠st来做递归中的判断
    if(fstat(fd,&st)<0){
        fprintf(2,"can't get the info of file %s \n",path);
        close(fd);
        return;
    }

    // ! ******************** 判断这个path是目录还是我们想找的文件 ********************
    // 通过递归,依次获得fd(当前目录)下的文件保存在st中,判断他们是filename(找的目标)还是目录
    switch (st.type){
    case T_DEVICE:
    case T_FILE:
        // ******************** 如果是文件类型, 判断是否是我们想要找的文件 ********************
        // 先path+strlen(path):指针指向path最后元素的后一位
        // 再往前strlen(filename)位,指向文件名的起始位. 
        // 最后判断这2个指针指向的字符串是否相同
		if(strcmp(path+strlen(path)-strlen(filename), filename) == 0) {
			printf("%s\n", path);
		}
        break;
    case T_DIR:
        // ******************** 如果是目录类型, 遍历该目录下的每一个文件 ********************
        // 判断路径会不会太长
        if(strlen(path)+1+DIRSIZ+1 > sizeof(buf)){
            printf("path too long \n");
            break;
        }

        // 创建当前目录的完整路径并加上'/', 以便后面遍历所有文件时直接把文件名加在后面,就是每个文件的完整路径了
        strcpy(buf, path);
        p = buf+strlen(buf);// 将p指针移动到字符数组buf(path)最后一个元素后的哪一个空字符处.
        *p++ = '/';         // 将p所指的空字符赋值'/',相当于在path后加一个"/",然后p再后移一位.

        // 从文件描述符 fd 所指向的目录文件path中读取一个文件条目的信息，包括inode号和文件名，然后将这些信息存储到 de 结构体中。
        while(read(fd, &de,sizeof(de))==sizeof(de)){
            if(de.inum == 0)  // 正常情况下inode不会为0,如果遇到就跳过,可能是文件受损
                continue;

            // ******************** 获得该目录下的每一个文件/目录/设备的完整路径 ********************
            // 将目录下每个文件的名字加在目录后,这之后p指向的字符串就是每个文件/目录的完整地址
            memmove(p, de.name, DIRSIZ);  // 从de.name复制DIRSIZ个字节数据到p所指向的内存位置
            p[DIRSIZ] = 0;                // 运用指针偏移,直接在末尾加上终止符

            // 指针p指向的是buf,所以现在buf的字符串存的是: 每个文件/目录的完整地址
            if(stat(buf, &st) == -1){       // 尝试获取指定路径 buf 的文件信息并将其存储在结构体 st 中
                printf("find: cannot stat %s\n", buf);
                continue;
            }

            // ******************** 将这个文件的路径送入递归 ********************
            // 不要进入 `.` 和 `..`
            // buf+strlen(buf)-2 将指针移动到路径字符串的倒数第二个字符处
            // 然后使用 strcmp 函数来比较这个子字符串与 "/." 是否相等。 /..同理
			if(strcmp(buf+strlen(buf)-2, "/.") != 0 && strcmp(buf+strlen(buf)-3, "/..") != 0) {
				find(buf, filename); // 递归查找
			}
        }
        break;
    }
    close(fd); // 关闭每一个递归下新开的文件描述符
}

int main(int argc, char* argv[]){

    if(argc < 3){
        fprintf(2,"usage: find [path] [filename] \n");
        exit(0);
    }
    find(argv[1],argv[2]);  // find path filename
    exit(0);
}