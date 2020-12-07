#include "mailbox_utilities.h"

#ifdef __KERNEL__
#include <linux/string.h>
#include <linux/stringhash.h>
#else
uint64_t my_hash(char* name) {
	uint64_t hash = 5381;
	int c;

	while ((c = *name++) != 0)
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}
#endif

mailbox_t *find_mailbox_by_name(const char *name,
				struct list_head *mailbox_list)
{
	mailbox_t *pos;

	list_for_each_entry (pos, mailbox_list, link) {
		if (strcmp(name, pos->name) == 0) {
			return pos;
		}
	}

	return NULL;
}

mailbox_t *find_mailbox_by_id(mailbox_id_t id, struct list_head *mailbox_list)
{
	mailbox_t *pos;

	list_for_each_entry (pos, mailbox_list, link) {
		if (pos->id == id) {
			return pos;
		}
	}

	return NULL;
}

long create_mailbox(mailbox_t **mbox_ptr, char *name, uint64_t size)
{
	mailbox_t *the_mailbox;
	uint8_t *data;
	long error;
	uint8_t string_length;

	ALLOCATE(data, size, error, cleanup_exit);
	ALLOCATE(the_mailbox, sizeof(mailbox_t), error, cleanup_data);

	string_length = strlen(name);

	the_mailbox->name = name;
	the_mailbox->name_length = string_length;
	the_mailbox->data = data;
	the_mailbox->size = size;
	the_mailbox->id = XHASH(the_mailbox, the_mailbox->name,
					 the_mailbox->name_length);
					 
	*mbox_ptr = the_mailbox;
	return 0;

cleanup_data:
	XFREE(data);
cleanup_exit:
	return error;
}