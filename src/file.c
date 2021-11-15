#include "file.h"
#include <hpgcc49.h>

void file_init(File *f)
{
	f->fileSize = 0;
	while (fgetc(f->filePtr) > -1)
		++f->fileSize;
	rewind(f->filePtr);

	f->ptrPosition = 0;
	f->bufIt = 0;
}

int file_tell(File *f)
{
	return f->ptrPosition;
}

int file_getc(File *f)
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

int __file_seek(File *f, int position)
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

int file_seek(File *f, int delta, int type)
{
	return __file_seek(f, __getPos(f, delta, type));
}

int file_prevc(File *f)
{
	if (file_seek(f, -2, SEEK_CUR) > -1)
		return file_getc(f);
	else
		return -1;
}

int file_getcAt(File *f, int delta, int type)
{
	unsigned int cpos = f->ptrPosition;
	int result = file_seek(f, delta, type);
	int c = file_getc(f);
	__file_seek(f, cpos);
	if (result < 0)
		return -1;
	return c;
}

int file_skipTo(File *f, int (*stopFunction)(File *))
{
	while (stopFunction(f) == 0)
		if (file_tell(f) == f->fileSize)
			return -1;
	return 0;
}

int file_skipToChar(File *f, char c)
{
	while (file_getc(f) != c)
		if (file_tell(f) == f->fileSize)
			return -1;
	return 0;
}

int file_skipToNewline(File *f)
{
	return file_skipToChar(f, '\n');
}
