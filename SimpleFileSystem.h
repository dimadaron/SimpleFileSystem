#include<windows.h>
#include<stdio.h>
#include<stdlib.h>



BOOL initFS(char* OsFileName, int SizeInBlocks);
BOOL attachfs(char* OsFileName);
BOOL detachfs();

void* create (char* FileName, int SizeBlocks);
void* open   (char* FileName, char RWflag);
int   read   (int fd, int BlockNum, char* Buff, int BuffLengh);
int   write  (int fd, int BlockNum, char* Buff, int BuffLengh);
BOOL close(int fd);

BOOL rename(char* OldFileName, char* NewFileName);
BOOL deleteFile(char* FileName);


struct file {
	char* name;
	int size;
}

struct directory {
	char* name;
	file* fileArray;
}

