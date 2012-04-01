#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
    fd_set rfds;
    struct timeval tv;
    int retval;

   /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

   /* Wait up to 0.1 seconds. */
    tv.tv_sec = 0.01;
    tv.tv_usec = 0;
    const int timeout = 100;
    int time = timeout;
    
    while(retval = select(1, &rfds, NULL, NULL, &tv)>=0){
        time--;
        if(time==0){
            printf("timeout: do something here\n");
        }
        if (retval) {
            printf("Data is available now.\n");
        }
        
    }
    /* Don't rely on the value of tv now! */

   if (retval == -1)
        perror("select()");

   exit(EXIT_SUCCESS);
}
