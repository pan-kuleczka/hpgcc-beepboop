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

void file_init(File *f);

int file_tell(File *f);

int file_getc(File *f);
int file_prevc(File *f);

int file_seek(File *f, int delta, int type);
int file_getcAt(File *f, int delta, int type);

int file_skipTo(File *f, int (*stopFunction)(File *));
int file_skipToChar(File *f, char c);
int file_skipToNewline(File *f);

#endif
