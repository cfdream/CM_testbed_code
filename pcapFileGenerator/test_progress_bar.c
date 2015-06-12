#include <stdio.h>
 
int main() {
    int i = 0;
 
    printf("Count is: %2d", i);
    for (;i<60;i++) {
        //printf("Count is: %2d", i);
        printf("\b\b%2d",i); 
        //fflush(stdout);
        sleep(1);
    }
                             
    return 0;
}
