#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("Initial memory size: %d bytes\n",memsize());
  char* mem = malloc(20*1024); //allocate 20k
  if(mem==0){
    printf("Failed to allocate mem\n");
    exit(1,"");
  }
  printf("memory size after allocation : %d bytes \n",memsize());
  free(mem);

  int afterFree = memsize();
  printf("memory size after free: %d bytes \n",afterFree);
  exit(0,"");
}
