#include "types.h"
#include "user.h"
#include "fcntl.h"

int
main(void)
{
    int size = 1024;
    
    // Get the initial value
    //char *paddr = sbrk(0);
    
    // Allocte some memory
    char *saddr = sbrk(size);
    
    // Call wrprotect to remove write permissions
    wrprotect((void *)saddr, size);
    
    // Should give an error
    //*paddr = 1;

    exit();
}