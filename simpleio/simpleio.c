/*-------------------------------------------------------------------------
 * simpleio.c:  A simple IO library, derived from code in the multiboot spec.
 */

#include "simpleio.h"

#define COLUMNS         80
#define LINES           25
#define ATTRIBUTE       7
#define VIDEO           0xB8000

static int xpos = 0;
static int ypos = 0;

typedef unsigned char single[2];
typedef single        row[COLUMNS];
typedef row           screen[LINES];

static screen* video = (screen*)VIDEO;

static int top=0,  bottom=LINES;
static int left=0, right=COLUMNS;
static int attr=ATTRIBUTE;

/*-------------------------------------------------------------------------
 * Set a new start address for the video display.  If the video RAM is
 * mapped in kernel space, then the kernel should use that address for
 * output, avoiding the need for video RAM mappings in individual user
 * address spaces.
 */
void setVideo(unsigned v) {
  video = (screen*)v;
}

/*-------------------------------------------------------------------------
 * Set portion of video display to use for output.  Inputs specify the
 * (zero-based) coordinate of the top-left character and the height and
 * width of the display window.
 */
void setWindow(int t, int h, int l, int w) {
  if (0<=t && 0<h && (t+h)<=LINES && 0<=l && 0<w && (l+w)<=COLUMNS) {
    top    = t;
    bottom = t+h;
    left   = l;
    right  = l+w;
  }
}

/*-------------------------------------------------------------------------
 * Set the video color attribute:
 */
void setAttr(int a) {
  attr = a;
}

/*-------------------------------------------------------------------------
 * Clear the output window.
 */
void cls(void) {
    for (int i=top; i<bottom; ++i) {
      for (int j=left; j<right; ++j) {
        (*video)[i][j][0] = ' ';
        (*video)[i][j][1] = attr;
      }
    }
    ypos = top;
    xpos = left;
}

/*-------------------------------------------------------------------------
 * Output a single character.
 */
void putchar(int c) {
    extern void serial_putc(int);
    serial_putc(c);
    if (c!='\n' && c!='\r') {
        (*video)[ypos][xpos][0] = c & 0xFF;
        (*video)[ypos][xpos][1] = attr;
        if (++xpos < right) {
            return;
        }
    }

    xpos = left;            // Perform a newline
    if (++ypos >= bottom) { // scroll up top lines of screen ... 
        ypos = bottom-1;
        for (int i=top; i<ypos; ++i) {
          for (int j=left; j<right; ++j) {
            (*video)[i][j][0] = (*video)[i+1][j][0];
            (*video)[i][j][1] = (*video)[i+1][j][1];
          }
        }
        for (int j=left; j<right; ++j) { // fill in new blank line
          (*video)[ypos][j][0] = ' ';
          (*video)[ypos][j][1] = attr;
        }
    }
}

/*-------------------------------------------------------------------------
 * Output a zero-terminated string.
 */
void puts(char *msg) {
  while (*msg) {
    putchar(*msg++);
  }
}

/*-------------------------------------------------------------------------
 * Convert an integer into a string.  If base == 'x', then the string
 * is printed in hex; if base == 'd', then the string is printed as a
 * signed integer; otherwise an unsigned integer is assumed.
 */
static void itoa(char* buf, int base, long d) {
    char* p  = buf;
    char* p1;
    char* p2;
    unsigned long ud = d;
    int divisor = 10;

    if (base=='d' && d<0) {
        *p++ = '-';
        buf++;
        ud = -d;
    } else if (base=='x') {
        divisor = 16;
    }

    do {
        int remainder = ud % divisor;
        *p++ = ((remainder < 10) ? '0' : ('a' - 10)) + remainder;
    } while (ud /= divisor);

    *p = 0;

    p1 = buf;
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1      = *p2;
        *p2      = tmp;
        p1++;
        p2--;
    }
}

/*-------------------------------------------------------------------------
 * Simple version of printf that displays a string with (optional)
 * embedded placeholders for numeric and/or string data.
 */
void printf(const char *format, ...) {
    char** arg = (char**) &format;
    int    c;
    char   buf[20];

    arg++;
    while ((c = *format++) != 0) {
        if (c != '%') {
            putchar(c);
        } else {
            char* p;
            int padChar  = ' ';
            int padWidth = 0;
            int longArg  = 0;

            c = *format++;
            if (c == '0') {
                padChar = '0';
                c = *format++;
            }

            if (c>='0' && c<='9') {
              do {
                padWidth = 10 * padWidth + (c - '0');
                c = *format++;
              } while (c>='0' && c<='9');
            }

            if (c == 'l') {
                longArg = 1;
                c = *format++;
            }

            switch (c) {
                case 'd' :
                case 'u' :
                case 'x' :
                    if (longArg) {
                      itoa(buf, c, *((long*)arg++));
                    } else {
                      itoa(buf, c, (long)(*((int*)arg++)));
                    }
                    for (p = buf; *p; p++) {
                      padWidth--;
                    }
                    while (0<padWidth--) {
                        putchar(padChar);
                    }
                    for (p = buf; *p; p++) {
                        putchar(*p);
                    }
                    break;

		case 'c' :
		    putchar(*((char*) arg++));
		    break;

                case 's' :
                    p = *arg++;
                    if (!p) {
                        p = "(null)";
                    }
                    while (*p) {
                        putchar(*p++);
                    }
                    break;

                default :
                    putchar(*((int *) arg++));
                    break;
            }
        }
    }
}

/*-----------------------------------------------------------------------*/
