#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>

struct dyn_string {
	char*  string;
	size_t len;
};

struct EditorConfig {
	int cx;
	int cy;
	int screencols;
	int screenrows;
	int numrows;
	dyn_string* rows;
	struct termios original;
};


enum arrows { ARROW_UP    = -1,
			  ARROW_DOWN  = -2,
			  ARROW_RIGHT = -3,
			  ARROW_LEFT  = -4,
			  PAGE_UP   = -5,
			  PAGE_DOWN = -6,
			  HOME_KEY  = -7,
			  END_KEY   = -8,
			  DEL_KEY   = -9
};


EditorConfig State = {};


/*** Working with terminal ***/

int  EditorReadKey ();
int  NumericEscSequence (char seq[]);
int  LetterEscSequence  (int letter);

void ActivateRawMode ();
void DisactivateRawMode ();

int  GetCursorPos (int* columns, int* rows);
int  GetWinSize   (int* columns, int* rows);

//-----------------



/*** File I/O ***/

void EditorFileOpen (char* filename);

//----------------------



/*** Row Operations ***/

void AppendRow (char* line, size_t length);

//----------------------



/*** Buffer ***/

void AddToBuffer (dyn_string* buff, const char* string, int length);

void FreeBuffer  (dyn_string* buff);

//----------------------



/*** Input ***/

void EditorProcessingKeypress ();

void EditorMoveCursor  (int key);
void EditorSpecialKeys (int key);

//-----------------



/*** Output ***/

void AddWelcome (dyn_string* buff);

void EditorDrawRaws (dyn_string* buff);

void EditorRefreshScreen ();

//-----------------



/*** Initialize ***/

void InitEditor ();

//-----------------



/*** Error handle ***/

void Surrender (char* function);

//-----------------



/*** Help ***/

int min (int first, int second);

//----------------------