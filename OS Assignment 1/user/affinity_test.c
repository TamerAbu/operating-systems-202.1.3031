#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    int pid = getpid();
    int mask = 0b101; // Allow running only on CPU 0

    printf("Setting affinity mask of process %d to %d\n", pid, mask);
    if (set_affinity_mask(mask) < 0) {
        printf("Error setting affinity mask\n");
        exit(1,"");
    }

    for(;;) {
        // You may need to implement or replace getcpuid with the correct function
        printf("Process %d is running\n", pid);
        sleep(10);
    }

    exit(0,"");
}