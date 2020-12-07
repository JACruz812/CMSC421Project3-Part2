#include "test.h"

void main(){
	mbox_init_421_syscall();
	mailbox_id_t* this_id = malloc(sizeof(mailbox_id_t));
	uint8_t* read_data = malloc(sizeof(uint8_t));
	mbox_open_421_syscall("tony",this_id,10);
	

	printf("\n\nTESTS FOR MAILBOX READ\n");
	char* data = "hello";
	mbox_write_421_syscall(this_id,data,5,0);
	mbox_read_421_syscall(this_id,read_data,5,0);
	printf("read result: %s\n", read_data);

	uint8_t data2 = 2;

	mbox_write_421_syscall(this_id,&data2,sizeof(uint8_t),0);
	mbox_read_421_syscall(this_id,read_data,sizeof(uint8_t),0);
	printf("read result: %d\n", *read_data);
	perror("Error value:" );
	
	

	mbox_close_421_syscall(this_id);
	mbox_shutdown_421_syscall();
	free(this_id);
	free(read_data);
}
