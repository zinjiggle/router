#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
    fd_set rfds;
    struct timeval tv,constv;
    int retval;
    char buf[100];
   /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

   /* Wait up to 0.1 seconds. */
    constv.tv_sec = 0;
    constv.tv_usec = 10000;
    tv = constv;
    const int timeout = 50;
    int time = timeout;
    
    while(retval = select(1, &rfds, NULL, NULL, &tv)>=0){
        printf("remain time: %.2f \n",(float)tv.tv_usec/1e6+tv.tv_sec);
	time--;
        if(time==0){
            printf("timeout: do something here\n");
	    time = timeout;
        }
        if (FD_ISSET(0,&rfds)) {
           	fgets(buf,100,stdin);
		 printf("Data is available now: %s\n", buf);
        }
	tv = constv;
	FD_ZERO(&rfds);
	FD_SET(0,&rfds);
       
    }
    /* Don't rely on the value of tv now! */

   if (retval == -1)
        perror("select()");

   exit(EXIT_SUCCESS);
}
