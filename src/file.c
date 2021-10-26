#include "file.h"
#include <hpgcc49.h>

void init(File *f){
	f->ptrPosition = 0;
	f->bufIt = 0;
	fgets(f->buf, FILE_BUFFER_SIZE, f->filePtr);
}

int nextChar(File *f)
{
    
    f->bufIt++;
	if (f->bufIt == FILE_BUFFER_SIZE)
	{
        memcpy(f->buf, f->buf + FILE_BUFFER_SIZE - FILE_BUFFER_MIDDLE, FILE_BUFFER_MIDDLE);
		fgets(f->buf + FILE_BUFFER_MIDDLE, FILE_BUFFER_SIZE - FILE_BUFFER_MIDDLE, f->filePtr);
		f->bufIt = FILE_BUFFER_MIDDLE;
	}

    int c = f->buf[f->bufIt];
	if(c > -1) ++f->ptrPosition;

    return c;
}

int prevChar(File *f){
    
}

/*int nextFileChar(File *f)
{
	int c = fgetc(f->filePtr);
	if (c > -1)
		++f->ptrPosition;
	return c;
}

int moveBack(File *f)
{
	if (f->ptrPosition > 0)
	{
		ungetc(f->filePtr);
		f->ptrPosition--;
		return 0;
	}
	return -1;
}

int moveFilePtr(File *f, int offset, int whence)
{
	if (whence == SEEK_END)
		return -1;
	int ret = fseek(f->filePtr, offset, whence);
	if (!ret)
	{
		switch (whence)
		{
		case SEEK_SET:
			f->ptrPosition = offset;
			break;
		case SEEK_CUR:
			f->ptrPosition += offset;
			break;
		default:
			break;
		}
	}
	return ret;
}

int fileCharAt(File *f, unsigned int pos)
{
	int currentPos = f->ptrPosition;
	int result = moveFilePtr(f, pos, SEEK_SET);
	int c = nextFileChar(f);
	moveFilePtr(f, currentPos, SEEK_SET);

	if (result != 0)
		return -1;
	return c;
}

unsigned int filePtrTell(File *f)
{
	return f->ptrPosition;
}

void skipToNextLine(File *f)
{
	int c;
	do
	{
		c = nextFileChar(f);
	} while (c > -1 && c != '\n');
}*/
