#include <linux/kernel.h>
#include <linux/rwsem.h>
#include <linux/syscalls.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/uaccess.h>
#include <linux/stringhash.h>
#include <linux/mailbox_421.h>
#include "mailbox_utilities.h"


//statically define semaphore
DECLARE_RWSEM(rwsem);

//statically 

static bool system_initialized = false;
static struct list_head mailboxes;
static mailbox_id_t INVALID_ID = INVALID_MAILBOX_ID;

SYSCALL_DEFINE0(mbox_init_421)
{
	if (system_initialized) {
		return -EALREADY;
	}

	INIT_LIST_HEAD(&mailboxes);
	system_initialized = true;
	return 0;
}

SYSCALL_DEFINE0(mbox_shutdown_421)
{
	mailbox_t *entry;
	mailbox_t *save;

	if (!system_initialized) {
		return -ESHUTDOWN;
	}

	/* Delete each individual mailbox */
	list_for_each_entry_safe (entry, save, &mailboxes, link) {
		list_del(&entry->link);
		XFREE(entry->name);
		XFREE(entry->data);
		XFREE(entry);
	}

	/* Poison the list itself, so it cannot be used again, until init */
	mailboxes.next = LIST_POISON1;
	mailboxes.prev = LIST_POISON2;
	system_initialized = false;
	return 0;
}

SYSCALL_DEFINE3(mbox_open_421, char __user *, name, mailbox_id_t __user *, id,
		uint64_t, size)
{
	

	long error;
	long string_length;
	char *mailbox_name;
	mailbox_t* the_mailbox;

	if (name == NULL)
		return -EINVAL;
	if (id == NULL)
		return -EINVAL;
	if (size == 0)
		return -EINVAL;

	/* The user gave us something weird for an id */
	if (!XACCESS_OK(id, sizeof(mailbox_id_t)))
		return -EFAULT;

	string_length = XSTRNLEN_USER(name, 64);

	/* We have an exception */
	if (string_length == 0)
		return -EFAULT;

	/* String is larger than 64 */
	if (string_length > 64) {
		string_length = 64;
	}

	ALLOCATE(mailbox_name, string_length, error, error_exit);

	if (COPY_FROM_USER(mailbox_name, name, string_length) != 0)
	{
		error = -EFAULT;
		goto cleanup_name;
	}

	/**
	 * This is not required if the string is already terminated
	 * But it is faster than another branch
	 */
	mailbox_name[string_length - 1] = 0x00;


	the_mailbox = find_mailbox_by_name(mailbox_name, &mailboxes);
	
	/**
	 * This is ugly code. Unfortunately we cannot really get around it 
	 * easily as open needs to return a mailbox if it exists
	 */
	if (the_mailbox == NULL) {
		error = create_mailbox(&the_mailbox, mailbox_name, size);
		if (error < 0) {
			goto cleanup_name;
		}

		list_add_tail(&the_mailbox->link, &mailboxes);
	}

	if (COPY_TO_USER(id, &the_mailbox->id, sizeof(mailbox_id_t)) != 0)
	{
		error = -EFAULT;
		goto cleanup_name;
	}

	return 0;

cleanup_name:
	XFREE(mailbox_name);
error_exit:
	return error;
}

SYSCALL_DEFINE4(mbox_write_421, mailbox_id_t __user *, id, uint8_t __user *,
		data, uint64_t, size, uint64_t, offset)
{
	printk("write before lock");
	//dynamically initialize semaphore
	init_rwsem(&rwsem);
	
	printk("write after lock");

	mailbox_id_t* the_id;
	mailbox_t* the_mailbox;
	uint8_t* the_data;
	long error;

	if (id == NULL){
		return -EINVAL;
	}

	if (data == NULL){
		return -EINVAL;
	}

	if (size == 0){
		return -EINVAL;
	}

	if (!XACCESS_OK(id, sizeof(mailbox_id_t))){
		return -EFAULT;
	}

	if (!XACCESS_OK(data, size)){
		return -EFAULT;
	}

	the_id = NULL;
	the_mailbox = NULL;
	the_data = NULL;

	ALLOCATE(the_id, sizeof(id), error, error_exit);

	if (COPY_FROM_USER(the_id, id, sizeof(mailbox_id_t)) != 0)
	{
		error = -EFAULT;
		goto cleanup_id;
	}

	if (*the_id == INVALID_MAILBOX_ID) {
		error = -EINVAL;
		goto cleanup_id;
	}
	
	the_mailbox = find_mailbox_by_id(*the_id, &mailboxes);

	if (the_mailbox == NULL) {
		error = -ENOENT;
		goto cleanup_id;
	}

	if (size + offset > the_mailbox->size) {
		error = -ENOMEM;
		goto cleanup_id;
	}

	ALLOCATE(the_data, size, error, cleanup_data);

	if (COPY_FROM_USER(the_data, data, size) != 0) {
		error = -EFAULT;
		goto cleanup_data;
	}

	//make sure that no one is reading else write lock
	down_write(&rwsem);
	
	memcpy(the_mailbox->data + offset, the_data, size);
	
	up_write(&rwsem);
	
	/**
	 * We let it fall through here, since we do want to clean up all of that
	 */
	error = size;
cleanup_data:
	XFREE(the_data);
cleanup_id:
	XFREE(the_id);
error_exit:
	return error;
}

SYSCALL_DEFINE4(mbox_read_421, mailbox_id_t __user *, id, uint8_t __user *,
		data, uint64_t, size, uint64_t, offset)
{
	//dynamically initialize semaphore
	init_rwsem(&rwsem);


	mailbox_t* the_mailbox;
	long error;

	if (id == NULL){
		return -EINVAL;
	}

	if (data == NULL){
		return -EINVAL;
	}

	if (size == 0){
		return -EINVAL;
	}

	if (!XACCESS_OK(id, sizeof(mailbox_id_t))){
		return -EFAULT;
	}

	if (!XACCESS_OK(data, size)){
		return -EFAULT;
	}

	the_mailbox = NULL;


	if (*id == INVALID_MAILBOX_ID) {
		error = -EINVAL;
		goto error_exit;
	}
	
	the_mailbox = find_mailbox_by_id(*id, &mailboxes);

	if (the_mailbox == NULL) {
		error = -ENOENT;
		goto error_exit;
	}

	if (size + offset > the_mailbox->size) {
		error = -EFAULT;
		goto error_exit;
	}

	//grab semaphore, wait if writing else read
	down_read(&rwsem);
	
	if (COPY_TO_USER(data, the_mailbox->data+offset, size) != 0) {
		error = -EFAULT;
//		up_read(&rwsem);
		goto error_exit;
	}
	
	up_read(&rwsem);
	
	/**
	 * We let it fall through here, since we do want to clean up all of that
	 */
	error = size;
error_exit:
	return error;
}

SYSCALL_DEFINE1(mbox_close_421, mailbox_id_t __user *, id)
{
	mailbox_id_t* the_id;
	mailbox_t* the_mailbox;
	long error;

	if (id == NULL)
		return -EINVAL;
	
	if (!XACCESS_OK(id, sizeof(mailbox_id_t)))
		return -EFAULT;

	ALLOCATE(the_id, sizeof(id), error, error_exit);

	if (COPY_FROM_USER(the_id, id, sizeof(mailbox_id_t)) != 0)
	{
		error = -EFAULT;
		goto error_exit;
	}

	if (*the_id == INVALID_MAILBOX_ID) {
		error = -EINVAL;
		goto cleanup_id;
	}

	the_mailbox = find_mailbox_by_id(*the_id, &mailboxes);

	if (the_mailbox == NULL) {
		error = -ENOENT;
		goto cleanup_id;
	}

	list_del(&the_mailbox->link);
	XFREE(the_mailbox->name);
	XFREE(the_mailbox->data);
	XFREE(the_mailbox);

	UNUSED(COPY_TO_USER(id, &INVALID_ID, sizeof(mailbox_id_t)))

	error = 0;

cleanup_id:
	XFREE(the_id);
error_exit:
	return error;
}
