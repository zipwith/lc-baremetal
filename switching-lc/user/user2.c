/* A simple program that we will run in user mode.
 */
#include "simpleio.h"

extern void kputc(unsigned);
extern void yield(void);

void kputs(char* s) {
  while (*s) {
    kputc(*s++);
  }
}

void cmain() {
  int i;
  setWindow(13, 11, 47, 32);   // user process on right hand side
  cls();
  puts("in user2 code\n");
  for (i=0; i<400; i++) {
    kputs("hello, kernel console2\n");
    puts("hello, user console2\n");
//    yield();
  }
  unsigned* flagAddr = (unsigned*)0x402744;
  printf("flagAddr = 0x%x\n", flagAddr);
  *flagAddr = 1234;
  puts("\n\nUser2 code does not return\n");
  for (;;) { /* Don't return! */
  }
  puts("This message won't appear!\n");
}
