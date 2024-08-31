#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define size 4096

int main(int argc, char *argv[])
{
    printf("Memory before malloc: %d\n", sbrk(0));
    char *mem = malloc(size);

    int father_pid = getpid();

    int pid = fork();
    if (pid == 0) {     //child
        printf("Memory before map: %d\n", sbrk(0));
        char* shared_mem = (char*) map_shared_pages(father_pid, getpid(), (uint64)mem, size);
        if (shared_mem == 0) {
            printf("map_shared_pages failed\n");
            exit(1);
        }
        printf("Memory after map: %d\n", sbrk(0));


        // write hello daddy
        strcpy(shared_mem, "hello daddy");
        unmap_shared_pages(getpid(), (uint64)shared_mem, size);
        printf("Memory after doing unmap: %d\n", sbrk(0));
        // malloc
        char* shared_mem2 = (char*) malloc(size);
        if (shared_mem2 == 0) {
            printf("malloc failed\n");
            exit(1);
        }
        printf("Memory after final malloc : %d\n", sbrk(0));
    }
    else {
        wait(0);
        printf("parent reads from mem: %s\n", mem);
    }

    return 0;;
}
