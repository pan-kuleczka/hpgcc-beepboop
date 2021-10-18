#include <hpgcc49.h>

const char *programInfo = "BEEPBOOP.C v0.7.6\nMADE BY PAN-KULECZKA USING HPGCC\n";

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

typedef struct
{
	char isPause;
	int a4Offset;
	float duration;
} Note;

void playNote(Note note, float durationMult)
{
	if (note.isPause)
		printf("PAUSE %.3f s\n", note.duration * durationMult);
	else
		printf("NOTE %.3f Hz %.3f s\n", getFrequency(note.a4Offset), note.duration * durationMult);

	if (note.isPause)
		sys_sleep(note.duration * durationMult * 1000);
	else
		kos_beep(getFrequency(note.a4Offset), note.duration * 1000 * durationMult, 2);
}

char isNote(char c)
{
	return (c >= 'A' && c <= 'G') || (c >= 'a' && c <= 'g') || (c == 'z') || (c == 'x');
}

int getNoteOffset(char c)
{
	switch (c)
	{
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
		return 0;
	}
}

unsigned int getNoteCount(FILE *fp)
{
	unsigned int count = 0;
	char isInBracket = 0;

	for (;;)
	{
		int c = fgetc(fp);
		if (c < 0)
			break;
		if (c == '[')
			count++, isInBracket = 1;
		else if (c == ']')
			isInBracket = 0;
		else if (!isInBracket)
			count += isNote(c);
	}

	return count;
}

void separateIntoNotes(FILE *fp, Note *noteArray, int baseShift)
{
	Note *notePtr = noteArray;
	int changeAccumulator = 0;

	char isInBracket = 0;
	char firstNoteInBracket = 1;

	int c = fgetc(fp);
	if (c < 0)
		return;

	for (;;)
	{
		if (c == '[')
		{
			isInBracket = 1;
			firstNoteInBracket = 1;
			c = fgetc(fp);
			if (c < 0)
				return;
		}
		else if (c == ']')
		{
			isInBracket = 0;
			if (!firstNoteInBracket)
				notePtr++;
			c = fgetc(fp);
			if (c < 0)
				return;
		}

		else if (isNote(c))
		{
			Note note;
			note.isPause = (c == 'z') || (c == 'x');
			note.a4Offset = getNoteOffset(c) + changeAccumulator + baseShift;
			changeAccumulator = 0;

			c = fgetc(fp);
			if (c < 0)
				return;

			while (c == ',' || c == '\'')
			{
				if (c == ',')
					note.a4Offset -= 12;
				else
					note.a4Offset += 12;
				c = fgetc(fp);
			}

			float duration = 1.0f;
			if (isdigit(c))
				duration = 0.0f;

			while (isdigit(c))
			{
				duration = 10.0f * duration + (c - '0');
				c = fgetc(fp);
			}

			if (c == '/')
			{
				unsigned int slashCount = 0;
				while (c == '/')
				{
					slashCount++;
					c = fgetc(fp);
				}

				float divisor = 0;
				while (isdigit(c))
				{
					divisor = 10.0f * divisor + (c - '0');
					c = fgetc(fp);
				}

				if (divisor == 0.0f)
					divisor = (1 << slashCount);

				duration /= divisor;
			}

			note.duration = duration;

			if (isInBracket)
			{
				if (firstNoteInBracket)
					*notePtr = note;
				else
				{
					notePtr->duration = min(notePtr->duration, note.duration);
					if (notePtr->isPause)
					{
						notePtr->isPause = note.isPause;
						notePtr->a4Offset = note.a4Offset;
					}
					else if (!note.isPause)
						notePtr->a4Offset = max(notePtr->a4Offset, note.a4Offset);
				}
			}
			else
				*notePtr = note;

			firstNoteInBracket = 0;

			if (!isInBracket)
				notePtr++;
		}

		else
		{
			if (c < 0)
				return;
			if (c == '^')
				changeAccumulator++;
			else if (c == '=')
				;
			else if (c == '_')
				changeAccumulator--;
			c = fgetc(fp);
		}
	}
}

int main()
{
	char *filename;
	SAT_STACK_ELEMENT e;
	SAT_STACK_DATA d;
	FILE *fp;

	unsigned int noteCount;
	Note *notes;

	sat_get_stack_element(1, &e);
	sat_decode_stack_element(&d, &e);
	if (d.type == SAT_DATA_TYPE_STRING)
	{
		filename = str_unquote(d.sval, '\'');
	}
	else
	{
		sat_stack_push_string("Missing filename");
		sat_push_real(1);
		return 0;
	}

	if ((fp = fopen(filename, "r")) == NULL)
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

	clear_screen();
	printf(programInfo);
	printf("Loading %s ...\n", filename);

	long double tempoModifier = 0.5;

	unsigned int musicContentsIndex = 0;
	while (!fp->eof_state)
	{
		int c0 = fgetc(fp);
		int c1 = fgetc(fp);
		musicContentsIndex += 2;

		if (c1 != ':')
		{
			fseek(fp, -1, SEEK_CUR);
			musicContentsIndex--;
			break;
		}

		if (c0 == 'Q')
		{
			musicContentsIndex++, c0 = fgetc(fp);
			float tempoA = 0, tempoB = 0, tempoC = 0;
			while (!isdigit(c0))
				musicContentsIndex++, c0 = fgetc(fp);
			while (isdigit(c0))
				tempoA = tempoA * 10.0f + c0 - '0', musicContentsIndex++, c0 = fgetc(fp);
			while (c0 != '/')
				musicContentsIndex++, c0 = fgetc(fp);
			musicContentsIndex++, c0 = fgetc(fp);
			while (!isdigit(c0))
				musicContentsIndex++, c0 = fgetc(fp);
			while (isdigit(c0))
				tempoB = tempoB * 10.0f + c0 - '0', musicContentsIndex++, c0 = fgetc(fp);
			while (c0 != '=')
				musicContentsIndex++, c0 = fgetc(fp);
			musicContentsIndex++, c0 = fgetc(fp);
			while (!isdigit(c0))
				musicContentsIndex++, c0 = fgetc(fp);
			while (isdigit(c0))
				tempoC = tempoC * 10.0f + c0 - '0', musicContentsIndex++, c0 = fgetc(fp);

			tempoModifier = 60.0f * tempoB / (tempoA * tempoC);
		}

		while (c0 != '\n' && c0 >= 0)
			musicContentsIndex++, c0 = fgetc(fp);
	}

	noteCount = getNoteCount(fp);
	fseek(fp, musicContentsIndex, SEEK_SET);

	notes = malloc(sizeof(Note) * noteCount);
	separateIntoNotes(fp, notes, 12);

	fclose(fp);

	printf("Done!\n");
	int i;

	float totalTime = 0.0f;
	for (i = 0; i < noteCount; ++i)
		totalTime += notes[i].duration * tempoModifier;

	printf("%d notes, %.1f seconds\n--------------------------------\n", noteCount, totalTime);

	sys_waitRTCTicks(12);

	for (i = 0; i < noteCount; ++i)
	{
		playNote(notes[i], tempoModifier);
		if (keyb_isAnyKeyPressed())
		{
			sat_push_real(0);
			return 0;
		}
	}

	beep();
	WAIT_CANCEL;

	sat_push_real(0);
	return 0;
}
