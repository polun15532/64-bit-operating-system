#ifndef __UNISTD_H__
#define __UNISTD_H__

int close(int fildes);
long read(int fildes, void *buf, long nbyte);
long write(int fildes, const void *buf, long nbyte);
long lseek(int fildes, long offset, int whence);

int fork();
int vfork();
int execve(const char *path, char *const argv[], char *const envp[]);
unsigned long brk(unsigned long brk);

#endif
