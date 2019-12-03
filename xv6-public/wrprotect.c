#include "types.h"
#include "user.h"
#include "fcntl.h"

int
main(void)
{
    int size = 100;
    
    // Allocate some memory
    char *saddr = sbrk(size);
    
    // Call wrprotect to remove write permissions
    wrprotect((void *)saddr, size);
    
    // Should give an error
    char *paddr = saddr;
    *paddr = 1;
    // And continue once the page is made writable
    printf(1, "Program resumed. paddr has value: %d\n", *paddr);

    exit();
}