/*
 * on_off_main.c
 *
 *  Created on: Apr 17, 2019
 *      Author: user
 */

#include <stdint.h>
#include <stdlib.h>
#include "stdio.h"
#include "string.h"
#include "unistd.h"
#include "time.h"

#include "on_off_common.h"
#include "distribs.h"
//#include "load_gen.h"

//POneSourceOnOff completeSetOfSourcesToUse = NULL;

int dry_run = 1;

void goChild(TOneSourceOnOff src, int *pkt_size, int *pkt_time)
{
	Pdistribdata dataOn, dataOff;
//	if(src.rand_seed!=0)
//		srand(src.rand_seed);
//	else
//		srand((unsigned int) rand() +src.number_of_source);

	srand48((long)time(NULL));
	if (dry_run || !src.receiver) {
		dataOn  = getDistribution(src.turns, src.Don, true, pkt_size, pkt_time);
		dataOff = getDistribution(src.turns, src.Doff, false, pkt_size, pkt_time);
	} else {
		dataOn  = NULL;
		dataOff = NULL;
	}

	if (dry_run) {
		printf("Source #%d: ", src.number_of_source);
		if(src.receiver)
			printf("receiver ");
		else
			printf("transmitter (to <%s>) ", src.destination);

		if(src.tcp)
			printf("TCP (VBR) ");
		else
			printf("UDP (CBR) ");

//		printDistributions(src.Don, dataOn, 1, 0);
//		printDistributions(src.Doff, dataOff, 0, 0);
	} else {
//		nanosleep_manually_compensated(src.delay_before_start, NULL);

//		runNetworkWithDistributions(src, dataOn, dataOff);
	}

//	free_distribdata(dataOn);
//	free_distribdata(dataOff);
}


POneSourceOnOff initializeOneSourceOnOff(POneSourceOnOff nextOne)
{
	POneSourceOnOff newsource = malloc(sizeof(TOneSourceOnOff));
	memset(newsource, 0, sizeof(TOneSourceOnOff));

//	memcpy(newsource, nextOne, sizeof(TOneSourceOnOff));
//	if (nextOne)
//		newsource->number_of_source = nextOne->number_of_source + 1;
//	else
//		newsource->number_of_source = 1;

	newsource->number_of_source = 80; //No of TCP connection towards the PDCP
	newsource->receiver 		= true;
	newsource->ipv4 			= true;

//	newsource->Don.type			= weibull;
	newsource->Don.type			= pareto;
	newsource->Don.max 			= 1500; /* in bytes per timeunit */
	newsource->Don.min 			= 50; /* in bytes per timeunit */
	newsource->Don.lambda 		= 0.45;
	newsource->Don.k 			= 0.74;
	newsource->Don.avg 			= (double) newsource->Don.max * 0.5;
	newsource->Don.sigma 		= (double) newsource->Don.max * 0.1;
//	newsource->Don.alpha 		= 0.36;
	newsource->Don.alpha 		= 0.16;
	newsource->Don.xm 			= 4.0;

//	newsource->Doff.type		= poisson;
	newsource->Doff.type		= normal;
	newsource->Doff.max 		= (myInteger) (0.1 * MS_IN_NS); /* in ns */
	newsource->Doff.min 		= (myInteger) (.001*MS_IN_NS); /* in ns */
	newsource->Doff.lambda 		= 0.05;
	newsource->Doff.k 			= 0.74;
	newsource->Doff.avg 		= (double) newsource->Doff.max * 0.5;
	newsource->Doff.sigma 		= (double) newsource->Doff.max * 0.1;
	newsource->Doff.alpha 		= 1.0;
	newsource->Doff.xm 			= 1.0;

	newsource->udp_delay_precision 		= DEFAULT_DELAY_PRECISION;
	newsource->internal_buffer_size 	= DEFAULT_INTERNAL_BUFFER_SIZE;
	newsource->port_number 				= DEFAULT_PORT_NUMBER;

	newsource->turns 					= NUMBER_OF_PACKETS_SEND;

//	newsource->next 					= (void *) nextOne;
	return newsource;
}

/*void createNewSourcesSet()
{
	completeSetOfSourcesToUse = initializeOneSourceOnOff(
			completeSetOfSourcesToUse);
}*/


void on_off_main(int *pkt_size, int *pkt_time)
{
	setbuf(stdout, NULL);
	srand((unsigned int) time(NULL));

//	createNewSourcesSet();
//	parseCmdLine(argc, argv);
	printf("\nSources ON/OFF generator\n");
//	checkConsistencyOfAllSourcesOnOff();
//	startAllSources();
	POneSourceOnOff completeSetOfSourcesToUse = NULL;
	completeSetOfSourcesToUse = initializeOneSourceOnOff(
				completeSetOfSourcesToUse);

	POneSourceOnOff src = completeSetOfSourcesToUse;
	TOneSourceOnOff current_src;

	current_src = *src;

	goChild(current_src, pkt_size, pkt_time);
}
