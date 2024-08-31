#include "kernel/types.h"
#include "user/user.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "kernel/fcntl.h"

#include "kernel/crypto.h"

int main(void) {
  if(open("console", O_RDWR) < 0){
    mknod("console", CONSOLE, 0);
    open("console", O_RDWR);
  }
  dup(0);  // stdout
  dup(0);  // stderr

  printf("crypto_srv: starting\n");

  // TODO: implement the cryptographic server here
  /****************************************************** Assignment 3  ******************************************************/
  if (getpid() != 2) {
    printf("crypto_srv: Not runnable from shell\n");
    exit(1);
  }

  while(1){
    void* dst_va;
    uint64 size;
    while (take_shared_memory_request(&dst_va, &size) < 0);

    struct crypto_op* op = (struct crypto_op*)dst_va;

    printf("crypto_srv: Received memory request\n");

    if (op->state != CRYPTO_OP_STATE_INIT) {
      printf("crypto_srv: Error!  Invalid operation state!\n");
      asm volatile ("fence rw,rw" : : : "memory");
      op->state = CRYPTO_OP_STATE_ERROR;
      continue;
    }

    if (op->type == CRYPTO_OP_TYPE_ENCRYPT) {
      printf("crypto_srv: Encrypting the message!\n");
    }
    else if (op->type == CRYPTO_OP_TYPE_DECRYPT) {
      printf("crypto_srv: Decrypting the message!\n");
    }
    else {
      printf("crypto_srv: Error! Unknown crypto operation!\n");
      asm volatile ("fence rw,rw" : : : "memory");
      op->state = CRYPTO_OP_STATE_ERROR;
      continue;
    }

    for (int i = 0; i < op->data_size; i++) {
      op->payload[op->key_size + i] ^= op->payload[i % op->key_size];
    }

    asm volatile ("fence rw,rw" : : : "memory");
    op->state = CRYPTO_OP_STATE_DONE;

    if (remove_shared_memory_request(dst_va, size) < 0) {
      printf("crypto_srv: Failed to remove memory request\n");
    }
  }

  exit(0);
}
