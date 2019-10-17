#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
int a[32];
unsigned long sum(int n) {
    int i;
    unsigned long sum = 0;

    for (i = 0; i < n; i++) {
        sum = sum + i;
    }

    return sum;
}

int main(void) {

    unsigned long s;

    s = sum(100);
    printf("Hello world, the sum:%ld\n", s);

    return 0;
}