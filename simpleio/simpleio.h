#ifndef SIMPLEIO_H
#define SIMPLEIO_H
extern void setVideo(unsigned);
extern void setWindow(int t, int h, int l, int w);
extern void setAttr(int a);
extern void cls(void);
extern void putchar(int c);
extern void puts(char *msg);
extern void printf(const char *format, ...);
#endif
