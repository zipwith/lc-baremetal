#ifndef SIMPLEIO_H
#define SIMPLEIO_H

#if defined(__cplusplus)
extern "C" {
#endif

extern void setVideo(unsigned);
extern void setWindow(int t, int h, int l, int w);
extern void setAttr(int a);
extern void cls(void);
extern void putchar(int c);
extern void printf(const char *format, ...);
extern int  getc(void);

#if defined(__cplusplus)
}
#endif

#endif
