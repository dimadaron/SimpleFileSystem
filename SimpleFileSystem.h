#include<windows.h>
#include<stdio.h>
#include<stdlib.h>

struct file {
	char* name;
	int size;
}

struct directory {
	char* name;
	file* filearray;

}

