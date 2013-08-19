#include <stdio.h>

#pragma import(__use_no_semihosting_swi)


//In arch_serial.c
extern int sendchar(int ch);
extern int getkey(void);

/*--------------------------- fputc --------------------------------------*/

int fputc(int ch, FILE *f)
{

	return (sendchar(ch));
}


/*--------------------------- ferror --------------------------------------*/

int ferror(FILE *f)
{

	/* Your implementation of ferror */
	return EOF;
}


/*--------------------------- _ttywrch --------------------------------------*/

void _ttywrch (int ch)
{

#ifdef STDIO
	sendchar(ch);
#endif
}


/*--------------------------- _sys_exit -------------------------------------*/

void _sys_exit (int return_code)
{

	/* Endless loop. */
	while (1);
}



