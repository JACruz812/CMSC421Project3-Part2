#ifndef MAILBOX_UTILITIES_H
#define MAILBOX_UTILITIES_H

#ifdef __KERNEL__
	#include <linux/list.h>
	#include <linux/limits.h>
	#include <linux/slab.h>
    #include <linux/uaccess.h>
	#include <linux/types.h>
	#define XALLOC(size) kmalloc(size, GFP_KERNEL)
	#define XFREE(ptr) kfree(ptr)
	#define XACCESS_OK(ptr, size) access_ok(ptr, size)
	#define XHASH(salt, name, len) full_name_hash(salt, name, len)
	#define XSTRNLEN_USER(str, size) strnlen_user(str, size)
	#define COPY_TO_USER(to, from, size) copy_to_user(to, from, size)
	#define COPY_FROM_USER(to, from, size) copy_from_user(to, from, size)
#else
	#include <string.h>
	#include <stdlib.h>
	#include <limits.h>
	#include <stdint.h>
	#include "list.h"
	#define XALLOC(size) malloc(size)
	#define XFREE(ptr) free(ptr) 
	#define XSTRNLEN_USER(str, size) strnlen(str, size) + 1
	#define XACCESS_OK(ptr, size) 1
	extern uint64_t my_hash(char* name);
	#define XHASH(salt, name, len) my_hash(name)
	#define COPY_TO_USER(to, from, size) ({memcpy(to, from, size); 0;})
	#define COPY_FROM_USER(to, from, size) ({memcpy(to, from, size); 0;})
#endif
#include <linux/mailbox_421.h>

typedef struct mailbox {
	char *name;
	uint8_t name_length;
	uint8_t *data;
	uint64_t size;
	mailbox_id_t id;
	struct list_head link;
} mailbox_t;

mailbox_t* find_mailbox_by_name(const char* name, struct list_head* mailbox_list);
mailbox_t* find_mailbox_by_id(mailbox_id_t id, struct list_head* mailbox_list);

long create_mailbox(mailbox_t** mbox_ptr, char* name, uint64_t size);

/**
 * It will try to kmalloc @ptr to @size. If the allocation fails it will
 * set @error_var to -ENOMEM then jump to the @jump label
 * 
 * @ptr: The variable that needs to be kmalloc'd
 * @size: The size of @ptr in bytes
 * @error_var: The variable that will contain the error if allocation fails
 * @jump: The label where it will jump upon failure
 */
#define ALLOCATE(ptr, size, error_var, jump)                                   \
	do {                                                                   \
		ptr = XALLOC(size);                                            \
		if (ptr == NULL) {                                             \
			error_var = -ENOMEM;                                   \
			goto jump;                                             \
		}                                                              \
	} while (0)

#define UNUSED(x)  if (x) {}
#endif