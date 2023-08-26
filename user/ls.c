#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void
ls(char *path)
{
  char buf[512], *p;
  int fd;
  struct dirent de;   // 文件信息包括inode号和文件名
  struct stat st;     // 用于描述文件的状态信息

  // open(,0)：0代表读，1代表写
  // open()如果成功返回一个file descriptor:fd
  if((fd = open(path, 0)) < 0){
    // 2：标准错误输出
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  // 获得与文件描述符fd关联的文件信息，并保存到结构体st中
  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  // 判断st存储的文件fd类型
  printf("filename file_type file_inode file_size\n");
  switch(st.type){
  case T_DEVICE:  // 3
  case T_FILE:    // 2
    printf("%s %d %d %l\n", fmtname(path), st.type, st.ino, st.size);
    break;

  case T_DIR:     // 1
    // DIRSIZ=14:目录名的size为14
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)){
      printf("ls: path too long\n");
      break;
    }
    // 复制完整路径
    strcpy(buf, path);
    p = buf+strlen(buf);// 将p指针移动到字符数组buf(path)最后一个元素后的哪一个空字符处.
    *p++ = '/';         // 将p所指的空字符赋值'/',相当于在path后加一个"/",然后p再后移一位.
    // 从fd:当前路径下,依次读取文件名
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)  // 正常情况下inode不会为0,如果遇到就跳过,可能是文件受损
        continue;
      memmove(p, de.name, DIRSIZ);  // 从de.name复制DIRSIZ个字节数据到p所指向的内存位置
      p[DIRSIZ] = 0;                // 运用指针偏移,直接在末尾加上终止符
      
      if(stat(buf, &st) < 0){
        printf("ls: cannot stat %s\n", buf);
        continue;
      }
      printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    ls(".");
    exit(0);
  }
  for(i=1; i<argc; i++)
    ls(argv[i]);
  exit(0);
}
