/*
    Copyright 2014-2018 Mark P Jones, Portland State University

    This file is part of CEMLaBS/LLP Demos and Lab Exercises.

    CEMLaBS/LLP Demos and Lab Exercises is free software: you can
    redistribute it and/or modify it under the terms of the GNU General
    Public License as published by the Free Software Foundation, either
    version 3 of the License, or (at your option) any later version.

    CEMLaBS/LLP Demos and Lab Exercises is distributed in the hope that
    it will be useful, but WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
    PURPOSE.  See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CEMLaBS/LLP Demos and Lab Exercises.  If not, see
    <https://www.gnu.org/licenses/>.
*/
/* A simple program that we will run in user mode.
 */
#include "simpleio.h"

extern void kputc(unsigned);

void kputs(char* s) {
  while (*s) {
    kputc(*s++);
  }
}

extern void serial_putc(int c);

void cmain() {
  int i;
  kputs("User process begins ...\n");
  setWindow(1, 23, 51, 28);   // user process on right hand side
  cls();
  puts("in user code\n");
  for (i=0; i<4; i++) {
    kputs("hello, kernel console\n");
    puts("hello, user console\n");
  }
  puts("\n\nUser code does not return\n");
  for (;;) { /* Don't return! */
  }
  puts("This message won't appear!\n");
}
