/* ------------------------------------------------------------------------
 * mimgmake.c:  Utility to construct a memory image by preloading
 *              data and ELF files.
 *
 * Mark P Jones, March 2006.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include "mimg.h"

#define ABORT exit(1)

/* ========================================================================
 * Support code for writing a (binary) output file:
 */
#define BINOUTLEN  1024
static unsigned char binoutbuf[BINOUTLEN];
static unsigned binoutpos;
static unsigned binoutlen;
static int binoutfd;

void binoutOpen(char* filename) {
  binoutpos = 0;
  binoutlen = 0;
  binoutfd  = open(filename, O_CREAT|O_TRUNC|O_WRONLY, 0666);
  if (binoutfd<0) {
    printf("Unable to create output file \"%s\"\n", filename);
    ABORT;
  }
}

void binoutFlush() {
  if (write(binoutfd, binoutbuf, binoutpos)!=binoutpos) {
    printf("Unable to write to output file\n");
    ABORT;
  }
  binoutlen += binoutpos;
  binoutpos  = 0;
}

void binoutClose() {
  if (binoutpos>0) {
    binoutFlush();
  }
  close(binoutfd);
  printf("Wrote %d bytes\n", binoutlen);
}

void outbyte(unsigned char b) {  /* Output a single bye                 */
  binoutbuf[binoutpos++] = (b & 0xff);
  if (binoutpos>=BINOUTLEN) {
    binoutFlush();
  }
}

void outword(unsigned w) {       /* Output word in little endian format */
  outbyte(w);
  outbyte(w >> 8);
  outbyte(w >> 16);
  outbyte(w >> 24);
}

/* ========================================================================
 * Represents the list of loaded files.
 */
struct Header {
  unsigned minAddr, maxAddr, entry;
  struct Header* next;
};

/* ------------------------------------------------------------------------
 * Display contents of a header:
 */
void showHeader(struct Header* hdr) {
  printf("[0x%08x-0x%08x], entry 0x%08x",
         hdr->minAddr, hdr->maxAddr, hdr->entry);
}

/* ------------------------------------------------------------------------
 * Find first entry point specified in a list of headers:
 */
unsigned firstEntry(struct Header* hdrs) {
  for (; hdrs; hdrs=hdrs->next) {
    if (hdrs->entry!=NOENTRY) {
      return hdrs->entry;
    }
  }
  return NOENTRY;
}

/* ------------------------------------------------------------------------
 * Calculate length of header list:
 */
unsigned numHeaders(struct Header* hdrs) {
  unsigned len = 0;
  for (; hdrs; hdrs=hdrs->next) {
    len++;
  }
  return len;
}

/* ------------------------------------------------------------------------
 * Output the header bytes for an image.
 */
void outheaders(struct Header* hdrs, unsigned first, unsigned last) {
  unsigned len = numHeaders(hdrs);
  unsigned req = BOOTLEN(len);
  if (1+(last-first)<req) {
    printf("Headers will not fit in [0x%x-0x%x]: ", first, last);
    printf("at least 0x%x bytes required\n", req);
    ABORT;
  }
  /* Output the header list */
  outword(len);
  for (; hdrs; hdrs=hdrs->next) {
    outword(hdrs->minAddr);
    outword(hdrs->maxAddr);
    outword(hdrs->entry);
  }
}

/* ------------------------------------------------------------------------
 * Add a new header to the end of a list.
 */
struct Header* addHeader(struct Header* hdrs,
                         unsigned minAddr, unsigned maxAddr,
                         unsigned entry) {
  struct Header* prev = NULL;
  struct Header* curr = hdrs;
  struct Header* new  = (struct Header*)malloc(sizeof(struct Header));
  if (!new) {
    printf("Could not allocate header structure\n");
    ABORT;
  }
  while (curr) {
    prev = curr;
    curr = curr->next;
  }
  new->minAddr = minAddr;
  new->maxAddr = maxAddr;
  new->entry   = entry;
  new->next    = NULL;
  if (prev) {
    prev->next = new;
    return hdrs;
  } else {
    return new;
  }
}

/* ========================================================================
 * An in-memory representation for a file.
 */
struct FileImage {
  char*    filename;
  unsigned length;
  void*    contents;
};

/* ------------------------------------------------------------------------
 * Read the contents of a file into a buffer.
 */
struct FileImage* readFile(char* filename) {
  struct stat sb;
  int         fd;
  unsigned    length;
  void*       contents;
  struct FileImage* img = NULL;

  if (stat(filename, &sb)) {
    printf("Could not stat file \"%s\"\n", filename);
    ABORT;
  } else if ((length=(unsigned)sb.st_size)==0) {
    printf("File \"%s\" is empty\n", filename);
    ABORT;
  } else if ((fd=open(filename, O_RDONLY))<0) {
    printf("Could not open file \"%s\"\n", filename);
    ABORT;
  } else {
    if (!(contents=(char*)malloc(length))) {
      printf("Could not allocate buffer for contents of \"%s\"\n", filename);
      ABORT;
    } else if (!(img=(struct FileImage*)malloc(sizeof(struct FileImage)))) {
      printf("Could not allocate file image structure\n");
      ABORT;
    } else if (read(fd, contents, length)!=length) {
      printf("Could not read contents of \"%s\"\n", filename);
      ABORT;
    } else {
      img->filename = filename;
      img->length   = length;
      img->contents = contents;
    }
    close(fd);
  }
  return img;
}

/* ========================================================================
 * ELF file and program headers:
 */
typedef unsigned long  ElfWord, ElfAddr, ElfOff;
typedef unsigned short ElfHalf;

/* Defines the layout of the header at the beginning of an ELF file.
 * For the time being, we are only interested in executable IA32 object
 * files.
 */
struct ElfHeader {
    unsigned char ident[16];            /* elf Identification:             */
#define EI_CLASS    4                   /*   ident[EI_CLASS] file class    */
#define ELFCLASS32  1                   /*     32 bit objects              */
#define EI_DATA     5                   /*   ident[EI_DATA] data encoding  */
#define ELFDATA2LSB 1                   /*     LSB in lowest addr          */
#define ELFDATA2MSB 2                   /*     MSB in lowest addr          */

    ElfHalf       type;                 /* Object file type:               */
#define ET_EXEC 2                       /*   executable file               */

    ElfHalf       machine;              /* Required architecture:          */
#define EM_386  3                       /*   Intel 80386                   */

    ElfWord       version;              /* Object file version             */

    ElfAddr       entry;                /* Entry point (virtual address)   */

    ElfOff        phoff;                /* Offset to program header table  */
                                        /*   (or zero if no prog hdr tab)  */
    ElfOff        shoff;                /* Offset to section header table  */
                                        /*   (or zero if no sec hdr tab)   */

    ElfWord       flags;                /* Processor specific flags:       */
                                        /*   zero on EM_386                */

    ElfHalf       ehsize;               /* Size of ELF header in bytes     */

    ElfHalf       phentsize;            /* Size of prog hdr entry (bytes)  */
    ElfHalf       phnum;                /* Number of prog hdr entries      */

    ElfHalf       shentsize;            /* Size of sec hdr entry (bytes)   */
    ElfHalf       shnum;                /* Number of sec hdr entries       */

    ElfHalf       shstrndx;             /* Index of sec name string tab:   */
#define SHN_UNDEF 0                     /*   no section name string table  */
};

struct ElfProgHeader {
    ElfWord type;                       /* Segment type:                   */
#define PT_LOAD      1                  /*   loadable segment              */
#define PT_GNU_STACK 0x6474e551         /*   stack segment (from binutils  */
                                        /*    include/elf/common.h)        */
    ElfOff  offset;                     /* Offset in file of first byte    */
    ElfAddr vaddr;                      /* Virtual address of first byte   */
    ElfAddr paddr;
    ElfAddr filesz;                     /* Size in file image              */
    ElfAddr memsz;                      /* Size in memory image            */
    ElfAddr flags;
    ElfAddr align;                      /* Alignment requirement           */
};

static unsigned lobyte = 0x00ff, hibyte = 0xff00;
static unsigned char byteorder; /* ELFDATA2MSB or ELFDATA2LSB */

void calcByteorder() {
  unsigned word = (ELFDATA2MSB<<24) | (ELFDATA2LSB);
  byteorder = *(unsigned char*)(&word);
}

ElfHalf elfHalf(struct ElfHeader* hdr, ElfHalf x) {   /* x = ab   */
  if (hdr->ident[EI_DATA]==byteorder) {
    return x;
  } else {
    return ((x>>8) & lobyte)     /* shifted: -a;    masked:  0a   */
         | ((x & lobyte)<<8);    /* masked:  0b;    shifted: b0   */
  }
}

ElfWord elfWord(struct ElfHeader* hdr, ElfWord x) {   /* x = abcd */
  if (hdr->ident[EI_DATA]==byteorder) {
    return x;
  } else {
    return ((x>>24) & lobyte)    /* shifted: ---a;  masked:  000a */
         | ((x>>8)  & hibyte)    /* shifted: -abc;  masked:  00b0 */
         | ((x & hibyte) <<8)    /* masked:  00c0;  shifted: 0c00 */
         | ((x & lobyte)<<24);   /* masked:  000d;  shifted: d000 */
  }
}

struct ElfHeader* elfHeader(void* contents, unsigned length) {
  struct ElfHeader* hdr = (struct ElfHeader*) contents;
  if (length >= sizeof(struct ElfHeader)
      && hdr->ident[0] == 0x7f
      && hdr->ident[1] == 'E'
      && hdr->ident[2] == 'L'
      && hdr->ident[3] == 'F'
      && hdr->ident[EI_CLASS] == ELFCLASS32
      && elfHalf(hdr, hdr->type) == ET_EXEC
      && elfHalf(hdr, hdr->machine) == EM_386
      && elfHalf(hdr, hdr->ehsize) == sizeof(struct ElfHeader)
      && elfHalf(hdr, hdr->phentsize) == sizeof(struct ElfProgHeader)) {
      return hdr;
  }
  return 0;
}

/* ========================================================================
 * Represent a memory image as a linked list of headers and a linked list
 * of sections.
 */
struct MemImage {
  struct Header*  hdrs;     /* List of headers                   */
  struct Section* list;     /* List of sections                  */
  struct Section* mri;      /* Most recently inserted section    */
  unsigned entry;           /* Entry point, post load            */
};

struct Section {
  unsigned          first;  /* first address in memory image      */
  unsigned          last;   /* last address in memory image       */
  struct FileImage* img;    /* image for this section             */
  unsigned          offset; /* offset of section in image         */
  struct Section*   next;
};

/* ------------------------------------------------------------------------
 * Allocate a new section:
 */
struct Section* section(unsigned first, unsigned last,
                        struct FileImage* img, unsigned offset) {
  struct Section* sec = NULL;
  if (first>last) {
    printf("Empty section [0x%08x-0x%08x]\n", first, last);
    ABORT;
  } else if (!(sec = (struct Section*)malloc(sizeof(struct Section)))) {
    printf("Could not allocate section\n");
    ABORT;
  } else {
    sec->first  = first;
    sec->last   = last;
    sec->img    = img;
    sec->offset = offset;
    sec->next   = NULL;
  }
  return sec;
}

/* ------------------------------------------------------------------------
 * Display contents of a section:
 */
void showSection(struct Section* sec) {
  printf("[0x%08x-0x%08x] ", sec->first, sec->last);
  if (sec->img) {
    printf("from \"%s\", offset 0x%x", sec->img->filename, sec->offset);
  } else {
    printf("type %d", sec->offset);
  }
}

/* ------------------------------------------------------------------------
 * Output the byte representation for a section:
 */
void outsection(struct MemImage* mimg, struct Section* sec) {
  if (sec->img==NULL && sec->offset==RESERVED) {
    return; /* skip RESERVED sections */
  }
  outword(sec->first);
  outword(sec->last);
  outword(0/*prev*/);
  if (sec->img) {
    outword(DATA);
    unsigned len     = sec->last - sec->first;
    unsigned char* p = (unsigned char*)(sec->img->contents) + sec->offset;
    unsigned char* q = p + len;
    while (p<=q) {
      outbyte(*p++);
    }
  } else {
    outword(sec->offset);
    if (sec->offset==BOOTDATA) {
      outheaders(mimg->hdrs, sec->first, sec->last);
    }
  }
}

/* ------------------------------------------------------------------------
 * Allocate a memory image:
 */
struct MemImage* memImage() {
  struct MemImage* mimg = (struct MemImage*)malloc(sizeof(struct MemImage));
  if (!mimg) {
    printf("Could not allocate memory image\n");
    ABORT;
  }
  mimg->hdrs  = NULL;
  mimg->list  = NULL;
  mimg->mri   = NULL;
  mimg->entry = NOENTRY;
  return mimg;
}

/* ------------------------------------------------------------------------
 * Check that a valid entry point has been provided.
 */
void checkEntry(struct MemImage* mimg) {
  struct Section* list = mimg->list;
  if (mimg->entry==NOENTRY) {
    mimg->entry = firstEntry(mimg->hdrs);
  }
  if (mimg->entry==NOENTRY) {
    printf("No entry point has been specified.");
    ABORT;
  }
  for (; list; list=list->next) {
    if (list->img && list->first<=mimg->entry && mimg->entry<=list->last) {
      return;
    }
  }
  printf("Entry point, 0x%x, is not loaded in any section.\n", mimg->entry);
  ABORT;
}

/* ------------------------------------------------------------------------
 * Display contents of a memory image:
 */
void showMemImage(char* name, struct MemImage* mimg) {
  struct Section* list = mimg->list;
  struct Header*  hdrs = mimg->hdrs;
  int n = 0;
  printf("Memory Image \"%s\":\n", name);
  printf(" Sections:\n");
  for (; list; list=list->next) {
    printf("  Section[%d]: ", n++);
    showSection(list);
    printf("\n");
  }
  printf(" Headers:\n");
  for (n=0; hdrs; hdrs=hdrs->next) {
    printf("  Header[%d]: ", n++);
    showHeader(hdrs);
    printf("\n");
  }
  printf(" Entry point: 0x%x\n", mimg->entry);
}

/* ------------------------------------------------------------------------
 * Output the byte representation for a memory image.
 */
void outimage(struct MemImage* mimg) {
  struct Section* list = mimg->list;
  outbyte('m'); outbyte('i'); outbyte('m'); outbyte('g');/*"magic number"*/
  outword(0);                                            /*version number*/
  outword(mimg->entry);                                  /* entry point  */
  for (; list; list=list->next) {
    outsection(mimg, list);
  }
}

/* ------------------------------------------------------------------------
 * Insert a new section into a memory image.
 */
void insert(struct MemImage* mimg, struct Section* new) {
  struct Section* prev = NULL;
  struct Section* curr = mimg->list;

  /* Check that new section is valid ---------- */
  if (!new || new->first>new->last) {
    printf("New section is not valid: ");
    showSection(new);
    printf("\n");
    ABORT;
  }

  /* Find insert point for new section -------- */
  while (curr && new->last>=curr->first) {
    if (new->first<=curr->last) {
      printf("Overlapping sections:\n   ");
      showSection(curr);
      printf("\nvs ");
      showSection(new);
      printf("\n");
      ABORT;
    }
    prev = curr;
    curr = curr->next;
  }

  /* Insert new section ----------------------- */
  new->next = curr;
  mimg->mri = new;
  if (prev) {
    prev->next = new;
  } else {
    mimg->list = new;
  }
}

/* ------------------------------------------------------------------------
 * Insert a FileImage:
 */
void insertFile(struct MemImage* mimg, char* filename, unsigned first) {
  struct FileImage* img = readFile(filename);
  unsigned last         = first + img->length - 1;
  insert(mimg, section(first, last, img, 0));
  mimg->hdrs = addHeader(mimg->hdrs, first, last, NOENTRY);
}

/* ------------------------------------------------------------------------
 * Insert loadable parts of an ELF file:
 */
void insertElf(struct MemImage* mimg, char* filename, int load) {
  struct FileImage* img = readFile(filename);
  struct ElfHeader* hdr = elfHeader(img->contents, img->length);
  if (!hdr) {
    printf("Input file \"%s\" is not in ELF format\n", filename);
    ABORT;
  } else {
    ElfWord phoff   = elfWord(hdr, hdr->phoff);
    ElfHalf phnum   = elfHalf(hdr, hdr->phnum);
    ElfWord entry   = elfWord(hdr, hdr->entry);
    ElfWord minAddr = 0xffffffff;
    ElfWord maxAddr = 0x00000000;
    if (phoff && phnum) {
      int i = 0;
      struct ElfProgHeader* phdrs
            = (struct ElfProgHeader*)((void*)hdr + phoff);
      for (; i<phnum; i++) {
        if (elfWord(hdr, phdrs[i].type)==PT_LOAD) {
          ElfWord offset = elfWord(hdr, phdrs[i].offset);
          ElfWord paddr  = elfWord(hdr, phdrs[i].paddr);
          ElfWord filesz = elfWord(hdr, phdrs[i].filesz);
          ElfWord memsz  = elfWord(hdr, phdrs[i].memsz);
          struct Section* sec = NULL;
          if (offset+filesz>img->length) {
            printf("Invalid ELF section passes end of file \"%s\"\n", filename);
            ABORT;
          }

          /* Try to reserve/insert data and zero sections */
          if (load) {
            if (filesz>0) {
              insert(mimg, section(paddr, paddr+filesz-1, img, offset));
            }
            if (memsz>filesz) {
              insert(mimg, section(paddr+filesz, paddr+memsz-1, NULL, ZERO));
            }
          } else {
            insert(mimg, section(paddr, paddr+memsz-1, NULL, RESERVED));
          }
          if (paddr < minAddr) {
            minAddr = paddr;
          }
          if (paddr+memsz-1 > maxAddr) {
            maxAddr = paddr+memsz-1;
          }

        } /* end LOAD section */
      } /* end for each program section */
    } /* end phoff and phnum non-zero */
    if (load) {
      mimg->hdrs = addHeader(mimg->hdrs, minAddr, maxAddr, entry);
    }
  } /* end found valid ELF file */
}

/* ========================================================================
 * Parse arguments:
 */

/* Decode a single hexadecimal digit.
 */
static int hexDigit(char c) {
  return (c>='0' && c<='9') ? (c - '0')
       : (c>='a' && c<='f') ? (10 + (c - 'a'))
       : (c>='A' && c<='F') ? (10 + (c - 'A'))
       : (-1);
}

static unsigned addr;

/* Read an address in hex with an optional 0x prefix.
 */
static char* readAddr(char* arg, char* s) {
  int digit = 0;
  addr      = 0;
  if (s[0]=='0' && (s[1]=='x' || s[1]=='X')) { /* skip leading 0x prefix */
    s+=2;
  }
  if ((digit=hexDigit(*s))<0) {
    printf("Missing address in argument \"%s\"\n", arg);
    ABORT;
  }
  do {
    if (addr >> 28) {  /* examine top nibble for overflow */
      printf("Address overflow in argument \"%s\"\n", arg);
      ABORT;
    }
    addr = (addr << 4) | (0xf & digit);
  } while ((digit=hexDigit(*++s))>=0);
  return s;
}

char* copyname(char* from, char* upto) {
  char* new = (char *)malloc((upto-from)+1);
  if (!new) {
    printf("Could not allocate a filename copy\n");
    ABORT;
  } else {
    char* t = new;
    while (from<upto) {
      *t++ = *from++;
    }
    *t = '\0';
  }
  return new;
}

unsigned nextAddr(struct Section* mri, char* arg, unsigned align) {
  if (!mri) {
    printf("No previous loaded section for argument \"%s\"\n", arg);
    ABORT;
  }
  return ((mri->last >> align) + 1) << align;
}

/* Parse and interpret an argument string:
 */
void parseArg(struct MemImage* mimg, char* arg) {
  char* s = arg;
  while (*s && *s!=':' && *s!='@') {
    s++;
  }
  if (*s=='\0') {                /* simple filename; ELF load */
    insertElf(mimg, arg, 1);
  } else if (*s=='@') {          /* filename@addr; file load  */
    char* filename = copyname(arg,s++);
    if (strcmp(s, "next")==0) {
      addr = nextAddr(mimg->mri, arg, 1);
    } else if (strcmp(s,"page")==0) {
      addr = nextAddr(mimg->mri, arg, 12);
    } else if (*readAddr(arg, s)) {
      printf("Junk after address in argument \"%s\"\n", arg);
      ABORT;
    }
    if (strcmp(filename,"entry")==0) {
      if (mimg->entry!=NOENTRY && addr!=mimg->entry) {
        printf("Multiple entry points (0x%x, 0x%x) specified\n",
               mimg->entry, addr);
        ABORT;
      }
      mimg->entry = addr;
    } else {
      insertFile(mimg, filename, addr);
    }
  } else if (strncmp(arg, "noload:", 7)==0) {
    insertElf(mimg, s+1, 0);      /* noload:file; ELF reserve  */
  } else {                        /* keyword:addr-addr; special */
    unsigned first, last;
    s     = readAddr(arg, s+1);
    first = addr;
    if (*s!='-') {
      printf("Missing range in argument \"%s\"\n", arg);
      ABORT;
    }
    s     = readAddr(arg, s+1);
    last  = addr;
    if (*s!='\0' && *s!=';') {
      printf("Junk after range in argument \"%s\"\n", arg);
      ABORT;
    }
    if (first>last) {
      printf("Illegal range in argument \"%s\"\n", arg);
      ABORT;
    }
    if (*arg=='z') {          /* z{ero} section   */
      insert(mimg, section(first, last, NULL, ZERO));
      mimg->hdrs = addHeader(mimg->hdrs, first, last, NOENTRY);
    } else if (*arg=='b') {   /* b{ootdata} section */
      insert(mimg, section(first, last, NULL, BOOTDATA));
      mimg->hdrs = addHeader(mimg->hdrs, first, last, NOENTRY);
    } else if (*arg=='r') {   /* r{eserved} section */
      insert(mimg, section(first, last, NULL, RESERVED));
      mimg->hdrs = addHeader(mimg->hdrs, first, last, NOENTRY);
    } else {
      printf("Unrecognized argument \"%s\"\n", arg);
      ABORT;
    }
  }
}

int main(int argc, char* argv[]) {
  calcByteorder();
  if (argc<2) {
    printf("Usage: mimgmake imagefile [arg ...]\n");
    printf("where each arg is one of the following:\n");
    printf("  file               load ELF file\n");
    printf("  noload:file        reserve ELF file\n");
    printf("  zero:addr-addr     zero all addresses in specified range\n");
    printf("  bootdata:addr-addr store bootdata in specified range\n");
    printf("  reserved:addr-addr reserve all addresses in specified range\n");
    printf("  entry@addr         set explicit entry point\n");
    printf("  file@addr          load file at given address\n");
    printf("  file@next          load file at next address\n");
    printf("  file@page          load file at next page boundary\n");
    ABORT;
  } else {
    struct MemImage* mimg = memImage();
    int i = 2;
    for (; i<argc; i++) {
      parseArg(mimg, argv[i]);
    }
    checkEntry(mimg);

    showMemImage(argv[1], mimg);
    binoutOpen(argv[1]);
    outimage(mimg);
    binoutClose();
  }
  return 0;
}

/* ===================================================================== */
