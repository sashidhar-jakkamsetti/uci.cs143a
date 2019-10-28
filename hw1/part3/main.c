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

unsigned long crash_array(int n) {
    int i;
    unsigned long sum = 0;

    // BUG FIX: Added one more condition in the loop to check for the array size bound.
    for (i = 0; i < n && i < sizeof(a)/sizeof(a[0]); i++) {
        sum = sum + a[i];
    }

    return sum;
}

int main(void) {

    unsigned long s;

    s = sum(100);
    printf("Hello world, the sum:%ld\n", s);

    s = crash_array(10000);
    printf("crash array sum:%ld\n", s);
    return 0;
}

