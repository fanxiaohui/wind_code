

#ifndef XLINK_PROCESS_TCP_DATA_H_
#define XLINK_PROCESS_TCP_DATA_H_
#ifdef  __cplusplus
extern "C" {
#endif

#include "xlink_type.h"
#include "xlink_client.h"
#include "xlink_data.h"
#include "xlink_tcp_client.h"

#if __ALL_DEVICE__
#include "XlinkByteQueue.h"
extern ByteQueue sg_queue;
#endif

extern XLINK_FUNC void xlink_process_tcp_data(unsigned char * data, x_int16 datalen,x_uint32 bodylen);
extern XLINK_FUNC void xlink_process_tcp_notify(unsigned char * Buffer, unsigned int BufferLen);

#ifdef  __cplusplus
}
#endif
#endif /* XLINK_PROCESS_TCP_DATA_H_ */
