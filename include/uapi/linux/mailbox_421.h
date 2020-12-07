#ifndef MAILBOX_421_H
#define MAILBOX_421_H

#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/limits.h>
#else
#include <stdint.h>
#include <limits.h>
#endif

typedef uint64_t mailbox_id_t;
#define INVALID_MAILBOX_ID ULONG_MAX
#endif