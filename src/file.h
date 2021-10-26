#ifndef FILE_H
#define FILE_H

#include <hpgcc49.h>

// WARNING: setting FILE_BUFFER_MIDDLE to more than half of FILE_BUFFER_SIZE will lead to
// a significant performance decrease
#define FILE_BUFFER_SIZE 20
#define FILE_BUFFER_MIDDLE 8

#if FILE_BUFFER_MIDDLE >= FILE_BUFFER_SIZE
#error "FILE_BUFFER_MIDDLE is outside the file buffer"
#endif

struct File
{
	FILE *filePtr;
	unsigned int ptrPosition;
	unsigned int bufIt;
	int buf[FILE_BUFFER_SIZE];
};

typedef struct File File;

void init(File *f);

int nextChar(File *f);
int prevChar(File *f);

#endif