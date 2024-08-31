#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX_PRIMES 100
#define DEFAULT_CHECKERS 3

int is_prime(int n) {
    if (n < 2) return 0;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return 0;
    }
    return 1;
}

void generator(int gen_to_check) {
    int num = 2;
    while (1) {
        if (channel_put(gen_to_check, num) < 0) {
            exit(0); 
        }
        num++;
    }
}

void checker(int id, int gen_to_check, int check_to_print) {
    int num;
    while (1) {
        if (channel_take(gen_to_check, &num) < 0) {
            sleep(1); 
            if (channel_destroy(gen_to_check) < 0) {
                printf("checker %d, pid %d, failed destroying\n", id, getpid());
            } else {
                printf("checker %d, pid %d, destroyed\n", id, getpid());
            }
            sleep(1); 
            exit(0);
        }
        if (is_prime(num)) {
            if (channel_put(check_to_print, num) < 0) {
                sleep(1); 
                if (channel_destroy(gen_to_check) < 0) {
                    printf("checker %d, pid %d, failed destroying\n", id, getpid());
                } else {
                    printf("checker %d, pid %d, destroyed\n", id, getpid());
                }
                sleep(1); 
                exit(0);
            }
        }
    }
}

void printer(int check_to_print) {
    int num;
    int primes_found = 0;
    while (primes_found < MAX_PRIMES) {
        if (channel_take(check_to_print, &num) < 0) {
            sleep(1);
            exit(0);
        }
        printf("Prime %d: %d\n", primes_found + 1, num);
        primes_found++;
    }
    sleep(1); 
    if (channel_destroy(check_to_print) < 0) {
        printf("printer, pid %d, failed destroying\n", getpid());
    } else {
        printf("printer, pid %d, destroyed\n", getpid());
    }
    sleep(1); 
    exit(0);
}

void wait_for_children(int num_checkers) {
    for (int i = 0; i < num_checkers + 1; i++) {  
        wait(0);
    }
}

int main(int argc, char *argv[]) {
    int num_checkers = (argc > 1) ? atoi(argv[1]) : DEFAULT_CHECKERS;

    while (1) {
        int gen_to_check = channel_create();
        int check_to_print = channel_create();
        if (gen_to_check < 0 || check_to_print < 0) {
            printf("Failed to create channels\n");
            exit(1);
        }

        int pid;

        if ((pid = fork()) == 0) {
            generator(gen_to_check);
            exit(0);
        }

        for (int i = 0; i < num_checkers; i++) {
            if ((pid = fork()) == 0) {
                checker(i + 1, gen_to_check, check_to_print);
                exit(0);
            }
        }

        if ((pid = fork()) == 0) {
            printer(check_to_print);
            exit(0);
        }

        wait_for_children(num_checkers);

        sleep(1); 
        if (channel_destroy(gen_to_check) < 0) {
            printf("generator, pid %d, failed destroying\n", getpid());
        } else {
            printf("generator, pid %d, destroyed\n", getpid());
        }

        
        char buffer[10];
        printf("Restart the system? (y/n): ");
        gets(buffer, sizeof(buffer));
        if (buffer[0] != 'y' && buffer[0] != 'Y') {
            printf("System exit.\n");
            break;
        }

       
        sleep(1);
    }

    return 0;
}
