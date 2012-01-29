#include<windows.h>
#include<stdio.h>
#include<stdlib.h>
#include <conio.h>
#include <tchar.h>

#define FILE_NAME 16
#define MAX_FILES 20
#define MAX_OPEN_FILES 20


BOOL initFS(char* OsFileName, int SizeInBlocks);/* size in blocks */ 
BOOL attachfs(char* OsFileName);
BOOL detachfs();

void* create (char* FileName, int SizeBlocks);
void* open   (char* FileName, int RWflag);
int   read   (int fd, int BlockNum, char* Buff, int BuffLengh);
int   write  (int fd, int BlockNum, char* Buff, int BuffLengh);
BOOL close(int fd);

BOOL renameFile(char* OldFileName, char* NewFileName);
BOOL deleteFile(char* FileName);

void createVirtualDisk(char* OsFileName, int virtualDiskSize);
void CreateMyBitmap();
void CreateFolder();
void CreateOpenFiles();
void BlockOccupy(int blockNum);
void BlockFree(int blockNum);


typedef struct fileInFolder 
{
	char* filename;
	int blockNumber;
	int numOfBlock;
	BOOL free;
} File;

typedef struct openFile
{
	File file;
	int readWrite;
} OpenFiles;
//
//struct directory {
//	char* name;
//	file* fileArray;
//}

