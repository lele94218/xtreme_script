#include "c.h"

/* bsize < 0: no character read or has input error;
 * bsize = 0: reach end of input;
 * bsize > 0: read bsize characters.
 */
static int bsize;
static unsigned char buffer[MAXLINE + 1 + BUFSIZE + 1];
unsigned char *cp;    /* current input character */
char *file;           /* current input file name */
char *firstfile;      /* first input file */
unsigned char *limit; /* points to last character + 1 */
char *line;           /* current line */
int lineno;           /* line number of current line */

void input_init()
{
    limit = cp = &buffer[MAXLINE + 1];
    bsize = -1;
    lineno = 0;
    file = NULL;
    /* refill buffer */
	fillbuf();
	if (cp >= limit)
		cp = limit;
    nextline();
}

void nextline()
{
    do
    {
        if (cp >= limit)
        {
			fillbuf();
			if (cp >= limit)
				cp = limit;
            /* refill buffer */
            if (cp == limit) return;
        }
        else
        {
            lineno ++;
        }
        for (line = (char *)cp; *cp == ' ' || *cp == '\t'; cp ++);
    } while (*cp == '\n' && cp == limit);
    
    if (*cp == '#')
    {
        resynch();
        nextline();
    }
}

void fillbuf()
{
    if (bsize == 0)
        return;
    if (cp >= limit)
        cp = &buffer[MAXLINE + 1];
    else
    {
        /* move the tail portion */
        int n = limit - cp;
        unsigned char *s = &buffer[MAXLINE + 1] - n;
        line = (char *)s - ((char *)cp - line);
        while (cp < limit)
            *s++ = *cp++;
        cp = &buffer[MAXLINE + 1] - n;
    }
    
	bsize = fread(&buffer[MAXLINE+1], 1, BUFSIZE, stdin);
    if (bsize < 0)
    {
        printf("read error\n");
        exit(1);
    }
    limit = &buffer[MAXLINE + 1 + bsize];
    *limit = '\n';
}