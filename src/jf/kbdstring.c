/*       Copyright (c) 1987 AT&T   */
/*       All Rights Reserved       */

/*       THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T   */
/*       The copyright notice above does not evidence any      */
/*       actual or intended publication of such source code.   */


static char _2Vsccsid[]="@(#)kbdstring.c	1.1.1.5 88/02/24 13:44:09";

/* includes */
#include <dmd.h>
#ifdef DMD630
#	include <5620.h>
#endif /* DMD630 */
#include <font.h>

/* defines */
#ifdef DMD630
#	define SIGNED
#else
#	define SIGNED(ch)	((ch) & 0x80 ? (ch) | 0xffffff00 : (ch))
#endif /* DMD630 */

#define echo(ich,p)	bitblt(defont.bits,\
				Rect(ich->x,0,(ich+1)->x,defont.height),\
				&display,Pt(p.x+SIGNED(ich->left),p.y),F_XOR)

#define kbdcurs(p)	rectf(&display,Rect(p.x,p.y,p.x+1,p.y+defont.height),F_XOR)

/* procedures */


kbdstring (str, nchmax, p)	/* read string from keyboard with echo at p */
	register char *str;
	int nchmax;
	Point p;
{
	register int kbd, nchars=0;
	register Fontchar *ich;

	if (nchmax-- == 0)
		return;

	*str = '\0';
	kbdcurs(p);
	for (;;) {
		wait(KBD);
		kbdcurs(p);
		switch (kbd=kbdchar()) {
			case '\0':
				break;
			case '\r':
			case '\n':
				return nchars;
			case '\b':
				if (nchars <= 0)
					break;
				kbd = *--str;
				*str='\0';
				nchars--;
				ich=defont.info+kbd;
				p.x -= ich->width;
				echo(ich,p);
				break;
			default:
				if (nchars >= nchmax)
					break;
				*str++ = kbd;
				*str='\0';
				nchars++;
				ich=defont.info+kbd;
				echo(ich,p);
				p.x += ich->width;
		}
		kbdcurs(p);
	}
}

atoi(str)
register char *str;
{
	register n = 0, s = 1;
	while (*str == ' ' || *str == '\t')
		str++;
	if (*str == '-') {
		str++; s = -1;
	} else if (*str == '+')
		str++;
	while (*str >= '0' && *str <= '9')
		n = 10*n + *str++ - '0';
	return s*n;
}

char *
itoa(m)
int m;
{
	static char result[8]; register char *str = result + sizeof result;
	register int n = m;
	if (n < 0) n = -n;
	*--str = '\0';
	do {
		*--str = n%10 + '0';
		n /= 10;
	} while (n > 0);
	if (m < 0) *--str = '-';
	return str;
}
