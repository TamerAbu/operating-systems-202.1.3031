#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define size 8180

int
main(int argc, char *argv[])
{
    char *mem = sbrk(0);

    sbrk(size);
    strcpy(mem, "hello child");
    int src_pid = getpid();

    int pid = fork();
    if (pid == 0) {     //child proccess
        char* shared_mem = (char*) map_shared_pages(src_pid, getpid(), (uint64)mem, size);
        if (shared_mem == 0) {
            printf("map_shared_pages failed\n");
            exit(1);
        }

        printf("child's recieved message from parent: %s\n", shared_mem);
    }
    else {  // father proccess
        wait(0);
        
    }
    exit(0);
}