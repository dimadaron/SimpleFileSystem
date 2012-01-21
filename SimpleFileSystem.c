
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

#define BUF_SIZE ((1<<20)*100)
TCHAR szName[]=TEXT("Global\\MyFileMappingObject");
TCHAR szMsg[]=TEXT("Message from first process.");

int _tmain()
{
	TCHAR VirtualDisc[]=TEXT("Carmi.carmi");
	HANDLE hMapFile;
	char* pBuf;
	HANDLE hFile = INVALID_HANDLE_VALUE;

	hFile = CreateFile (
		(LPCTSTR)VirtualDisc,
		(GENERIC_READ | GENERIC_WRITE),
		NULL,
		NULL,
		CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL,
		NULL);


	hMapFile = CreateFileMapping(
		hFile,                   // use local file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		BUF_SIZE,                // maximum object size (low-order DWORD)
		szName);                 // name of mapping object

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Could not create file mapping object (%d).\n"),
			GetLastError());
		return 1;
	}

	pBuf = (char*) MapViewOfFile(hMapFile,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,
		0,
		BUF_SIZE);

	if (pBuf == NULL)
	{
		_tprintf(TEXT("Could not map view of file (%d).\n"),
			GetLastError());

		CloseHandle(hMapFile);

		return 1;
	}

	UnmapViewOfFile(pBuf);

	CloseHandle(hMapFile);

	return 0;
}

