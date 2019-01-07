/* ------------------------------------------------------------------------
 * A memory image is a contiguous array of bytes containing an initial
 * "magic number" (the byte sequence 'm', 'i', 'm', 'g'), a version
 * number (currently unused), an entry point address, and then a sequence
 * of "sections", each of which takes the form:
 *
 *     +-------+------+------+------+------  ----+
 *     | first | last | prev | type | data . . . |
 *     +-------+------+------+------+------  ----+
 *
 * Here: "first" is the address of the first byte in the section
 *       "last" is the address of the last byte in the section
 *       "prev" is reserved space that is set to zero in the image.
 *         The loader can use this space for any purpose; however,
 *         as the name suggests, it is likely to be used to store
 *         a pointer to the start of the previous section.
 *       "type" is a word that describes the type of the section
 *         (ZERO=0, DATA=1, HEADER=2, RESERVED=3, ...)
 *       "data" is a sequence of bytes; this will be empty, except
 *         for a DATA or HEADER section where it will contain
 *         exactly (last-first)+1 bytes.
 *
 * The "first", "last", and "type" fields are stored using little
 * endian byte order.
 * --------------------------------------------------------------------- */

#include "mimguser.h"

typedef void (*entrypoint)();

struct MimgHeader {
  unsigned char magic[4];
  unsigned      version;
  entrypoint    entry;
};

struct SectionHeader {
  unsigned first;
  unsigned last;
  unsigned prev;
  unsigned type;
};

#define ZERO     (0)        /* the "type" for a ZERO section             */
#define DATA     (1)        /* the "type" for a DATA section             */
#define BOOTDATA (2)        /* the "type" for a BOOTDATA section         */
#define RESERVED (3)        /* the "type" for a RESERVED section         */

/* How many bytes are required for the array of header information?      */
#define HDRLEN(l) (4+12*(l))/* Number of bytes in a header section with  */
                            /* l headers: 4 bytes for length + 12 bytes  */
                            /* per header.                               */

/* How many bytes (minimum) are required for a header section that has
 * len headers in it?  16 bytes for initial BootData structure;
 *                     HDRLEN(len) bytes for the header information;
 *                     4 (or more) bytes for memory map information;
 *                     2 (or more) bytes for null bytes of strings.
 */
#define BOOTLEN(len) (sizeof(struct BootData) + HDRLEN(len) + 4 + 2)

/* --------------------------------------------------------------------- */
