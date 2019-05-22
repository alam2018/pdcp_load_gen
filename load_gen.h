/*
 * load_gen_lib.h
 *
 *  Created on: Oct 4, 2018
 *      Author: user
 */

#include <stdio.h>
#include "stdbool.h"
#include "platform_types.h"
#include "types.h"

/* includes required to use the compression part of the ROHC library */
#include <rohc/rohc.h>
#include <rohc/rohc_comp.h>

#define ROHC_COMPRESSION
//#undef ROHC_COMPRESSION

#define PAYLOAD_SIZE    1400
#define NUM_TO_MSGDX(n) (n << 16)

#define SDU_BUFFER_SIZE					2500
#define ROHC_BUFFER_SIZE				500
#define MAX_BUFFER_REC_WINDOW 			1
#define MAX_SOCKET_CONNECTION 			100
#define BUFFER_SIZE 					10000
#define PACKET_SIZE						1500
#define NUMBER_OF_PACKETS_SEND			4000

#define TRAFFIC_MODEL_ENABLE
#undef TRAFFIC_MODEL_ENABLE


//******************************************************************************
// Name Message Identifiers
//******************************************************************************

#define MSGID_PDCP_INCOMMING               NUM_TO_MSGDX(1)    // (95536)
#define MSGID_PDCP_OUTGOING                NUM_TO_MSGDX(2)

//******************************************************************************
// Name Message Identifiers
//******************************************************************************

/*
 * Incoming message identifiers
*/

#define PDCP_BASE_MSG_IN_ID             (MSGID_PDCP_INCOMMING)

#define PDCP_RUN_FUNC                (PDCP_BASE_MSG_IN_ID) // (65536)
#define PDCP_DATA_REQ_FUNC			 (PDCP_BASE_MSG_IN_ID + 1)
#define PDCP_DATA_IND				 (PDCP_BASE_MSG_IN_ID + 2)
#define PDCP_REMOVE_UE				 (PDCP_BASE_MSG_IN_ID + 3)
#define RRC_PDCP_CONFIG_REQ	 		 (PDCP_BASE_MSG_IN_ID + 4)
#define PDCP_MODULE_CLEANUP			 (PDCP_BASE_MSG_IN_ID + 5)
#define RLC_STATUS_FOR_PDCP			 (PDCP_BASE_MSG_IN_ID + 6)
#define PDCP_REG_REQ				 (PDCP_BASE_MSG_IN_ID + 7)
#define RRC_PDCP_CONFIG_ASN1_REQ	 (PDCP_BASE_MSG_IN_ID + 8)
#define PDCP_MODULE_INIT_REQ		 (PDCP_BASE_MSG_IN_ID + 9)
#define PDCP_CONFIG_SET_SECURITY_REQ (PDCP_BASE_MSG_IN_ID + 10)
#define PDCP_HASH_COLLEC 			 (PDCP_BASE_MSG_IN_ID + 11)
#define PDCP_GET_SEQ_NUMBER_TEST	 (PDCP_BASE_MSG_IN_ID + 12)

/*
 * Outgoing message identifiers
*/

#define PDCP_BASE_MSG_OUT_ID             (MSGID_PDCP_OUTGOING)

#define RLC_DATA_REQ					 (PDCP_BASE_MSG_OUT_ID)
#define PDCP_REG_RSP					 (PDCP_BASE_MSG_OUT_ID + 1)
#define PDCP_DATA_REQ_RSP				 (PDCP_BASE_MSG_OUT_ID + 2)
#define PDCP_DATA_IND_SEND_RSP			 (PDCP_BASE_MSG_OUT_ID + 3)
#define PDCP_RRC_DATA_IND_RSP			 (PDCP_BASE_MSG_OUT_ID + 4)
#define MAC_ENB_GET_RRC_STATUS_REQ		 (PDCP_BASE_MSG_OUT_ID + 5)
#define MAC_ENB_GET_RRC_STATUS_RSP		 (PDCP_BASE_MSG_OUT_ID + 6)



int isUplink;

typedef struct extMsg
{
    UINT32 msgId; //Indicates the messageID
    UINT32 msgSize; //Indicates the size of the message payload
    UINT8  dataPacket[BUFFER_SIZE]; //Contains the actual Payload
} __attribute__((packed)) EXT_MSG_T;

typedef struct rohc_time
{
	uint64_t sec;   /**< The seconds part of the timestamp */
	uint64_t nsec;  /**< The nanoseconds part of the timestamp */
} __attribute__((packed)) rohc_time;

typedef struct rohc_msg
{
	rohc_time time;  						/**< The timestamp associated to the data */
	uint8_t ipData[ROHC_BUFFER_SIZE];		/**< Contains IP data for ROHC compression and decompression */
	uint8_t dataBuffer[SDU_BUFFER_SIZE];        	/**< The actual buffer data of UE*/
	size_t max_len;       					/**< The maximum length of the buffer */
	size_t offset;        					/**< The offset for the beginning of the data */
	size_t len;           					/**< The data length (in bytes) */
}  __attribute__((packed)) rohc_msg;

typedef struct
{
	 protocol_ctxt_t  ctxt_pP;						//Running context.
	 srb_flag_t srb_flagP;							//Radio Bearer ID
	 rb_id_t rb_id;
	 mui_t muiP;
	 confirm_t confirmP;
	 sdu_size_t sdu_buffer_size;					//sdu_buffer_size Size of incoming SDU in bytes
#ifdef ROHC_COMPRESSION
	 rohc_msg rohc_packet;
#else
	 uint8_t  buffer[SDU_BUFFER_SIZE];				//Buffer carrying SDU
#endif

#ifdef BUFFER_READ_DELAY_REPORT
	 struct timespec packet_send;
#endif

	 pdcp_transmission_mode_t mode;					//flag to indicate whether the userplane data belong to the control plane or data plane or transparent
//	 mem_block_alam pdcp_pdu;						//when RLC is not available and PDCP is in testing mode
	 uint16_t           pdcp_pdu_size;
	 boolean_t pdcp_result;
	 int qci;										//QCI value if the bearer
	 int sliceID;									//Slice ID to which the user belong to
} __attribute__((packed)) PDCP_DATA_REQ_FUNC_T;


typedef struct
{
/*
    protocol_ctxt_t ctxt_pP;						//Running context.
    srb_flag_t srb_flagP;							//Shows if rb is SRB
    MBMS_flag_t MBMS_flagP;							//Tells if MBMS traffic
    rb_id_t rb_id;									//rab_id Radio Bearer ID
    sdu_size_t sdu_buffer_size;						//sdu_buffer_size Size of incoming SDU in bytes
    mem_block_t sdu_buffer;							//sdu_buffer Buffer carrying SDU
*/
    MBMS_flag_t MBMS_flagP;							//Tells if MBMS traffic
	 protocol_ctxt_t  ctxt_pP;						//Running context.
	 srb_flag_t srb_flagP;							//Radio Bearer ID
	 rb_id_t rb_id;
	 mui_t muiP;
	 confirm_t confirmP;
	 sdu_size_t sdu_buffer_size;					//sdu_buffer_size Size of incoming SDU in bytes
#ifdef ROHC_COMPRESSION
	 rohc_msg rohc_packet;
#else
	 uint8_t  buffer[SDU_BUFFER_SIZE];				//Buffer carrying SDU
#endif

#ifdef BUFFER_READ_DELAY_REPORT
	 struct timespec packet_send;
#endif

	 pdcp_transmission_mode_t mode;					//flag to indicate whether the userplane data belong to the control plane or data plane or transparent
//	 mem_block_alam pdcp_pdu;						//when RLC is not available and PDCP is in testing mode
	 uint16_t           pdcp_pdu_size;
	 boolean_t pdcp_result;
	 int qci;
} __attribute__((packed)) PDCP_DATA_IND_T;



//Database of connection information for all the connected entities
typedef struct pdcpBuffer {
	bool 			isBufferUsed;
	UINT32 			msgSize;			//Size of pData
	uint8_t 		*pData;				//contains the original buffer
}_tpdcpBuffer;

typedef struct schedSockbufferHdr {
	int 			sockFD;				//File descriptor for downlink
	int				sockUplink;			//File descriptor for uplink
	UINT32			msgID;
	bool 			isBufferUsed;
	UINT32 			msgSize;			//Size of pData
	uint8_t 		*pData;				//contains the original buffer
}_tSchedBuffer;

_tSchedBuffer activeRequests[MAX_SOCKET_CONNECTION];

//Function decleration
int create_packet(struct rohc_buf *const packet);
