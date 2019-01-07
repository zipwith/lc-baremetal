
#define NOENTRY  0xffffffff /* Used to signal a missing entry point      */

/* ------------------------------------------------------------------------
 * A BOOTDATA section starts with four pointers:
 * - unsigned* hdrs   points to an array of header information
 * - unsigned* mmap   points to an array of memory map information
 * - char*     cmd    loader command line string
 * - char*     str    boot module command line string
 * --------------------------------------------------------------------- */

struct BootData {
  unsigned* headers;
  unsigned* mmap;
  char*     cmdline;
  char*     imgline;
};
