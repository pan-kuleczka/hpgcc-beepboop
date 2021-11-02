#include "file.h"
#include <hpgcc49.h>

void init(File *f)
{
	f->fileSize = 0;
	while (fgetc(f->filePtr) > -1)
		++f->fileSize;
	rewind(f->filePtr);

	f->ptrPosition = 0;
	f->bufIt = 0;
}

int nextChar(File *f)
{
	f->buf[f->bufIt] = fgetc(f->filePtr);
	int c = f->buf[f->bufIt];

	if (c > -1)
	{
		++f->bufIt;
		++f->ptrPosition;

		if (f->bufIt == FILE_BUFFER_SIZE)
		{
			memcpy(f->buf, f->buf + FILE_BUFFER_SIZE - MIN_FILE_BUFFER_MEMORY, MIN_FILE_BUFFER_MEMORY);
			f->bufIt = MIN_FILE_BUFFER_MEMORY;
		}
	}

	return c;
}

int __getPos(File *f, int delta, int type)
{
	switch (type)
	{
	case SEEK_SET:
		return delta;
	case SEEK_CUR:
		return f->ptrPosition + delta;
	case SEEK_END:
		return f->fileSize + delta;
	default:
		return -1;
	}
}

int __moveFilePtr(File *f, int position)
{
	if (position < 0 || position >= f->fileSize)
		return -1;

	int bufBegin = f->ptrPosition - f->bufIt;
	int delta = position - (bufBegin + f->bufIt);

	f->ptrPosition = position;

	if (position < bufBegin)
	{
		int lookbehind = min(position, FILE_SEEK_BACKWARD_LOOKBEHIND);
		fseek(f->filePtr, position - lookbehind, SEEK_SET);
		fgets(f->buf, lookbehind, f->filePtr);
		f->bufIt = lookbehind;
	}

	else if (position >= bufBegin + FILE_BUFFER_SIZE)
	{
		fseek(f->filePtr, position, SEEK_SET);
		f->bufIt = 0;
	}

	else if (delta < 0)
		f->bufIt += delta;
	else
	{
		fgets(f->buf + f->bufIt, delta, f->filePtr);
		f->bufIt += delta;
	}

	return 0;
}

int moveFilePtr(File *f, int delta, int type)
{
	return __moveFilePtr(f, __getPos(f, delta, type));
}

int prevChar(File *f)
{
	if (moveFilePtr(f, -2, SEEK_CUR) > -1)
		return nextChar(f);
	else
		return -1;
}

int getCharAt(File *f, int delta, int type)
{
	unsigned int cpos = f->ptrPosition;
	int result = moveFilePtr(f, delta, type);
	int c = nextChar(f);
	__moveFilePtr(f, cpos);
	if (result < 0)
		return -1;
	return c;
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
