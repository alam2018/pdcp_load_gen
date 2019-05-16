/*
 * File common.h
 * Copyright © ENAC, 2013 (Antoine Varet).
 *
 * ENAC's URL/Lien ENAC : http://www.enac.fr/.
 * sourcesonoff's URL : http://www.recherche.enac.fr/~avaret/sourcesonoff/
 * Mail to/Adresse électronique : avaret@recherche.enac.fr et nicolas.larrieu@enac.fr
 *
 
**fr**
 
Cette œuvre est une mise en œuvre du programme sourcesonoff, c'est-à-dire un programme permettant de générer du trafic réseau réaliste à l'aide de sources ON/OFF
Ce programme informatique est une œuvre libre, soumise à une double licence libre
Etant précisé que les deux licences appliquées conjointement ou indépendamment à
 l’œuvre seront, en cas de litige, interprétées au regard de la loi française et soumis à la compétence des tribunaux français ; vous pouvez utiliser l’œuvre, la modifier, la publier et la redistribuer dès lors que vous respectez les termes de l’une au moins des deux licences suivantes :  
-  Soit la licence GNU General Public License comme publiée par la Free Software
     Foundation, dans sa version 3 ou ultérieure ;  
-  Soit la licence CeCILL comme publiée par CeCILL, dans sa version 2 ou ultérieure.

Vous trouverez plus d'informations sur ces licences aux adresses suivantes :
-  http://www.recherche.enac.fr/~avaret/sourcesonoff/gnu_gplv3.txt
     ou fichier joint dans l'archive;
-  http://www.recherche.enac.fr/~avaret/sourcesonoff/Licence_CeCILL_V2-fr.txt
     ou fichier joint dans l'archive.
 
**en**
 
This work is an implementation of the sourcesonoff program, thus a program to generate realistic network data trafic with ON/OFF sources
This library is free software under the terms of two free licences. In case of problem, the licences will be interpreted with the french law and submitted to the competence of the french courts; you can use the program, modify it, publish and redistribute it if you respect the terms of at least one of the next licenses:
-  The GNU Lesser General Public License v3 of the Free Software Fundation,
-  The CeCILL License, version 2 or later, published by CeCILL.
 
See more information about the licenses at:
-  http://www.recherche.enac.fr/~avaret/sourcesonoff/gnu_gplv3.txt or local file;
-  http://www.recherche.enac.fr/~avaret/sourcesonoff/Licence_CeCILL_V2-fr.txt or local file.
    
*/



/*
 * common.h
 *
 *  Created on: 9 oct. 2012
 *      Author: avaret
 */

#ifndef COMMON_H_
#define COMMON_H_

#include "load_gen.h"

/* This port may be used with udp and tcp, if not overloaded */
#define DEFAULT_PORT_NUMBER 9955


//#define bool int
//#define true 1
//#define false 0

/* Pour préciser les fonctions de sortie */
#define EXIT_POINT  __attribute__((noreturn))

/* type MyInteger: (-1 .. 1E15) [unit=ns].
 * -1 generally means "error", 0 can mean INFINITE or just 0 (nul) */
#define myInteger long long

/* PercentMyInt is the symbol for the printf&co functions */
#define PercentMyInt "%lld"

#define NS_IN_SECONDS 1000000000

#define MS_IN_NS 1000000

/* MAX_DATA_IN_DISTRIBDATA limits the number of random data in distributions.
 *  When more data are required, read the data cyclicly... */
#define MAX_DATA_IN_DISTRIBDATA 1000000

/* It defines the value of internal_buffer_size */
#define DEFAULT_INTERNAL_BUFFER_SIZE (1470)

#define DEFAULT_DELAY_PRECISION (50 * MS_IN_NS)

typedef struct {
	unsigned int size;
	myInteger * data;
} Tdistribdata;

typedef Tdistribdata * Pdistribdata;

typedef enum __Edistrib {
	erroneous, constant, uniform, exponential, poisson, normal, weibull, pareto
} Edistrib;

typedef struct {
	Edistrib type;
	myInteger min, max;
	double lambda, k, avg, sigma, alpha, xm;
	char * datafilename;
} Tdistrib;

typedef struct {
	bool defined_by_user;
	unsigned int  number_of_source;

	bool tcp;
	bool receiver;
	char * destination;
	unsigned int port_number;
	unsigned int internal_buffer_size;

	bool ipv4;
	Tdistrib Don, Doff;

	unsigned int turns;
	myInteger delay_before_start, delay_stop_after;

	myInteger udp_delay_precision;
	int tcp_sock_opt;

	unsigned int tcp_max_conn_ign, tcp_max_conn_exit;
	unsigned int udp_max_bitr_ign, udp_max_bitr_exit;

	unsigned int rand_seed;

//	void * next; /* chained list */
} TOneSourceOnOff;

typedef TOneSourceOnOff * POneSourceOnOff;

extern int verbose;

#define debug if(verbose>0) printf
#define debug_verbose if(verbose>1) printf
#define debug_very_verbose if(verbose>2) printf

/* When the user wants the program to run indefinitely */
#define INFINITE_TURN_NUMBER MAX_DATA_IN_DISTRIBDATA

/* And the associed condition macro */
#define is_infinite_turn_number(x) (x.turns == INFINITE_TURN_NUMBER)

void on_off_main(int *pkt_size, int *pkt_time);

#endif /* COMMON_H_ */
