#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/syscall.h>
#include <linux/kernel.h>
#include <linux/mailbox_421.h>

#define __NR_mbox_init_421 436
#define __NR_mbox_shutdown_421 437
#define __NR_mbox_open_421 438
#define __NR_mbox_write_421 439
#define __NR_mbox_read_421 440
#define __NR_mbox_close_421 441

long mbox_init_421_syscall() {
    return syscall(__NR_mbox_init_421);
}

long mbox_shutdown_421_syscall() {
    return syscall(__NR_mbox_shutdown_421);
}

long mbox_open_421_syscall(char* name, mailbox_id_t* id, uint64_t size ) {
    return syscall(__NR_mbox_open_421, name, id, size);
}

long mbox_write_421_syscall(mailbox_id_t* id, uint8_t* data, uint64_t size, uint64_t offset) {
    return syscall(__NR_mbox_write_421, id, data, size, offset);
}

long mbox_read_421_syscall(mailbox_id_t* id, uint8_t* data, uint64_t size, uint64_t offset) {
    return syscall(__NR_mbox_read_421, id, data, size, offset);
}

long mbox_close_421_syscall(mailbox_id_t* id) {
    return syscall(__NR_mbox_close_421, id);
}
