/* ------------------------------------------------------------------------
 * mimgload.c:  memory image boot loader
 *
 * Mark P. Jones, March 2006
 */
#include "mimg.h"
#include "simpleio.h"

#define  DEBUG(cmd)  /*cmd*/

/* ------------------------------------------------------------------------
 * Multiboot data structures:
 */
extern struct MultibootInfo* mbi;
extern unsigned              mbi_magic;
#define MBI_MAGIC 0x2BADB002

struct MultibootInfo {
  unsigned                flags;
# define MBI_MEM_VALID    (1 << 0)
# define MBI_CMD_VALID    (1 << 2)
# define MBI_MODS_VALID   (1 << 3)
# define MBI_MMAP_VALID   (1 << 6)

  unsigned                memLower;
  unsigned                memUpper;
  unsigned                bootDevice;
  char*                   cmdline;
  unsigned                modsCount;
  struct MultibootModule* modsAddr;
  unsigned                syms[4];
  unsigned                mmapLength;
  unsigned                mmapAddr;
};

struct MultibootModule {
  unsigned modStart;
  unsigned modEnd;
  char*    modString;
  unsigned reserved;
};

struct MultibootMMap {
  unsigned size;
  unsigned baseLo;
  unsigned baseHi;
  unsigned lenLo;
  unsigned lenHi;
  unsigned type;
};

/* ------------------------------------------------------------------------
 * Determine whether a memory map is (or can be made) available from the
 * supplied multiboot information structure.
 */
struct MultibootMMap defaultMMap[2] = {
  {20, 0, 0, 0, 0, 1},
  {20, 0, 0, 0, 0, 1}
};

/* Determine whether we can obtain a memory map from the multiboot info.
 */
int hasMMap() {
  if (mbi->flags & MBI_MMAP_VALID) {        /* Already has a memory map? */
    return 1;
  } else if (mbi->flags & MBI_MEM_VALID) {  /* Fake a simple memory map? */
    defaultMMap[0].lenLo  = (mbi->memLower << 10);
    defaultMMap[1].baseLo = 0x100000;  /* 1MB */
    defaultMMap[1].lenLo  = (mbi->memUpper << 10);
    mbi->mmapAddr         = (unsigned)(&defaultMMap);
    mbi->mmapLength       = sizeof(defaultMMap);
    return 1;
  }                                         /* No memory map!            */
  return 0;
}

/* Determine whether a given MultibootMMap structure describes an
 * available range of physical (32 bit) addresses.
 */
int mmapAvailable(struct MultibootMMap* mmap) {
  return (mmap->type=1)
      && (mmap->baseHi==0) && (mmap->lenHi==0)
      && (mmap->baseLo+mmap->lenLo-1 >= mmap->baseLo);
}

/* Determine whether a given range of addresses fits in the memory map.
 */
int fitsInMemory(unsigned first, unsigned last) {
  unsigned m = mbi->mmapAddr;
  unsigned l = mbi->mmapAddr + mbi->mmapLength;
  while (m < l) {
    struct MultibootMMap* mmap = (struct MultibootMMap*)m;
    if (mmap->baseLo<=first
        && last<mmap->baseLo+mmap->lenLo
        && mmapAvailable(mmap)) {
      return 1;
    }
    m += (mmap->size + sizeof(mmap->size));
  }
  return 0;
}

/* Copy memory map details into a BOOTDATA section.
 */
unsigned copyMMap(unsigned first, unsigned last) {
  unsigned num = ((last-first)-3) / 8;
  if (num>=0) { /* We must have room at least for the count */
    unsigned  m  = mbi->mmapAddr;
    unsigned  l  = mbi->mmapAddr + mbi->mmapLength;
    unsigned* ns = (unsigned*)first;
    unsigned  i  = 0;
    while (i<num && m<l) {
      struct MultibootMMap* mmap = (struct MultibootMMap*)m;
      if (mmapAvailable(mmap)) {
        i++;
        ns[2*i-1] = mmap->baseLo;
        ns[2*i]   = mmap->baseLo + mmap->lenLo - 1;
        DEBUG(printf("memory map[%d]: %x-%x\n", i, ns[2*i-1], ns[2*i]));
      }
      m += (mmap->size + sizeof(mmap->size));
    }
    ns[0]  = i;        /* write the count */
    first += 4 + 8*i;  /* 4 bytes for count + 8 bytes for each entry */
  }
  return first;
}

/* ------------------------------------------------------------------------
 * Memory image loading:
 */

/* Calculate the address of the first byte after the current section.
 */
unsigned nextSection(struct SectionHeader* curr) {
  unsigned next = (unsigned)curr + sizeof(struct SectionHeader);
  if (curr->type==DATA) {
    next += 1 + (curr->last - curr->first);
  } else if (curr->type==BOOTDATA) {
    next += HDRLEN(*(unsigned*)(curr+1));
  }
  return next;
}

/* Validate a memory image, checking for structural consistency.
 * Return a string describing any error that is found; a NULL
 * string indicates that the image is valid.
 */
char* validImage(unsigned start, unsigned finish) {
  extern unsigned char _text_start[], _bss_end[];
  if (start>finish) {
    return "image start exceeds image finish";
  } else if (1+(finish-start)<sizeof(struct MimgHeader)) {
    return "image is too small";
  } else {
    struct MimgHeader* mimg = (struct MimgHeader*)start;
    unsigned allowed        = 0;
    unsigned foundEntry     = 0;
    if (mimg->magic[0]!='m' || mimg->magic[1]!='i'
     || mimg->magic[2]!='m' || mimg->magic[3]!='g') {
      return "image has incorrect magic number";
    } else if ((unsigned)mimg->entry==NOENTRY) {
      return "image does not specify an entry point";
    }
    start += sizeof(struct MimgHeader);  /* skip magic number */
    while (start<=finish) {
      struct SectionHeader* curr = (struct SectionHeader*)start;
      if (start+sizeof(struct SectionHeader)>finish+1) {
        return "incomplete section header";
      } else if (curr->first>curr->last) {
        return "section first exceeds section last";
      } else if (curr->first < allowed) {
        return "sections overlap or are not sorted";
      } else if (!fitsInMemory(curr->first, curr->last)) {
        return "section does not fit within memory map";
      } else if (!(curr->last  <  (unsigned)_text_start
                || curr->first >= (unsigned)_bss_end)) {
        return "section overlaps with loader";
      } else if (curr->type==BOOTDATA 
              && (curr->first+BOOTLEN(*(unsigned*)(curr+1)))>curr->last+1) {
        return "bootdata section is too small";
      } else {
        unsigned next = nextSection(curr);
        if (next>finish+1) {
          return "section does not fit in image";
        }
        if (next<start+sizeof(struct SectionHeader)) {
          return "section wraps around address space";
        }
        if (curr->type == DATA
         && curr->first <= (unsigned)mimg->entry
         && curr->last  >= (unsigned)mimg->entry) {
          foundEntry = 1;
        }
        start   = next;
        allowed = curr->last+1;
      }
    }
    return foundEntry ? 0 : "entry point falls outside loaded sections";
  }
}

/* Copy len bytes from one location to another, allowing for the
 * possibility that the two regions overlap.
 */
void smartcopy(unsigned to, unsigned from, unsigned len) {
  unsigned char* dst = (unsigned char*)to;
  unsigned char* src = (unsigned char*)from;
  if (to<from) {        /* load data front to back */
    for (; len>0; len--) {
      *dst++ = *src++;
    }
  } else if (to>from) { /* load data back to front */
    for (src+=len, dst+=len; len>0; len--) {
      *--dst = *--src;
    }
  }
  /* other case: to == from; no copy required! */
}

/* Copy a null-terminated string s into [first..last], truncating
 * if necessary.
 */
unsigned copyStr(char* s, unsigned first, unsigned last) {
  if (first<=last) {
    char* p = (char*)first;
    for (; first<last && *s; first++) {
      *p++ = *s++;
    }
    *p = '\0';
    first++;
  }
  return first;
}

/* Load the contents of a single section into memory, either by zeroing
 * all the bytes in the specified range, by copying bytes from the data
 * portion of the section, or by writing boot data into it.
 */
void loadSection(struct SectionHeader* sec) {
  unsigned data = (unsigned)sec + sizeof(struct SectionHeader);
  unsigned len  = 1 + (sec->last - sec->first);
  DEBUG(printf("section [%x-%x] loads to ", sec, nextSection(sec)));
  DEBUG(printf("[%x-%x]\n", sec->first, sec->last));
  if (sec->type==ZERO) {
    unsigned char* dst = (unsigned char*)sec->first;
    for (; len>0; len--) {
      *dst++ = '\0';
    }
  } else if (sec->type==DATA) {
    smartcopy(sec->first, data, len);
  } else if (sec->type==BOOTDATA) {
    /* first and last might be overwritten during the smartcopy step! */
    unsigned first      = sec->first;
    unsigned last       = sec->last;
    struct BootData* bd = (struct BootData*)first;
    unsigned req        = HDRLEN(*(unsigned*)data);
    unsigned hdrs       = first + sizeof(struct BootData);
    char*    cmdline    = (mbi->flags & MBI_CMD_VALID) ? mbi->cmdline : "";
    char*    imgline    = mbi->modsAddr[0].modString;
    unsigned nxt        = hdrs + req;
    smartcopy(hdrs, data, req);
    bd->headers = (unsigned*)hdrs;
    bd->mmap    = (unsigned*)nxt;
    nxt         = copyMMap(nxt, last-2);
    bd->cmdline = (char*)nxt;
    nxt         = copyStr(cmdline, nxt, last-1);
    bd->imgline = (char*)nxt;
    copyStr(imgline, nxt, last);
  }
}

/* Load the contents of an image, assuming that the first byte of the
 * first section header is at address "start" and that the last byte
 * of the last section header is at address "last".
 */
void loadImage(unsigned start, unsigned finish) {
  start += sizeof(struct MimgHeader);  /* skip magic number */
  while (start <= finish) {
    struct SectionHeader* prev = 0;
    struct SectionHeader* curr = (struct SectionHeader*)start;
    unsigned next              = nextSection(curr);

    /* Skip sections that cannot be loaded */
    while (next<=finish         /* This is not the last section and it   */
        && curr->last>=next     /* loads over later sections in the mimg */
        && curr->first<=finish) {
      curr->prev = (unsigned)prev;
      prev       = curr;
      curr       = (struct SectionHeader*)next;
      next       = nextSection(curr);
    }

    /* Load current section and any that came before it. */
    loadSection(curr);
    while (prev) {
      curr = prev;
      prev = (struct SectionHeader*)curr->prev;
      loadSection(curr);
    }

    /* Continue to load remaining sections */
    start = next;
  }
}

/* ------------------------------------------------------------------------
 * Main program:
 */
void mimgload() {
  cls();
  printf("Memory Image Loader (mimgload) 0.1\n");
  if (mbi_magic!=MBI_MAGIC) {
    printf("Invalid multiboot magic number.\n");
  } else if (!hasMMap()) {
    printf("Cannot obtain memory map.\n");
  } else if (!(mbi->flags & MBI_MODS_VALID)) {
    printf("Cannot locate memory image.\n");
  } else if (mbi->modsCount<1) {
    printf("No boot modules specified.\n");
  } else if (mbi->modsCount>1) {
    printf("Multiple boot modules specified.\n");
  } else {
    unsigned start  = mbi->modsAddr[0].modStart;
    unsigned finish = mbi->modsAddr[0].modEnd - 1;
    char*    msg    = validImage(start, finish);
    DEBUG(printf("Boot image located at [%x-%x]\n", start, finish));
    if (msg) {
      printf("Invalid image: %s\n", msg);
    } else {
      entrypoint entry = ((struct MimgHeader*)start)->entry;
      loadImage(start, finish);
      DEBUG(printf("Now branch to address 0x%x\n", entry));
      (*entry)();
    }
  }
  printf("Boot attempt failed; system halting.\n");
}

/* --------------------------------------------------------------------- */
