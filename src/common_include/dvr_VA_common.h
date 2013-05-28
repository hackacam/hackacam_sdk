#ifndef STRETCH_DVR_VA_COMMON_H
#define STRETCH_DVR_VA_COMMON_H

typedef struct dvr_va_msg_struct {
	unsigned char status;       // also doubles as 'seq'
	unsigned char job_type;     // must be 0 for host_proxy to work
	unsigned char job_id;       // like a camera id
	unsigned char op_code;      // 1,2,...
	unsigned char data[12];
} dvr_va_msg_t;

#define DVR_VA_OPCODE_CONFIG    1
#define DVR_VA_OPCODE_2         2
#define DVR_VA_OPCODE_SNAPSHOT  3
#define DVR_VA_OPCODE_PAUSE     4
#define DVR_VA_OPCODE_START     DVR_REGION_OPCODE_START_VA     // 5
#define DVR_VA_OPCODE_FINISH    DVR_REGION_OPCODE_FINISH_VA    // 6
#define DVR_VA_OPCODE_RESUME    7
#define DVR_VA_OPCODE_eoPLANE   8
#define DVR_VA_OPCODE_eoYUV     9

#endif /* STRETCH_DVR_VA_COMMON_H */
