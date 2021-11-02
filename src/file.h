#ifndef FILE_H
#define FILE_H

#include <hpgcc49.h>

#define FILE_BUFFER_SIZE 128
#define MIN_FILE_BUFFER_MEMORY 64

#if MIN_FILE_BUFFER_MEMORY > FILE_BUFFER_SIZE
#error "Min file buffer memory is too large"
#endif

#define FILE_SEEK_BACKWARD_LOOKBEHIND 0

struct File
{
	FILE *filePtr;
	unsigned int ptrPosition;
	unsigned int fileSize;
	int bufIt;
	int buf[FILE_BUFFER_SIZE];
};

typedef struct File File;

void init(File *f);

int nextChar(File *f);
int prevChar(File *f);

int moveFilePtr(File *f, int delta, int type);
int getCharAt(File *f, int delta, int type);

#endif