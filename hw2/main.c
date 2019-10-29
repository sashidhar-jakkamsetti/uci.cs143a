#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <sys/mman.h>

#include <sys/types.h>
#include <unistd.h>

// Format of an ELF executable file

#define ELF_MAGIC 0x464C457FU // "\x7FELF" in little endian

// File header
struct elfhdr
{
  unsigned int magic; // must equal ELF_MAGIC
  unsigned char elf[12];
  unsigned short type;
  unsigned short machine;
  unsigned int version;
  unsigned int entry;
  unsigned int phoff;
  unsigned int shoff;
  unsigned int flags;
  unsigned short ehsize;
  unsigned short phentsize;
  unsigned short phnum;
  unsigned short shentsize;
  unsigned short shnum;
  unsigned short shstrndx;
};

// Program section header
struct proghdr
{
  unsigned int type;
  unsigned int off;
  unsigned int vaddr;
  unsigned int paddr;
  unsigned int filesz;
  unsigned int memsz;
  unsigned int flags;
  unsigned int align;
};

// Values for Proghdr type
#define ELF_PROG_LOAD 1

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC 1
#define ELF_PROG_FLAG_WRITE 2
#define ELF_PROG_FLAG_READ 4

int main(int argc, char *argv[])
{
  struct elfhdr elf;
  struct proghdr ph;
  int (*sum)(int a, int b);
  void *entry = NULL;
  void *code_va = NULL;
  int ret;
  int i;

  FILE *file = fopen(argv[1], "r");

  if (file != NULL)
  {
    // 1. Read the ELF header
    fread(&elf, 1, sizeof(elf), file);

    if (elf.magic == ELF_MAGIC)
    {
      // 2. Go the Program Header Table offset
      fseek(file, elf.phoff, SEEK_SET);

      // 3. Read each entry of the program header table
      for (i = 0; i < elf.phnum; i++)
      {
        fread(&ph, 1, sizeof(ph), file);
        if (ph.type == ELF_PROG_LOAD)
        {
          // 4. Load segment into the memory
          fseek(file, ph.off, SEEK_SET);
          code_va = mmap(code_va, ph.memsz,
                         PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE,
                         0, 0);
          fread((void *)code_va, 1, ph.memsz, file);
          fseek(file, elf.phoff + ((i + 1) * sizeof(ph)), SEEK_SET);

          if (code_va != (void *)-1)
          {
            // 5. Jump to the entry point of the program
            entry = code_va;
          }
        }
      }
    }
  }

  if (entry != NULL)
  {
    sum = entry;
    ret = sum(1, 2);
    printf("sum:%d\n", ret);
  };

  return 0;
}