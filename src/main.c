#include <hpgcc49.h>
#include "file.h"

const char *programInfo = "BEEPBOOP.C v0.8\nMADE BY PAN-KULECZKA USING HPGCC\n";

struct Note
{
	char a4Offset;
	float durationBeats;
};

typedef struct Note Note;

const float A4_FREQ = 440;
const float NOTE_FREQ_INTERVAL = 1.059463;

float getFrequency(int a4Offset)
{
	float multiplier = 1.0;

	unsigned int absOffset = abs(a4Offset);

	unsigned int octaveShift = absOffset / 12;
	multiplier *= (1 << octaveShift);
	absOffset -= octaveShift * 12;
	multiplier *= pow(NOTE_FREQ_INTERVAL, absOffset);

	if (a4Offset > 0)
		return A4_FREQ * multiplier;
	else
		return A4_FREQ / multiplier;
}

unsigned long long getTicks(Note note)
{
	return (unsigned long long)((double)1e9 * note.durationBeats);
}

#define OFFSET_NOT_A_NOTE 0x7f
#define OFFSET_PAUSE 0x80

char getNoteA4Offset(char c)
{
	switch (c)
	{
	case 'z':
		return OFFSET_PAUSE;
	case 'x':
		return OFFSET_PAUSE;
	case 'C':
		return -21;
	case 'D':
		return -19;
	case 'E':
		return -17;
	case 'F':
		return -16;
	case 'G':
		return -14;
	case 'A':
		return -12;
	case 'B':
		return -10;
	case 'c':
		return -9;
	case 'd':
		return -7;
	case 'e':
		return -5;
	case 'f':
		return -4;
	case 'g':
		return -2;
	case 'a':
		return 0;
	case 'b':
		return 2;
	default:
		return OFFSET_NOT_A_NOTE;
	}
}

int moveToNextCurrentLineChar(File *f, char searched)
{
	int c = nextFileChar(f);
	while (c != '%' && c != '\n' && c > -1 && c != searched)
		c = nextFileChar(f);
	if (c == '%' || c == '\n' || c < 0)
		return -1;
	moveFilePtr(f, -1, SEEK_CUR);
	return 0;
}

int moveToNextCurrentLineDigit(File *f)
{
	int c = nextFileChar(f);
	while (c != '%' && c != '\n' && c > -1 && !isdigit(c))
		c = nextFileChar(f);
	if (c == '%' || c == '\n' || c < 0)
		return -1;
	moveFilePtr(f, -1, SEEK_CUR);
	return 0;
}

int nextMusicChar(File *f)
{
	int c;
	for (;;)
	{
		c = nextFileChar(f);
		c2 = nextFileChar(f);

		if (c < 0)
			break;

		if (c == '%')
			skipToNextLine(f);
		else if (fileCharAt(f, filePtrTell(f)) == ':')
		{
			// may be the beginning of a tag
			if (filePtrTell(f) == 1 || fileCharAt(f, filePtrTell(f) - 2) == '\n')
				skipToNextLine(f);
			else
				break;
		}
		else
			break;
	}

	return c;
}

int moveToNextTag(File *f)
{
	int c;
	for (;;)
	{
		c = nextFileChar(f);
		if (c < 0)
			return -1;

		if (c == '%')
			skipToNextLine(f);
		else if (c == ':')
		{
			if (filePtrTell(f) == 2)
			{
				moveFilePtr(f, 0, SEEK_SET);
				return 0;
			}
			else if (filePtrTell(f) > 2)
			{
				int c2 = fileCharAt(f, filePtrTell(f) - 3);
				if (c2 == '\n')
				{
					moveFilePtr(f, -2, SEEK_CUR);
					return 0;
				}
			}
		}
	}
}

unsigned int getNoteCount(File *f)
{
	moveFilePtr(f, 0, SEEK_SET);
	unsigned int count = 0;

	for (;;)
	{
		int c = nextMusicChar(f);
		if (c < 0)
			break;
		if (getNoteA4Offset(c) != OFFSET_NOT_A_NOTE)
			++count;
	}

	return count;
}

int parse(File *f, char *groupArray, Note *noteArray, int baseNoteShift)
{
	moveFilePtr(f, 0, SEEK_SET);

	Note *nextFreeNote = noteArray;
	char *nextFreeGroupField = groupArray;

	char isInBracket = 0;
	int shiftAccumulator = 0;

	int c = nextMusicChar(f);

	for (;;)
	{
		if (c < 0)
			return 0;

		if (getNoteA4Offset(c) != OFFSET_NOT_A_NOTE)
		{
			// c is a note/pause
			int a4offset = getNoteA4Offset(c);
			shiftAccumulator += baseNoteShift;

			for (;;)
			{
				c = nextMusicChar(f);
				if (c == ',')
					shiftAccumulator -= 12;
				else if (c == '\'')
					shiftAccumulator += 12;
				else
					break;
			}

			float duration = 1.0f;

			if (isdigit(c))
				duration = 0.0f;

			while (isdigit(c))
			{
				duration = 10.0f * duration + (c - '0');
				c = nextMusicChar(f);
			}

			if (c == '/')
			{
				unsigned int slashCount = 0;
				while (c == '/')
				{
					slashCount++;
					c = nextMusicChar(f);
				}

				float divisor = 0;
				while (isdigit(c))
				{
					divisor = 10.0f * divisor + (c - '0');
					c = nextMusicChar(f);
				}

				if (divisor == 0.0f)
					divisor = (1 << slashCount);

				duration /= divisor;
			}

			Note note;

			note.a4Offset = a4offset;

			if (note.a4Offset != OFFSET_PAUSE)
				note.a4Offset += shiftAccumulator;

			shiftAccumulator = 0;

			note.durationBeats = duration;

			*nextFreeNote = note;
			nextFreeNote++;

			*nextFreeGroupField = isInBracket;
			nextFreeGroupField++;
		}
		else
		{
			if (c == '[')
				isInBracket = 1;
			else if (c == ']')
				*(nextFreeGroupField - 1) = 0;

			else if (c == '^')
				shiftAccumulator++;

			else if (c == '=')
				;

			else if (c == '_')
				shiftAccumulator--;

			c = nextMusicChar(f);
		}
	}

	return 0;
}

float getBeatLength(File *f)
{
	int c;
	unsigned int tA, tB, tC; // three parts of tA/tB=tC

	if (moveToNextCurrentLineDigit(f) < 0)
		return -1;

	tA = 0;
	c = nextFileChar(f);
	while (isdigit(c))
		tA = tA * 10 + (c - '0'), c = nextFileChar(f);
	moveFilePtr(f, -1, SEEK_CUR);

	if (moveToNextCurrentLineChar(f, '/') < 0)
	{
		// file uses deprecated 'Q: <X>' notation
		return 60.0f / tA;
	}

	if (moveToNextCurrentLineDigit(f) < 0)
		return -1;

	tB = 0;
	c = nextFileChar(f);
	while (isdigit(c))
		tB = tB * 10 + (c - '0'), c = nextFileChar(f);
	moveFilePtr(f, -1, SEEK_CUR);

	if (moveToNextCurrentLineChar(f, '=') < 0)
		return -1;
	if (moveToNextCurrentLineDigit(f) < 0)
		return -1;

	tC = 0;
	c = nextFileChar(f);
	while (isdigit(c))
		tC = tC * 10 + (c - '0'), c = nextFileChar(f);
	moveFilePtr(f, -1, SEEK_CUR);

	return (tB * 60.0f) / (tA * tC);
}

void playNote(Note note, float bps)
{
	if (note.a4Offset == OFFSET_PAUSE || note.a4Offset == OFFSET_NOT_A_NOTE)
		return;
	kos_beep(getFrequency(note.a4Offset), note.durationBeats / bps * 1000, 2);
}

int main()
{
	char *filename;
	SAT_STACK_ELEMENT e;
	SAT_STACK_DATA d;
	File f;

	unsigned int noteCount;
	char *groupFlags;
	Note *notes;

	sat_get_stack_element(1, &e);
	sat_decode_stack_element(&d, &e);

	if (d.type == SAT_DATA_TYPE_STRING)
		filename = str_unquote(d.sval, '\'');
	else
	{
		sat_stack_push_string("Missing filename to open");
		sat_push_real(1);
		return 0;
	}

	if ((f.filePtr = fopen(filename, "r")) == NULL)
	{
		char errormsg[30];
		char fname[13];

		strncpy(fname, filename, 12);
		fname[12] = '\0';
		sprintf(errormsg, "Cannot open: %s", fname);

		sat_stack_push_string(errormsg);
		sat_push_real(1);
		return 0;
	}
	f.ptrPosition = 0;

	clear_screen();
	printf(programInfo);
	printf("Loading %s ...\n", filename);

	// Loading song tags

	float beatLength = -1;
	char songTitle[30];
	songTitle[0] = 0;

	while (moveToNextTag(&f) == 0)
	{
		int tagChar = nextFileChar(&f); // letter before ':'
		nextFileChar(&f);				// skipping the ':'
		int c = nextFileChar(&f);

		switch (tagChar)
		{
		case 'Q':
			beatLength = getBeatLength(&f);
			if (beatLength < 0)
			{
				// bad tag
				sat_stack_push_string("Bad \"Q:\" (tempo) tag");
				sat_push_real(1);
				return 0;
			}
			break;

		case 'T':
			while (c == ' ')
				c = nextFileChar(&f);

			unsigned int lenCtr = 0;
			while (lenCtr < 29 && c != '\n' && c != '%' && c > -1)
			{
				songTitle[lenCtr] = c;
				lenCtr++;
				c = nextFileChar(&f);
			}
			songTitle[lenCtr] = 0;
			break;

		default:
			break;
		}
	}

	printf("Loaded file tags ...\n");

	if (beatLength < 0)
	{
		// no tempo tag
		sat_stack_push_string("Missing \"Q:\" (tempo) tag");
		sat_push_real(1);
		return 0;
	}

	moveFilePtr(&f, 0, SEEK_SET);
	noteCount = getNoteCount(&f);
	groupFlags = malloc(sizeof(char) * noteCount);
	notes = malloc(sizeof(Note) * noteCount);

	printf("%d\n", noteCount);

	moveFilePtr(&f, 0, SEEK_SET);
	parse(&f, groupFlags, notes, -12);

	fclose(f.filePtr);

	printf("Done!\n");
	if (strlen(songTitle))
		printf("Title: %s\n", songTitle);

	sys_waitRTCTicks(12);

	// Play notes

	sys_timer_t timer = SYS_TIMER_INITIALIZER_NUM(3);
	sys_updateTimer(&timer);

	//unsigned int halfPeriodTicks = 5e8 / tone;

	/*for (;;)
	{
	}

	unsigned long long nextSwitchTick = timer.current + halfPeriodTicks;
	unsigned long long finishTick = timer.current + (duration * (unsigned int)1e9);

	while (timer.current < finishTick)
	{
		*GPBDAT = 0x4; //beeper on
		while (timer.current < nextSwitchTick)
			sys_updateTimer(&timer);
		nextSwitchTick += halfPeriodTicks;

		*GPBDAT = 0; //beeper off
		while (timer.current < nextSwitchTick)
			sys_updateTimer(&timer);
		nextSwitchTick += halfPeriodTicks;
	}*/

	/*for (i = 0; i < noteCount; ++i)
	{
		playNote(notes[i], tempoModifier);
		if (keyb_isAnyKeyPressed())
		{
			sat_push_real(0);
			return 0;
		}
	}*/

	printf("FINISHED.\n");
	WAIT_CANCEL;
	sat_push_real(0);
	return 0;
}
