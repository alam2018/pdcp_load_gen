/*
 * rohc_proc.c
 *
 *  Created on: Jan 29, 2018
 *      Author: idefix
 */

/* system includes */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
//#include "socket_msg.h"

#include "platform_types.h"
//#include "rohc_proc.h"


//#include "config.h" /* for HAVE_*_H definitions */

#if HAVE_WINSOCK2_H == 1
#  include <winsock2.h> /* for htons() on Windows */
#endif
#if HAVE_ARPA_INET_H == 1
#  include <arpa/inet.h> /* for htons() on Linux */
#endif

/* includes required to use the compression part of the ROHC library */
#include <rohc/rohc.h>
#include <rohc/rohc_comp.h>


/** The size (in bytes) of the buffers used in the program */
#define BUFFER_SIZE_ROHC 2048
#define FAKE_PAYLOAD "hello, ROHC world!"

#define IP_HEADER_SIZE 20
#define UDP_HEADER_SIZE 8
#define RTP_HEADER_SIZE 12


int create_packet(struct rohc_buf *const packet)
{
	packet->len = IP_HEADER_SIZE + strlen(FAKE_PAYLOAD) +UDP_HEADER_SIZE +RTP_HEADER_SIZE;
	int length = IP_HEADER_SIZE + strlen(FAKE_PAYLOAD) +UDP_HEADER_SIZE +RTP_HEADER_SIZE;
//	packet->len = IP_HEADER_SIZE + sdu_buffer_size_new +UDP_HEADER_SIZE +RTP_HEADER_SIZE;
//	int length = IP_HEADER_SIZE + sdu_buffer_size_new +UDP_HEADER_SIZE +RTP_HEADER_SIZE;

	/* IPv4 header */
	rohc_buf_byte_at(*packet, 0) = 4 << 4; /* IP version 4 */
	rohc_buf_byte_at(*packet, 0) |= 5; /* IHL: minimal IPv4 header length (in 32-bit words) */
	rohc_buf_byte_at(*packet, 1) = 0; /* TOS */
	rohc_buf_byte_at(*packet, 2) = (packet->len >> 8) & 0xff; /* Total Length */
	rohc_buf_byte_at(*packet, 3) = packet->len & 0xff;
	rohc_buf_byte_at(*packet, 4) = 0; /* IP-ID */
	rohc_buf_byte_at(*packet, 5) = 0;
	rohc_buf_byte_at(*packet, 6) = 0; /* Fragment Offset and IP flags */
	rohc_buf_byte_at(*packet, 7) = 0;
	rohc_buf_byte_at(*packet, 8) = 1; /* TTL */
	rohc_buf_byte_at(*packet, 9) = 17; /* Protocol: UDP */
	rohc_buf_byte_at(*packet, 10) = 0xa9; /* fake Checksum */
	rohc_buf_byte_at(*packet, 11) = 0xa0;
	rohc_buf_byte_at(*packet, 12) = 0x01; /* Source address */
	rohc_buf_byte_at(*packet, 13) = 0x02;
	rohc_buf_byte_at(*packet, 14) = 0x03;
	rohc_buf_byte_at(*packet, 15) = 0x04;
	rohc_buf_byte_at(*packet, 16) = 0x05; /* Destination address */
	rohc_buf_byte_at(*packet, 17) = 0x06;
	rohc_buf_byte_at(*packet, 18) = 0x07;
	rohc_buf_byte_at(*packet, 19) = 0x08;

//	 UDP header
	rohc_buf_byte_at(*packet, 20) = 0x42;  //source port
	rohc_buf_byte_at(*packet, 21) = 0x42;
	rohc_buf_byte_at(*packet, 22) = 0x27;  //destination port = 10042
	rohc_buf_byte_at(*packet, 23) = 0x3a;
	rohc_buf_byte_at(*packet, 24) = 0x00;  //UDP length
	rohc_buf_byte_at(*packet, 25) = 8 + 12 + strlen(FAKE_PAYLOAD);
	rohc_buf_byte_at(*packet, 26) = 0x00;  //UDP checksum = 0
	rohc_buf_byte_at(*packet, 27) = 0x00;

//	 RTP header
	rohc_buf_byte_at(*packet, 28) = 0x80;
	rohc_buf_byte_at(*packet, 29) = 0x00;
	rohc_buf_byte_at(*packet, 30) = 0x00;
	rohc_buf_byte_at(*packet, 31) = 0x2d;
	rohc_buf_byte_at(*packet, 32) = 0x00;
	rohc_buf_byte_at(*packet, 33) = 0x00;
	rohc_buf_byte_at(*packet, 34) = 0x01;
	rohc_buf_byte_at(*packet, 35) = 0x2c;
	rohc_buf_byte_at(*packet, 36) = 0x00;
	rohc_buf_byte_at(*packet, 37) = 0x00;
	rohc_buf_byte_at(*packet, 38) = 0x00;
	rohc_buf_byte_at(*packet, 39) = 0x00;

	/* copy the payload just after the IP/UDP/RTP headers */
//
	memcpy(rohc_buf_data_at(*packet, IP_HEADER_SIZE +UDP_HEADER_SIZE +RTP_HEADER_SIZE), FAKE_PAYLOAD, strlen(FAKE_PAYLOAD));
//	memcpy(rohc_buf_data_at(*packet, IP_HEADER_SIZE +UDP_HEADER_SIZE +RTP_HEADER_SIZE), sdu_buffer_new, sdu_buffer_size_new);

	return length;
}
