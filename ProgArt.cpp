#include "ProgArt.h"

int main ()
{
	ActivateRawMode ();

	char c = 0;
	int errcode = 0;
	while (true) 
	{
		c = 0;
		errcode = read (STDIN_FILENO, &c, 1);
		if (errcode == -1 && errno != EAGAIN)
		{
			Surrender ("read");
		}

		if (c == 'q')
		{
			break;
		}

		if (iscntrl(c))
		{
			printf ("%d\r\n", c);
		}
		else
		{
			printf ("%d %c\r\n", c, c);
		}
	}
    
	DisactivateRawMode ();

    return 0;
}

void ActivateRawMode ()
{
	struct termios raw;
	int errcode = 0;

	errcode = tcgetattr (STDIN_FILENO, &original);
	if (errcode == -1)
	{
		Surrender ("tcgetattr");
	}

	raw = original;

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

	//atexit (DisactivateRawMode);	if you want to use that, delete "DisactivateRawMode()" in main
}

void DisactivateRawMode ()
{
	int errcode = tcsetattr (STDIN_FILENO, TCSAFLUSH, &original);
	if (errcode == -1)
	{
		Surrender ("tcsetattr");
	}
}


void Surrender (char* function)
{
	perror (function);
	exit (1);
}