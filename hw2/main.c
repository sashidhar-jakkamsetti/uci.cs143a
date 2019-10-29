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

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian

// File header
struct elfhdr {
  unsigned int magic;  // must equal ELF_MAGIC
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
struct proghdr {
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
#define ELF_PROG_LOAD           1

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC      1
#define ELF_PROG_FLAG_WRITE     2
#define ELF_PROG_FLAG_READ      4

int main(int argc, char* argv[]) {
    struct elfhdr elf;
    struct proghdr ph;
    int (*sum)(int a, int b);
    void *entry = NULL;
    void *code_va = NULL;
    int ret; 

    FILE* file = fopen("elf", "rb");
    
    if(file) {
      // 1. Read the ELF header
      fread(&elf, 1, sizeof(elf), file);

      if(elf.magic == ELF_MAGIC &&
         elf.elf[0] == 'E' &&
         elf.elf[1] == 'L' &&
         elf.elf[2] == 'F' &&
         elf.type == 0x7f) {
            // 2. Go the Program Header Table offset
            fseek(file, elf.phoff - elf.ehsize, SEEK_SET);

            for(int i = 0; i < elf.phnum; i++) {
                fread(&ph, 1, sizeof(ph), file);
                if(ph.type == ELF_PROG_LOAD) {
                    code_va = mmap(code_va, ph.memsz, 
                        PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 
                        0, 0);
                    if(code_va != -1) {
                      entry = code_va + elf.entry;
                    }
                }
            }
         }
    }

    if (entry != NULL) {
        sum = entry; 
        ret = sum(1, 2);
        printf("sum:%d\n", ret); 
    };


}

