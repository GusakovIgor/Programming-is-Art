#include "ProgArt.h"


/*** defines ***/

#define PROGART_VERSION "0.0.1"

#define CTRL_KEY(symbol) ((symbol) & 0x1f)

#define BUFF_INIT {NULL, 0}

//----------------------


int main (int argc, char* argv[])
{
	ActivateRawMode ();
	InitEditor ();
	if (argc > 1)
	{
		EditorFileOpen (argv[1]);
	}

	while (true) 
	{
		EditorRefreshScreen ();
		EditorProcessingKeypress ();
	}

    return 0;
}




/*** Working with Terminal ***/

int EditorReadKey ()
{
	int c = 0;
	int nread = 0;

	nread = read (STDIN_FILENO, &c, 1);
	while (nread != 1)
	{
		if (nread == -1 && errno != EAGAIN)
		{
			Surrender ("read");
		}

		nread = read (STDIN_FILENO, &c, 1);
	}

	if (c == '\x1b')
	{
		char seq[3] = {};

		if (read (STDIN_FILENO, seq, 1) != 1)
		{
			return '\x1b';
		}
		if (read (STDIN_FILENO, seq + 1, 1) != 1)
		{
			return '\x1b';
		}

		if (seq[0] == '[')
		{

			if ('0' <= seq[1] && seq[1] <= '9')
			{
				return NumericEscSequence (seq);
			}
			else
			{
				return LetterEscSequence (seq[1]);
			}
		}
		
		if (seq[0] == 'O')
		{
			return LetterEscSequence (seq[1]);
		}
	}

	return c;
}

int NumericEscSequence (char seq[])
{
	if (read (STDIN_FILENO, seq + 2, 1) != 1)
	{
		return '\x1b';
	}
	if (seq[2] == '~')
	{
		switch (seq[1])
		{
			case '1': return HOME_KEY;

			case '3': return DEL_KEY;

			case '4': return END_KEY;

			case '5': return PAGE_UP;

			case '6': return PAGE_DOWN;

			case '7': return HOME_KEY;

			case '8': return END_KEY;
		}
	}
}

int LetterEscSequence (int letter)
{
	switch (letter)
	{
		case 'A':	return ARROW_UP;

		case 'B':	return ARROW_DOWN;

		case 'C':	return ARROW_RIGHT;

		case 'D':	return ARROW_LEFT;

		case 'F':	return END_KEY;

		case 'H':	return HOME_KEY;
	}
}

void ActivateRawMode ()
{
	struct termios raw;
	int errcode = 0;

	errcode = tcgetattr (STDIN_FILENO, &State.original);
	if (errcode == -1)
	{
		Surrender ("tcgetattr");
	}

	raw = State.original;

	raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |=  (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

	raw.c_cc[VMIN]  = 0;
	raw.c_cc[VTIME] = 1;

	errcode = tcsetattr (STDIN_FILENO, TCSAFLUSH, &raw);
	if (errcode == -1)
	{
		Surrender ("tcsetattr");
	}

	atexit (DisactivateRawMode);
}

void DisactivateRawMode ()
{
	int errcode = tcsetattr (STDIN_FILENO, TCSAFLUSH, &State.original);
	if (errcode == -1)
	{
		Surrender ("tcsetattr");
	}
}


int GetWinSize (int* columns, int* rows)
{
	struct winsize ws = {};

	int errcode = ioctl (STDOUT_FILENO, TIOCGWINSZ, &ws);
	if (errcode == -1 || ws.ws_col == 0)
	{
		if (write (STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
		{
			return -1;
		}

		return GetCursorPos (columns, rows);
	}
	else
	{
		*columns = ws.ws_col;
		*rows	 = ws.ws_row;

		return 0;
	}
}

int GetCursorPos (int* columns, int* rows)
{
	if (write (STDOUT_FILENO, "\x1b[6n", 4) != 4)
	{
		return -1;
	}

	char buff[32] = {};
	size_t i = 0;

	while (i < sizeof (buff))
	{
		if (read (STDIN_FILENO, &buff[i], 1) != 1)
		{
			break;
		}
		if (buff[i] == 'R')
		{
			break;
		}

		i++;
	}
	buff[i] = '\0';

	if (buff[0] != '\x1b' || buff[1] != '[')
	{
		return -1;
	}
	if (sscanf (buff + 2, "%d;%d", rows, columns) != 2)
	{
		return -1;
	}

	return 0;
}

//----------------------



/*** File I/O ***/

void EditorFileOpen (char* filename)
{
	FILE* input = fopen (filename, "r");
	if (!input)
	{
		Surrender ("fopen");
	}

	char* line = NULL;
	size_t line_cap = 0;
	size_t length = getline (&line, &line_cap, input);

	while (length != -1)
	{
		while (length > 0 && (line[length - 1] == '\n' || line[length - 1] == '\r'))
		{
			length--;
		}

		AppendRow (line, length);

		length = getline (&line, &line_cap, input);
	}

	free (line);
	fclose (input);
}

//----------------------



/*** Row Operations ***/

void AppendRow (char* line, size_t length)
{
	State.rows = (dyn_string*) realloc (State.rows, sizeof(dyn_string) * (State.numrows + 1));		// I think takes a lot of time. Check this out later!

	int new_row = State.numrows;

	State.rows[new_row].len = length;
	State.rows[new_row].string = (char*) calloc (length + 1, sizeof (char));

	memcpy (State.rows[new_row].string, line, length);
	State.rows[new_row].string[length] = '\0';
	State.numrows++;
}

//----------------------



/*** Buffer ***/

void AddToBuffer (dyn_string* buff, const char* string, int length)
{
	char* new_buff = (char*) realloc (buff->string, buff->len + length);

	if (new_buff == NULL)
		return;

	memcpy (new_buff + buff->len, string, length);
	
	buff->string = new_buff;
	buff->len   += length;
}

void FreeBuffer (dyn_string* buff)
{
	free (buff->string);
}

//----------------------



/*** Input ***/

void EditorProcessingKeypress ()
{
	int c = EditorReadKey ();

	switch (c)
	{
		case CTRL_KEY('q'):		write (STDOUT_FILENO, "\x1b[2J", 4);
								write (STDOUT_FILENO, "\x1b[H",  3);
								exit (0);
								break;

		case ARROW_UP:
		case ARROW_DOWN:
		case ARROW_RIGHT:
		case ARROW_LEFT:
			EditorMoveCursor (c);
			break;

		case PAGE_UP:
		case PAGE_DOWN:
		case HOME_KEY:
		case END_KEY:
			EditorSpecialKeys (c);
			break;
	}
}

void EditorMoveCursor (int key)
{
	switch (key)
	{
		case ARROW_UP:		if (State.cy != 0)
							{
								State.cy--;
							}
							break;

		case ARROW_DOWN:	if (State.cy != State.screenrows - 1)
							{
								State.cy++;
							}
							break;

		case ARROW_RIGHT:	if (State.cx != State.screencols - 1)
							{
								State.cx++;
							}
							break;

		case ARROW_LEFT:	if (State.cx != 0)
							{
								State.cx--;
							}
							break;

	}
}

void EditorSpecialKeys (int key)
{
	switch (key)
	{
		case PAGE_UP:		State.cy = 0;
							break;

		case PAGE_DOWN:		State.cy = State.screenrows - 1;
							break;

		case HOME_KEY:		State.cx = 0;
							break;

		case END_KEY:		State.cx = State.screencols - 1;
							break;
	}
}


//----------------------



/*** Output ***/

void AddWelcome (dyn_string* buff)
{
	char welcome [80];
	int length = snprintf (welcome, length, "ProgArt editor (version %s)", PROGART_VERSION);
	
	length = min (length, State.screencols);

	int indent = (State.screencols - length) / 2;

	if (indent)
	{
		AddToBuffer (buff, "~", 1);
		indent--;
	}

	while (indent > 0)
	{
		AddToBuffer (buff, " ", 1);
		indent--;
	}

	AddToBuffer (buff, welcome, length);
}

void EditorDrawRaws (dyn_string* buff)
{
	int y = 0;
	for (y = 0; y < State.screenrows; y++)
	{
		if (y >= State.numrows)
		{
			if (State.numrows == 0 && y == State.screenrows / 3)
			{
				AddWelcome (buff);
			}
			else
			{
				AddToBuffer (buff, "~", 1);
			}
		}
		else
		{
			int length = min (State.rows[y].len, State.screencols);

			AddToBuffer (buff, State.rows[y].string, length);
		}

		AddToBuffer (buff, "\x1b[K", 3);

		if (y < State.screenrows - 1)
		{
			AddToBuffer (buff, "\r\n", 2);
		}
	}
}

void EditorRefreshScreen ()
{
	dyn_string buff = BUFF_INIT;

	AddToBuffer (&buff, "\x1b[?25l", 6);
	AddToBuffer (&buff, "\x1b[H",  3);

	EditorDrawRaws (&buff);

	char esc_seq [30] = {};
	snprintf (esc_seq, sizeof(esc_seq), "\x1b[%d;%dH", State.cy + 1, State.cx + 1);
	AddToBuffer (&buff, esc_seq, strlen(esc_seq));

	AddToBuffer (&buff, "\x1b[?25h", 6);

	write (STDOUT_FILENO, buff.string, buff.len);
	FreeBuffer (&buff);
}

//----------------------



/*** Initialize ***/

void InitEditor ()
{
	State.cx = 0;
	State.cy = 0;

	int errcode = GetWinSize (&State.screencols, &State.screenrows);
	if (errcode == -1)
	{
		Surrender ("GetWinSize");
	}

	State.numrows = 0;
	State.rows = NULL;
}

//----------------------



/*** Error Handling ***/

void Surrender (char* function)
{
	write (STDOUT_FILENO, "\x1b[2J", 4);
	write (STDOUT_FILENO, "\x1b[H",  3);

	perror (function);
	exit (1);
}

//----------------------


/*** Help ***/

int min (int first, int second)
{
	return (first <= second) ? first : second;
}

//----------------------