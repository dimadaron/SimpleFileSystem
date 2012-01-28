

#include "SimpleFileSystem.h"

#define MB *(1<<20)
#define BLOCK_SIZE 512
#define BYTE_SIZE 8
#define CLOSED 0
#define READ 1
#define WRITE 2
#define READWRITE 3

TCHAR szName[]=TEXT("Global\\MyFileMappingObject");
TCHAR szMsg[]=TEXT("Message from first process.");

char* pDisk;
char* bitMap;
File* folder;
OpenFiles* openFiles;
char* data;
HANDLE Mutex;

int diskInBlocksSize;
int fileSystemSizeInBlocks;
int blocksForBitmap;
int blocksForFolder;
int blocksForOpenFiles;




int main()
{
	HANDLE myHandle;
	char* result = (char*)malloc(1);

	//TCHAR VirtualDisc[]=TEXT("Carmi.carmi");
	initFS("Carmi.Carmi1",100000);
	myHandle = create("dima.txt", 10);
	myHandle = open("dima.txt", READWRITE);
	write((int)myHandle, 0, "Dima is the best", 17);
	read((int)myHandle, 0, result , 17);
	puts(result);
	close((int)myHandle);	



//	UnmapViewOfFile(pDisk);

//	CloseHandle(hMapFile);

	return 0;
}




BOOL initFS(char* OsFileName, int SizeInBlocks)
{
//	HANDLE hFile;

	fileSystemSizeInBlocks = SizeInBlocks;

	diskInBlocksSize = fileSystemSizeInBlocks * BLOCK_SIZE;

	createVirtualDisk(OsFileName, diskInBlocksSize);
	//openMapView(OsFileName, sizeInBlocks,hFile);
	CreateMyBitmap();
	CreateFolder();
	CreateOpenFiles();

	return 0;

}

void createVirtualDisk(char* OsFileName, int virtualDiskSize)
{
		
	HANDLE hFile, hMapFile;

	hFile = CreateFile (
		(LPCWSTR)OsFileName,
		(GENERIC_READ | GENERIC_WRITE),
		(FILE_SHARE_READ | FILE_SHARE_WRITE),
		NULL,
		CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	hMapFile = CreateFileMapping(
		hFile,                   // use local file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		virtualDiskSize,                // maximum object size (low-order DWORD)
		szName);                 // name of mapping object


	pDisk = (char*) MapViewOfFile(hMapFile,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,
		0,
		0);

/*	if (pDisk == NULL)
	{
		_tprintf(TEXT("Could not map view of file (%d).\n"),
			GetLastError());

		CloseHandle(hMapFile);

		return 1;
	} */
}

void CreateMyBitmap()
{
	int bytesForBitmap;
	int i;
	
	bitMap = pDisk;
	bytesForBitmap = fileSystemSizeInBlocks / BYTE_SIZE;
	blocksForBitmap = (bytesForBitmap / BLOCK_SIZE) + 1;

	for (i = 0; i < blocksForBitmap; i++)
	{
		BlockOccupy(i);
	}

	for (i = blocksForBitmap; i < fileSystemSizeInBlocks; i ++)
	{
		BlockFree(i);
	}

}

void CreateFolder()
{
	int bytesForFolder;
	int i;

	folder = (File*)bitMap + blocksForBitmap * BLOCK_SIZE;

	bytesForFolder = sizeof(File) * MAX_FILES;
	blocksForFolder = bytesForFolder / BYTE_SIZE + 1;

	
	// 
	for (i = blocksForBitmap; i < (blocksForBitmap + blocksForFolder); i++)
	{
		BlockOccupy(i);
	}

	// free all files in folder
	for (i = 0; i < MAX_FILES; i ++)
	{
		folder[i].free = 1;
	}

}

void CreateOpenFiles()
{
	int bytesForOpenFiles;
	int i;

	openFiles = (OpenFiles*)folder + blocksForFolder * BLOCK_SIZE;

	bytesForOpenFiles = sizeof(OpenFiles) * MAX_FILES;
	blocksForOpenFiles = bytesForOpenFiles / BYTE_SIZE + 1;

	
	// blocks for the open files section
	for (i = blocksForBitmap + blocksForFolder; i <(blocksForBitmap + blocksForFolder + blocksForFolder); i++)
	{
		BlockOccupy(i);
	}

	for (i = 0; i < MAX_OPEN_FILES; i ++)
	{
		openFiles[i].readWrite = 0; // free space in open files array
	}
}

void BlockOccupy(int blockNum)
{
	int byteNumber, posInByte;

	byteNumber = blockNum / BYTE_SIZE + 1;
	posInByte = blockNum % BYTE_SIZE;

	bitMap[byteNumber]  |= 1 << posInByte;
}

void BlockFree(int blockNum)
{
	int byteNumber, posInByte;

	byteNumber = blockNum / BYTE_SIZE + 1;
	posInByte = blockNum % BYTE_SIZE;

	bitMap[byteNumber] = bitMap[byteNumber] & ~(1 << posInByte);
}

void* create(char* FileName, int sizeInBlocks)
{
	int sizeOfSystemBlock;
	int freeBlocksCounter;
	int startBlockForFile;
	int i, j;
	freeBlocksCounter = 0;
	Mutex = CreateMutex(NULL, FALSE, NULL);

	sizeOfSystemBlock = blocksForBitmap + blocksForFolder + blocksForOpenFiles;

	for(i = sizeOfSystemBlock + 1; i < fileSystemSizeInBlocks; i++ )
	{
		WaitForSingleObject(Mutex, INFINITE);
		// TODO use mutex to protect bitmap  
		if(bitMap[i]!=0)
		{
			freeBlocksCounter = 0;
			continue;
		}
		freeBlocksCounter ++;
		if(sizeInBlocks == freeBlocksCounter)
		{
			for(j = 0; j < MAX_FILES; j++)
			{
				if (folder[j].free == TRUE)
				{
					folder[j].filename = FileName;
					folder[j].blockNumber = i - sizeInBlocks + 1;
					folder[j].numOfBlock = sizeInBlocks;
					folder[j].free = FALSE;
					BlockOccupy(sizeInBlocks);
					ReleaseMutex(Mutex);
					return &folder[j];
				}
			}
			ReleaseMutex(Mutex);
			printf("Max number of files is reachead");
			return INVALID_HANDLE_VALUE;
		}
	}
	ReleaseMutex(Mutex);
	printf("Not enough disk space");
	return  INVALID_HANDLE_VALUE;
}

void* open (char* FileName, int RWflag)
{
	int i, j;
	for(i = 0; i < MAX_FILES; i++)
	{
		if(folder[i].filename == FileName)
		{
			for(j = 0; j < MAX_OPEN_FILES; j++)
			{
				if(openFiles[j].readWrite == 0)
				{
					openFiles[j].file = folder[i];
					openFiles[j].readWrite = RWflag;
					return &(folder[i]);
				}
			}
			printf("you'v reached max of opend files");
			return INVALID_HANDLE_VALUE;
		}	
	}
	printf("No such file");
	return INVALID_HANDLE_VALUE;
}

BOOL close (int fd)
{
	int i;
	File* fileToClose;
	fileToClose = (File*)fd;

	for(i = 0; i < MAX_OPEN_FILES; i ++)
	{
		if (openFiles[i].file.blockNumber == fileToClose->blockNumber)
		{
			openFiles[i].readWrite = 0;
			return TRUE;
		}
	}
	printf("No such file apeers to be open");
	return FALSE;
}

int write(int fd, int BlockNum, char* Buff, int BuffLengh)
{
	int i;
	File* fileToWrite;
	fileToWrite = (File*)fd;
	Mutex = CreateMutex(NULL, FALSE, NULL);
	WaitForSingleObject(Mutex, INFINITE);

	for(i = 0; i < MAX_OPEN_FILES; i ++)
	{
		if (openFiles[i].file.blockNumber == fileToWrite->blockNumber)
		{
			 if (openFiles[i].readWrite == WRITE || openFiles[i].readWrite == READWRITE)
			 {
				memcpy(&pDisk[fileToWrite->blockNumber + BlockNum], Buff, BuffLengh);
				ReleaseMutex(Mutex);
				return BuffLengh;  
			 }
			 else
			 {
				ReleaseMutex(Mutex);
				printf("File isn't open to writing");
				return -1;
			 }
		}
	}
	ReleaseMutex(Mutex);
	printf("No such file apeers to be open");
	return -1;
}

int   read  (int fd, int BlockNum, char* Buff, int BuffLengh)
{
	int i;
	char* result = (char*) malloc(BuffLengh);
	File* fileToRead;
	fileToRead = (File*)fd;
	Mutex = CreateMutex(NULL, FALSE, NULL);
	WaitForSingleObject(Mutex, INFINITE);

	for(i = 0; i < MAX_OPEN_FILES; i ++)
	{
		if (openFiles[i].file.blockNumber == fileToRead->blockNumber)
		{
			 if (openFiles[i].readWrite == READ || openFiles[i].readWrite == READWRITE)
			 {
				 memcpy(result,(&pDisk[fileToRead->blockNumber + BlockNum]),(size_t)BuffLengh);
			//	FIXME memory vailation 
				 strncpy(Buff, result,BuffLengh);
				ReleaseMutex(Mutex);
				return BuffLengh;  
			 }
			 else
			 {
				ReleaseMutex(Mutex);
				printf("File isn't open to reading");
				return -1;
			 }
		}
	}
	ReleaseMutex(Mutex);
	printf("No such file apeers to be open");
	return -1;

}

