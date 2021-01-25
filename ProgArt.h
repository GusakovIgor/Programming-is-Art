#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

struct termios original;

// Raw mode on/off
void ActivateRawMode ();
void DisactivateRawMode ();
//-----------------

// Error handle
void Surrender (char* function);
//-----------------