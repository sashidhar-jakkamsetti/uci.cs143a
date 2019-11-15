#include "console.h"

static inline void lcr3(unsigned int val)
{   
  asm volatile("movl %0,%%cr3" : : "r" (val));
}

static inline void halt(void)
{
    asm volatile("hlt" : : );
}

static unsigned int ptd[1024]  __attribute__ ((aligned (4096)));
static unsigned int pt[2][1024]  __attribute__ ((aligned (4096)));

int main(void)
{
    int i; 
    int sum = 0;

    int j;
    int cmem = 0;

    // Initialize the console
    uartinit(); 

    printk("Hello from C\n");
    
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 1024; j++) {
            pt[i][j] = (unsigned int)(cmem | 0b00000011);
            cmem += 0x1000;
        }
        ptd[i] = (unsigned int)&pt[i][0] | 0b00000011;
    }
    
    lcr3((unsigned int)&ptd[0]);

    for (i = 0; i < 64 /*64*/; i++) {
        int *p = (int *)(i * 4096 * 32);
        sum += *p; 
                
        printk("page\n"); 
    }
    halt(); 
    return sum; 
}

