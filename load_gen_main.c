/*
 * load_lib_main.c
 *
 *  Created on: Oct 4, 2018
 *      Author: user
 */

#include <stdint.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"
#include "netinet/tcp.h"
#include "sys/socket.h"
#include "sys/ioctl.h"
#include "sys/types.h"
#include "netinet/in.h"
#include "net/if.h"
#include "unistd.h"
#include "arpa/inet.h"
#include <netdb.h>
#include <sys/select.h>
#include <fcntl.h>//non blocking function
#include <sys/time.h>
#include "errno.h"
#include <time.h>

#include <stdbool.h>
#include <sched.h>
#include "signal.h"
#include <pthread.h>


//#include "load_gen.h"
#include "types.h"
#include "platform_types.h"
#include "on_off_common.h"
#include "sleeping.h"
/* includes required to use the compression part of the ROHC library */
#include <rohc/rohc.h>
#include <rohc/rohc_comp.h>



// Socket and socket function related variables
struct sockaddr_in socketAddrPDCP, socketAddrPDCPUplink;
int portno_PDCP, portno_PDCPUplink;
UINT8 *pdcpSendBuffer, *temppdcpSendBuffer;

//Update this value from the split PDCP pdcp_defense.h file
#define INDEX	22
const int qci_5g_value [INDEX] = {1, 2, 3, 4, 65, 66, 67, 75, 5, 6, 7, 8, 9, 69, 70, 79, 80, 81, 82, 83, 84, 85};

void init_db()
{
	int i;
	for (i = 0; i<MAX_SOCKET_CONNECTION; i++)
	{
		activeRequests[i].sockFD = 0;
		activeRequests[i].isBufferUsed = false;
		activeRequests[i].msgID = 0;
		activeRequests[i].msgSize = 0;
		activeRequests[i].pData = (UINT8*)malloc(BUFFER_SIZE);
		if (NULL == activeRequests[i].pData)
		{
			printf(("Failed to allocate memory for 'pdcpReceiveBuffer',QUIT\n"));
			printf("PDCP module start failed!!! \n");
			exit(EXIT_FAILURE);
		}
	}
}

/*!----------------------------------------------------------------------------
*Construction of buffer for sending message
*
------------------------------------------------------------------------------*/
int responseBufferSize;
VOID MsgInsertFunc (
                              UINT32 MsgId ,
                              UINT32 MsgSize,
                              VOID* MsgData,
                              UINT8 **buffer
		             )
{
    EXT_MSG_T lrmExtMsg;

    lrmExtMsg.msgId = MsgId;
    lrmExtMsg.msgSize = MsgSize; //Indicates the size of the message payload
    // Set/re-set the external structure
    memset(lrmExtMsg.dataPacket,0, BUFFER_SIZE);//SRK

    memcpy(lrmExtMsg.dataPacket,MsgData,lrmExtMsg.msgSize);
    memset(*buffer,0,BUFFER_SIZE);

    memcpy(*buffer,&lrmExtMsg.msgId, sizeof(lrmExtMsg.msgId));
    *buffer += sizeof(lrmExtMsg.msgId);

    memcpy(*buffer,&lrmExtMsg.msgSize, sizeof(lrmExtMsg.msgSize));
    *buffer += sizeof(lrmExtMsg.msgSize);

    memcpy(*buffer, lrmExtMsg.dataPacket, lrmExtMsg.msgSize);
    *buffer += lrmExtMsg.msgSize;

    responseBufferSize = (sizeof(lrmExtMsg.msgId))+ (sizeof(lrmExtMsg.msgSize))+ (lrmExtMsg.msgSize);

    return ;
}


static VOID MsgSend(int sendFD)
{
	INT32  retValue = 0;

	int sizeOfMsg = responseBufferSize;
	retValue = send(sendFD,pdcpSendBuffer,sizeOfMsg,0);
	printf("Sent msg on Fd (%d) \n\n",sendFD );


	if (SYSCALLFAIL == retValue)
	{
		printf ("Send failed at socket %d\n", sendFD);
	  perror("send failed");
	}

	// reset the sendBuffer flag
	temppdcpSendBuffer = pdcpSendBuffer;
	// Set to zero.
	responseBufferSize = 0;
	memset(pdcpSendBuffer,0,BUFFER_SIZE);
	return ;

}


uint8_t *pdcpReceiveBuffer,*temppdcpReceiveBuffer;

VOID MsgReceive(INT32 connectedSockFd, int dataBaseID)
{
    EXT_MSG_T ExtRecMsg;
    UINT32	sockExtHeaderSize = sizeof(ExtRecMsg.msgId) + sizeof (ExtRecMsg.msgSize);
	INT32 retValue = -1;
//	sckClose = false;
	// memorise the start address of the send buffer
//	int schedID = findCon(connectedSockFd);
//	temppdcpReceiveBuffer = activeRequests[schedID].pData;

	// reset the receive Buffer flag
	temppdcpReceiveBuffer = pdcpReceiveBuffer;
	memset(pdcpReceiveBuffer,0,BUFFER_SIZE);
	retValue = recv(connectedSockFd,temppdcpReceiveBuffer,sockExtHeaderSize,0); //LRM receives message on templrmReceiverBufer

	if (retValue == SYSCALLFAIL )
	{
		perror("recv");
		printf("recv() returned Error \n");
		retValue = 0;
		return;
	} else
	{

		 // if the Message has some data in the payload
		ExtRecMsg.msgId = ((EXT_MSG_T*)temppdcpReceiveBuffer)->msgId;
		ExtRecMsg.msgSize = ((EXT_MSG_T*)temppdcpReceiveBuffer)->msgSize;
		temppdcpReceiveBuffer += sockExtHeaderSize;


		 if (ExtRecMsg.msgSize)
		 {
			 retValue = recv(connectedSockFd,temppdcpReceiveBuffer, ExtRecMsg.msgSize, 0);
			 if (retValue == SYSCALLFAIL )
			 {
				 perror("recv2");
				 printf("recv() returned Error \n");
			 }
		 }

		 // call message handler here
//		 MsgHandler(ExtRecMsg.msgId, connectedSockFd);

		 PDCP_DATA_REQ_FUNC_T pdcpDataReqMsg;
		 PDCP_DATA_IND_T pdcpUplinkMsg;
			if (ExtRecMsg.msgId == PDCP_DATA_REQ_FUNC)
			{
				memset (&pdcpDataReqMsg, 0, sizeof(PDCP_DATA_REQ_FUNC_T));
				memset (&pdcpUplinkMsg, 0, sizeof(PDCP_DATA_IND_T));
				memcpy (&pdcpDataReqMsg, temppdcpReceiveBuffer, ExtRecMsg.msgSize);

				memcpy (&pdcpUplinkMsg.ctxt_pP, &pdcpDataReqMsg.ctxt_pP, sizeof (protocol_ctxt_t));
				pdcpUplinkMsg.confirmP = pdcpDataReqMsg.confirmP;
				pdcpUplinkMsg.mode = pdcpDataReqMsg.mode;
				pdcpUplinkMsg.muiP = pdcpDataReqMsg.muiP;
				pdcpUplinkMsg.pdcp_pdu_size = pdcpDataReqMsg.pdcp_pdu_size;
				pdcpUplinkMsg.srb_flagP = pdcpDataReqMsg.srb_flagP;
				pdcpUplinkMsg.rb_id = pdcpDataReqMsg.rb_id;
				pdcpUplinkMsg.sdu_buffer_size = pdcpDataReqMsg.sdu_buffer_size;


	#ifdef ROHC_COMPRESSION
				pdcpUplinkMsg.rohc_packet.len = pdcpDataReqMsg.rohc_packet.len;
				pdcpUplinkMsg.rohc_packet.offset = pdcpDataReqMsg.rohc_packet.offset;
				pdcpUplinkMsg.rohc_packet.max_len = pdcpDataReqMsg.rohc_packet.max_len;
				memcpy (pdcpUplinkMsg.rohc_packet.ipData, pdcpDataReqMsg.rohc_packet.ipData, ROHC_BUFFER_SIZE);
		memcpy (pdcpUplinkMsg.rohc_packet.dataBuffer, pdcpDataReqMsg.rohc_packet.dataBuffer, SDU_BUFFER_SIZE);

	#else
		memcpy (&pdcpUplinkMsg.buffer, pdcpDataReqMsg.buffer, SDU_BUFFER_SIZE);
	#endif


				MsgInsertFunc (PDCP_DATA_IND, sizeof (PDCP_DATA_IND_T), &pdcpUplinkMsg, &temppdcpSendBuffer);

			    MsgSend (activeRequests[dataBaseID].sockUplink);
//			    pdcpReceiveBuffer -= sockExtHeaderSize;
			}
	}
}


//Initializing all the parameters for PDCP connection
void set_txt_inp (int countLine, char *val)
{
	switch (countLine)
	{
	case 1:
		printf("PDCP connection parameter reading started\n\n");
		break;

	case 2:
		bzero((char *) &socketAddrPDCP, sizeof(socketAddrPDCP));
		socketAddrPDCP.sin_family = AF_INET;
		socketAddrPDCP.sin_addr.s_addr = inet_addr(val);
		memset(socketAddrPDCP.sin_zero, '\0', sizeof socketAddrPDCP.sin_zero);
		printf ("PDCP IP: %s", val);
		break;

	case 3:
		portno_PDCP = atoi(val);
		socketAddrPDCP.sin_port = htons(portno_PDCP);
		printf ("PDCP Port: %s", val);
		break;

	case 4:
		isUplink = atoi(val);
		printf ("PDCP direction: %s", val);
		break;

	case 5:
		bzero((char *) &socketAddrPDCPUplink, sizeof(socketAddrPDCPUplink));
		socketAddrPDCPUplink.sin_family = AF_INET;
		socketAddrPDCPUplink.sin_addr.s_addr = inet_addr(val);
		printf ("UPlink PDCP IP: %s", val);
		break;

	case 6:
		portno_PDCPUplink = atoi(val);
		socketAddrPDCPUplink.sin_port = htons(portno_PDCPUplink);
		printf ("Uplink PDCP Port: %s", val);
		break;
	 default:
	 {
		 printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		 printf("Unknown msg read from configuration file\n\n\n\n");
		 printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	 }
	 break;
	}
}

//pthread_mutex_t lock;
void * pdcp_packet_generator (void *arg)
{
//	UINT8 *sendBuffer[MAX_SOCKET_CONNECTION];
//	pthread_mutex_lock(&lock);
    int dbID = *((int *)arg);
//    pthread_mutex_unlock(&lock);
//    free (arg);
	UINT8 *sendBuffer;

#ifdef TRAFFIC_MODEL_ENABLE
	int pkt_size[NUMBER_OF_PACKETS_SEND], pkt_wait[NUMBER_OF_PACKETS_SEND];
	on_off_main(pkt_size, pkt_wait);
#endif

//    sleep (1);


//    sendBuffer[dbID] = activeRequests[dbID].pData;
	UINT8 		*pData;
	pData =  (UINT8*) (malloc(BUFFER_SIZE));
	if (NULL == pData)
		{
			printf("PDCP module start failed!!! \n");
			exit(EXIT_FAILURE);
		}

//    sendBuffer = activeRequests[dbID].pData;
    sendBuffer = pData;
//    memset(activeRequests[dbID].pData,0,BUFFER_SIZE);
    memset(pData,0,BUFFER_SIZE);

	PDCP_DATA_REQ_FUNC_T pdcpDataReqFuncMsg;

	pdcpDataReqFuncMsg.ctxt_pP.module_id 	= 'H';
	pdcpDataReqFuncMsg.ctxt_pP.enb_flag 	= true;
	pdcpDataReqFuncMsg.ctxt_pP.module_id 	= 10;
	pdcpDataReqFuncMsg.srb_flagP			= false;
	pdcpDataReqFuncMsg.rb_id				= 5;
	pdcpDataReqFuncMsg.muiP					= 30;
	pdcpDataReqFuncMsg.confirmP				= 40;
	pdcpDataReqFuncMsg.sdu_buffer_size		= PACKET_SIZE;
	pdcpDataReqFuncMsg.mode					= PDCP_TRANSMISSION_MODE_DATA;
	pdcpDataReqFuncMsg.qci					= qci_5g_value [rand () % 11];

	unsigned char *buffer = "NOTE 4: If the keys CK, IK resulting from an EPS AKA run were stored in the fields already available on "
			"the USIM for storing keys CK and IK this could lead to overwriting keys resulting from an earlier run of UMTS AKA. This "
			"would lead to problems when EPS security context and UMTS security context were held simultaneously (as is the case "
			"when security context is stored e.g. for the purposes of Idle Mode Signaling Reduction). Therefore, plastic roaming"
			" where a UICC is inserted into another ME will necessitate an EPS AKA authentication run if the USIM does not support "
			"EMM parameters storage. ";



#ifdef ROHC_COMPRESSION

	/* the buffer that will contain the IPv4 packet to compress */
	uint8_t ip_buffer[ROHC_BUFFER_SIZE];
	struct rohc_buf ip_packet = rohc_buf_init_empty(ip_buffer, ROHC_BUFFER_SIZE);
	int rohc_data_size;
	int total_pkt_size;

	rohc_data_size = create_packet(&ip_packet); //This length contains the size of IP header and fake payload
	total_pkt_size = rohc_data_size + 2 * sizeof (uint64_t) + 3 * sizeof (size_t);


	/* create a fake IP packet for the purpose of this example program */
//	printf("\nbuild a fake IP/UDP/RTP packet\n");

	pdcpDataReqFuncMsg.rohc_packet.len = ip_packet.len; //This length contains the size of IP header and fake payload
	pdcpDataReqFuncMsg.rohc_packet.time.sec = ip_packet.time.sec;
	pdcpDataReqFuncMsg.rohc_packet.time.nsec = ip_packet.time.nsec;
	pdcpDataReqFuncMsg.rohc_packet.max_len = ip_packet.max_len;
	pdcpDataReqFuncMsg.rohc_packet.offset = ip_packet.offset;
	memcpy (pdcpDataReqFuncMsg.rohc_packet.ipData, ip_packet.data, ip_packet.len);
//	memcpy (pdcpDataReqFuncMsg.rohc_packet.ipData, ip_packet.data, ROHC_BUFFER_SIZE);
	memcpy (pdcpDataReqFuncMsg.rohc_packet.dataBuffer, buffer, pdcpDataReqFuncMsg.sdu_buffer_size);
//	memcpy (pdcpDataReqFuncMsg.rohc_packet.dataBuffer, buffer, SDU_BUFFER_SIZE);

#else
	memcpy (&pdcpDataReqFuncMsg.buffer, buffer, pdcpDataReqFuncMsg.sdu_buffer_size);
#endif

    EXT_MSG_T lrmExtMsg;

	// Allocate memory to the send  buffer
//    sendBuffer =  (UINT8*) (malloc(BUFFER_SIZE));
/*	if (NULL == sendBuffer)
		{
			printf("PDCP module start failed!!! \n");
			exit(EXIT_FAILURE);
		}*/
	sdu_size_t test = ((PDCP_DATA_REQ_FUNC_T*)pData)->sdu_buffer_size;
//	test = ((PDCP_DATA_REQ_FUNC_T*)pData)->sdu_buffer_size;
#ifndef TRAFFIC_MODEL_ENABLE
    MsgInsertFunc (PDCP_DATA_REQ_FUNC, sizeof (PDCP_DATA_REQ_FUNC_T), &pdcpDataReqFuncMsg, &sendBuffer);
#endif

    int responseBufferSize = (sizeof(lrmExtMsg.msgId))+ (sizeof(lrmExtMsg.msgSize))+ sizeof (PDCP_DATA_REQ_FUNC_T);


 //   srand(time(0));

	int i;
    for (i= 0; i<NUMBER_OF_PACKETS_SEND; i++)
    {
    	test = ((PDCP_DATA_REQ_FUNC_T*)pData)->sdu_buffer_size;
#ifdef TRAFFIC_MODEL_ENABLE
    	pdcpDataReqFuncMsg.sdu_buffer_size = (sdu_size_t) (pkt_size[i]);

        MsgInsertFunc (PDCP_DATA_REQ_FUNC, sizeof (PDCP_DATA_REQ_FUNC_T), &pdcpDataReqFuncMsg, &sendBuffer);
#endif

//    	if (((EXT_MSG_T*)activeRequests[dbID].pData)->msgId != PDCP_DATA_REQ_FUNC)
//    		printf ("check\n");

//    	int retValue = send(activeRequests[dbID].sockFD,activeRequests[dbID].pData,responseBufferSize,0);
    	int retValue = send(activeRequests[dbID].sockFD,pData,responseBufferSize,0);

        printf("Sent msg on Fd (%d) \n\n",activeRequests[dbID].sockFD );

    	if (SYSCALLFAIL == retValue)
    	{
    		printf ("Send failed at socket %d\n", activeRequests[dbID].sockFD);
 //   	  perror("send failed");
    	}

//    	sendBuffer[dbID] -= responseBufferSize;
//        sendBuffer = activeRequests[dbID].pData;
//        memset(activeRequests[dbID].pData,0,BUFFER_SIZE);
        sendBuffer = pData;
#ifdef TRAFFIC_MODEL_ENABLE
        memset(pData,0,BUFFER_SIZE);
#endif
//    	sendBuffer = activeRequests[dbID].pData;
#ifdef TRAFFIC_MODEL_ENABLE
    	unsigned int t = pkt_wait[i] / 1000;
    	usleep (t);
#else
    	usleep (rand()%100000);
#endif
//    	nanosleep_manually_compensated (pkt_wait[i]);
    }
    pthread_exit(NULL);
}


void * thread_launcher (void *arg)
{
    int noThread = *((int *)arg);

    pthread_t tid[MAX_SOCKET_CONNECTION];
/*    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("Mutex initialization failed.\n");
        return 1;
    }*/
    int i;
    for (i=0; i<noThread; i++)
    {
        if( pthread_create(&tid[i], NULL, pdcp_packet_generator, &i) != 0 )
        {
            printf("Failed to create thread\n");
            exit(EXIT_FAILURE);
        } else
        	usleep (2000);
    }
    for (i=0; i<noThread; i++)
    {
    	(void) pthread_join (tid[i], NULL);
    }
}

static VOID lrmSetNonBlocking(
                              INT32 lrmNonBlockSock // IN
                             )
{
    INT32 lrmNonBlockopts;
    //LOG(("lrmSetNonBlocking \n"));
    //FILE *failure_file_ptr;

    lrmNonBlockopts = fcntl(lrmNonBlockSock,F_GETFL);

    if (0 > lrmNonBlockopts)
		{
			perror("fcntl(F_GETFL)");
			printf("LRM module start failed!!! \n");
			exit(EXIT_FAILURE);
		}

    lrmNonBlockopts = (lrmNonBlockopts | O_NONBLOCK);
    if (0 > fcntl(lrmNonBlockSock,F_SETFL,lrmNonBlockopts))
		{
			perror("fcntl(F_SETFL)");
			printf("LRM module start failed!!! \n");
			exit(EXIT_FAILURE);
		}

	int one = 4;
	unsigned int len = sizeof(one);
	int resOpt = getsockopt(lrmNonBlockSock, 6, TCP_NODELAY, (void*)&one, &len);

	one =1;

	resOpt = setsockopt(lrmNonBlockSock, 6, TCP_NODELAY, &one, sizeof(one));
	resOpt = getsockopt(lrmNonBlockSock, 6, TCP_CORK, (void*)&one, &len);
	resOpt = getsockopt(lrmNonBlockSock, 6, TCP_NODELAY, (void*)&one, &len);
    return;
}



int main (INT32 argc, INT8 **argv)
{
	int noConnection = atoi(argv[1]);
	printf ("Starting PDCP load generator with %d connection", noConnection);

	init_db();

	/* All declarations
	 * might need to change to global. but be careful about thread sharing
	 */
	int fdmax;
	int reuseaddr = true;

	INT32 gConnectSockFd[MAX_SOCKET_CONNECTION], gConnectSockFdUplink[MAX_SOCKET_CONNECTION];

	INT32 sendFD;
	fd_set fdgroup;
	fd_set readFds;
	// This defines the measurement time interval in sec and microsecs.
	int measurem_intvall_s  = 0;
	int measurem_intvall_us = 200;

	FILE* lrmConfFile = 0;
	lrmConfFile = fopen("OAI_sock_file.conf","r");

	int lineCount = 0;
	bool file_param;
	if (lrmConfFile == 0)
	{
		printf("Configuration file not found!!! \n");
		printf("PDCP module connection failed!!! \n\n");
		exit (EXIT_FAILURE);
	} else
	{
		printf("Configuration file found. Now analyzing the inputs \n");
		char line[80];
		while(fgets(line, sizeof(line),lrmConfFile)!=NULL)///something in the file
		{
			if (line[0] == 'S')
			{
				file_param = true;
			}

			if (file_param == true)
			{
				lineCount++;
				set_txt_inp (lineCount, &line[0]);
			}
		}
	}

	//Initialize the master fd_set
	FD_ZERO(&fdgroup);
	FD_ZERO(&readFds);
	int i;
	for (i= 0; i<noConnection; i++)
	{
	    gConnectSockFd[i] = socket(AF_INET, SOCK_STREAM, 0);

	    if (gConnectSockFd[i] < 0)
		{
		  perror("ERROR opening PDCP'socket");
		  printf("OAI module start failed!!! \n");
		  exit (EXIT_FAILURE);
		}

		fflush(stdout);

		// Feeder try to connect to PDCP
		int val = connect(gConnectSockFd[i],(struct sockaddr *) &socketAddrPDCP,sizeof(socketAddrPDCP));
		if ( val == SYSCALLFAIL)
		{
			perror("connect");
			printf("Check if PDCP is available for connection \n");
			printf("OAI module start failed!!! \n");
			exit(EXIT_FAILURE);
		} else
			printf ("Successful connection from OAI to PDCP established\n");

		activeRequests[i].sockFD = gConnectSockFd[i];

		//Allow socket descriptor to be reusable and non blocking
/*		val = setsockopt(gConnectSockFd[i], SOL_SOCKET,  SO_REUSEADDR,	(char *)&reuseaddr, sizeof(reuseaddr));

		if (val < 0)
			{
			  perror("setsockopt() failed");
			  close(gConnectSockFd[i]);
			  printf("OAI module start failed!!! \n");
			  exit(EXIT_FAILURE);
			}*/

		lrmSetNonBlocking(gConnectSockFd[i]);


		if (fdmax < gConnectSockFd[i])
			fdmax = gConnectSockFd[i];


		//Setting the master fd_set
		FD_SET(gConnectSockFd[i],&fdgroup);

	}

	if (isUplink == 1)
	{
		for (i= 0; i<noConnection; i++)
		{


		    gConnectSockFdUplink[i] = socket(AF_INET, SOCK_STREAM, 0);

		    if (gConnectSockFdUplink[i] < 0)
			{
			  perror("ERROR opening PDCP'socket");
			  printf("Uplink PDCP module start failed!!! \n");
			  exit (EXIT_FAILURE);
			}

			fflush(stdout);

			// OAI try to connect to PDCP
			int valUplink = connect(gConnectSockFdUplink[i],(struct sockaddr *) &socketAddrPDCPUplink,sizeof(socketAddrPDCPUplink));
			if ( valUplink == SYSCALLFAIL)
			{
				perror("connect");
				printf("Check if Uplink PDCP is available for connection \n");
				printf("Uplink PDCP module start failed!!! \n");
				exit(EXIT_FAILURE);
			} else
				printf ("Successful connection from OAI to PDCP established\n");

			activeRequests[i].sockUplink = gConnectSockFdUplink[i];

			//Allow socket descriptor to be reusable and non blocking
/*			valUplink = setsockopt(gConnectSockFdUplink[i], SOL_SOCKET,  SO_REUSEADDR,	(char *)&reuseaddr, sizeof(reuseaddr));

			if (valUplink < 0)
				{
				  perror("setsockopt() failed");
				  close(gConnectSockFdUplink[i]);
				  printf("OAI module start failed!!! \n");
				  exit(EXIT_FAILURE);
				}*/

			lrmSetNonBlocking(gConnectSockFdUplink[i]);

			if (fdmax < gConnectSockFdUplink[i])
				fdmax = gConnectSockFdUplink[i];

			//Setting the master fd_set
			FD_SET(gConnectSockFdUplink[i],&fdgroup);
		}
	}

	// Allocate memory to the send  buffer
	pdcpSendBuffer =  (UINT8*) (malloc(BUFFER_SIZE));
	if (NULL == pdcpSendBuffer)
		{
			printf("PDCP module start failed!!! \n");
			exit(EXIT_FAILURE);
		}
	temppdcpSendBuffer = pdcpSendBuffer;

	// Set to zero.
	memset(pdcpSendBuffer,0,BUFFER_SIZE);	// Allocate memory to the send  buffer

	//Start a new thread for launching all the connection threads
	pthread_t launch_thread;
	sleep(2);
	if( pthread_create(&launch_thread, NULL, thread_launcher, &noConnection) != 0 )
	        {
	            printf("Failed to create thread\n");
	            exit(EXIT_FAILURE);
	        }

	// Allocate memory to the receive buffer
	pdcpReceiveBuffer = (UINT8*) (malloc(BUFFER_SIZE));
	if (NULL == pdcpReceiveBuffer)
		{
			printf("PDCP module start failed!!! \n");
			exit(EXIT_FAILURE);
		}

if (isUplink == 1)
{
	while (true)
	{
//		sleep(1);
		struct timeval timeout;
		timeout.tv_sec  = 0;
		timeout.tv_usec = measurem_intvall_us;

		readFds = fdgroup;
		INT32 n = select(fdmax+1,&readFds,NULL,NULL,&timeout);

		if(n < 0)
		  {
			  perror("select() failed");
			  exit(EXIT_FAILURE);
		  }


		for (i=0; i<noConnection; i++)
		{
			if (activeRequests[i].sockFD > 0)
			{
				if (FD_ISSET(activeRequests[i].sockFD,&readFds))
				{
					MsgReceive (activeRequests[i].sockFD, i);

					}
				}

			}

	}
}else
	{
		while (true)
		{
			sleep (5);
		}
	}

	return 0;
}
