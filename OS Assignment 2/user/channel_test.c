#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void)
{
    int cd = channel_create();
    if (cd < 0)
    {
        printf("Failed to create channel\n");
        exit(1);
    }
    if (fork() == 0)
    {
        if (channel_put(cd, 42) < 0)
        {
            printf("Failed to put data 42 in channel\n");
            exit(1);
        } 
        if (channel_put(cd, 43) < 0){      
            printf("Failed to put data 43 in channel\n");
            exit(1);
        } 
        if(channel_destroy(cd) < 0){
            printf("Failed to destroy channel\n");
            exit(1);
        }
    }
    else
    {
        int data;
        if (channel_take(cd, &data) < 0)
        { // 42
            printf("Failed to take data 42 from channel\n");
            exit(1);
        } else{
            printf("take , data : %d (42)\n",data);
        }
        data = channel_take(cd, &data); // 43
        if (data < 0){
            printf("Failed to take data 43 from channel\n");
            exit(1);
        }
        else{
            printf("take , data : %d (43)\n",data);
        }
        data = channel_take(cd, &data); // Sleep until child destroys channel
        if (data < 0){
            printf("Failed to take because channel is destroyed (this is what we wanted)\n");
            exit(1);
        }
        else{
            printf("this is awkward....\n");
        }
    }
    exit(0);
}
