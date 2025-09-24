#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user.h"

// 返回path中最后一个元素的指针
char* last(char *path) {
  char *res = path;
  for (char *p = path; *p; p++) {
    if (*p == '/') res = p + 1;
  }
  return res;
}

// 在path目录下递归查找名字为target的文件
void dofind(char *path, char *target) {
  int fd;
  struct stat st;

  if ((fd = open((char*)path, 0)) < 0) {
    printf("find: cannot open %s\n", path);
    return;
  }
  if (fstat(fd, &st) < 0) {
    printf("find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  if (strcmp(last(path), target) == 0) {
    printf("%s\n", path);
  }

  if (st.type == T_DIR) {
    char buf[512], *p;
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
      printf("find: path too long\n");
      close(fd);
      return;
    }

    strcpy(buf, path);
    p = buf + strlen(buf);
    if (*(p - 1) != '/') *p++ = '/'; // 添加路径分隔符

    struct dirent de;
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      if (de.inum == 0) continue;
      if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) continue;

      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      dofind(buf, target);
    }
  }

  close(fd);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: find <path> <name>\n");
    exit(1);
  }
  dofind(argv[1], argv[2]);
  exit(0);
}

