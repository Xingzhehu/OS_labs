#include "kernel/types.h"
#include "user.h"

int main(int argc, char *argv[]) {
  int c2f[2];
  int f2c[2];

  pipe(c2f);
  pipe(f2c);

  int ppid = getpid();
  int pid = fork();

  if (pid == 0) {
    // 子进程不用f2c的写端和c2f的读端
    close(f2c[1]);
    close(c2f[0]);

    int from_parent;
    read(f2c[0], &from_parent, sizeof(from_parent));
    int self = getpid();
    printf("%d: received ping from pid %d\n", self, from_parent);

    write(c2f[1], &self, sizeof(self));

    close(f2c[0]);
    close(c2f[1]);
    exit(0);
  } else {
    // 父进程不用f2c的读端和c2f的写端
    close(f2c[0]);
    close(c2f[1]);

    write(f2c[1], &ppid, sizeof(ppid));

    int from_child;
    read(c2f[0], &from_child, sizeof(from_child));
    printf("%d: received pong from pid %d\n", ppid, from_child);

    close(f2c[1]);
    close(c2f[0]);
    wait(0);
    exit(0);
  }
}
