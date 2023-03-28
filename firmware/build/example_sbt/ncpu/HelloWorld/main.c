#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef struct {
	uint32_t flag;
	uint32_t addr;
	uint32_t len;
} msg_t;

#define MSG_FLAG_NONE		0
#define MSG_FLAG_ACTIVE		1

#define NCPU_MSG_FLAG_ADDR     0x83000000
#define NCPU_MSG_BUF_ADDR      0x83000010
#define NCPU_MSG_BUF_LEN       0x1024


int main( int argc, char **argv )
{
	uint8_t *pmsg_buf;
	uint32_t len, cnt = 0;
	msg_t *msg;
	msg = (msg_t*)NCPU_MSG_FLAG_ADDR;
	msg->flag = 0;
	msg->addr = NCPU_MSG_BUF_ADDR;
	pmsg_buf = (uint8_t *)NCPU_MSG_BUF_ADDR;
	memset((void*)NCPU_MSG_BUF_ADDR, 0, NCPU_MSG_BUF_LEN);
	while(1) {
		if (msg->flag == 0) {
			msg->len = sprintf(pmsg_buf, "Hello! This message %d comes from NCPU \r\n", cnt++);
			msg->flag = 1;
		}
	}
	return 0;
}
