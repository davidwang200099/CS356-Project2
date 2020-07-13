/*this program create a daemon process, 
which will make OOM_killer awaken periodically for some pre-set period T
*/
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <signal.h>
#define T 2


int main(void)
{
    if(daemon(0,0) == -1)
    {
        printf("fail to create daemon process\n");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        //use system call 384, which is oom_killer to check 
        //if a user has run out its memory quota.
        //If that, then it will kill the process that has the highest RSS 
        //among all processes belonging to the user. 
        //syscall(383,10070,100000000);
        syscall(378);
        sleep(T);
    }
    return 0;
}
