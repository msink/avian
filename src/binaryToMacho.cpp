/* Copyright (c) 2008, Avian Contributors

   Permission to use, copy, modify, and/or distribute this software
   for any purpose with or without fee is hereby granted, provided
   that the above copyright notice and this permission notice appear
   in all copies.

   There is NO WARRANTY for this software.  See license.txt for
   details. */

#include "stdint.h"
#include "stdio.h"
#include "string.h"

#include "sys/stat.h"
#include "sys/mman.h"
#include "fcntl.h"
#include "unistd.h"

#include "mach-o/loader.h"
#include "mach-o/nlist.h"

namespace {

inline unsigned
pad(unsigned n)
{
  return (n + (4 - 1)) & ~(4 - 1);
}

void
writeObject(FILE* out, const uint8_t* data, unsigned size,
            const char* dataName, const char* sizeName)
{
  unsigned dataNameLength = strlen(dataName) + 1;
  unsigned sizeNameLength = strlen(sizeName) + 1;

  mach_header header = {
    MH_MAGIC, // magic
    CPU_TYPE_I386, // cputype
    CPU_SUBTYPE_I386_ALL, // cpusubtype
    MH_OBJECT, // filetype,
    2, // ncmds
    sizeof(segment_command)
    + sizeof(section)
    + sizeof(symtab_command), // sizeofcmds
    0 // flags
  };
  
  segment_command segment = {
    LC_SEGMENT, // cmd
    sizeof(segment_command) + sizeof(section), // cmdsize
    "__TEXT", // segname
    0, // vmaddr
    pad(size), // vmsize
    sizeof(mach_header)
    + sizeof(segment_command)
    + sizeof(section)
    + sizeof(symtab_command), // fileoff
    pad(size), // filesize
    7, // maxprot
    7, // initprot
    1, // nsects
    0 // flags
  };

  section sect = {
    "__const", // sectname
    "__TEXT", // segname
    0, // addr
    pad(size), // size
    sizeof(mach_header)
    + sizeof(segment_command)
    + sizeof(section)
    + sizeof(symtab_command), // offset
    0, // align
    0, // reloff
    0, // nreloc
    S_REGULAR, // flags
    0, // reserved1
    0, // reserved2
  };

  symtab_command symbolTable = {
    LC_SYMTAB, // cmd
    sizeof(symtab_command), // cmdsize
    sizeof(mach_header)
    + sizeof(segment_command)
    + sizeof(section)
    + sizeof(symtab_command)
    + pad(size), // symoff
    2, // nsyms
    sizeof(mach_header)
    + sizeof(segment_command)
    + sizeof(section)
    + sizeof(symtab_command)
    + pad(size)
    + (sizeof(struct nlist) * 2), // stroff
    1 + dataNameLength + sizeNameLength, // strsize
  };

  struct nlist symbolList[] = {
    {
      reinterpret_cast<char*>(1), // n_un
      N_SECT | N_EXT, // n_type
      1, // n_sect
      0, // n_desc
      0 // n_value
    },
    {
      reinterpret_cast<char*>(1 + dataNameLength), // n_un
      N_ABS | N_EXT, // n_type
      NO_SECT, // n_sect
      0, // n_desc
      size // n_value
    }
  };

  fwrite(&header, 1, sizeof(header), out);
  fwrite(&segment, 1, sizeof(segment), out);
  fwrite(&sect, 1, sizeof(sect), out);
  fwrite(&symbolTable, 1, sizeof(symbolTable), out);

  fwrite(data, 1, size, out);
  for (unsigned i = 0; i < pad(size) - size; ++i) fputc(0, out);

  fwrite(&symbolList, 1, sizeof(symbolList), out);

  fputc(0, out);
  fwrite(dataName, 1, dataNameLength, out);
  fwrite(sizeName, 1, sizeNameLength, out);
}

} // namespace

int
main(int argc, const char** argv)
{
  if (argc != 4) {
    fprintf(stderr,
            "usage: %s <input file> <data symbol name> <size symbol name>\n",
            argv[0]);
    return -1;
  }

  uint8_t* data = 0;
  unsigned size;
  int fd = open(argv[1], O_RDONLY);
  if (fd != -1) {
    struct stat s;
    int r = fstat(fd, &s);
    if (r != -1) {
      data = static_cast<uint8_t*>
        (mmap(0, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
      size = s.st_size;
    }
    close(fd);
  }

  if (data) {
    writeObject(stdout, data, size, argv[2], argv[3]);
    munmap(data, size);
    return 0;
  } else {
    perror(argv[0]);
    return -1;
  }
}