/*
 * File distribs.c
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
 * distribs.c
 *
 *  Created on: 5 oct. 2012
 *      Author: avaret
 */

#include "stdlib.h"
#include "stdio.h"
#include "time.h"


#include "distribs.h"

#define _XOPEN_SOURCE

/* FIXME Poisson's distrib is badly implemented */
#define Coeff_Poisson_adjust 10.0
#define BOUND_RESEARCH_TRIES 1000

extern int pkt_size[NUMBER_OF_PACKETS_SEND], pkt_wait[NUMBER_OF_PACKETS_SEND];

long double __lambda, __k, __alpha, __xm; /* local copies to speed calculus */

/* Returns x if (min < x < max), else return min or max (nearest of x) */
myInteger bound_min_max(long double x, myInteger min, myInteger max)
{
	myInteger y = (myInteger) (x);
	if(y<min)
		return min;
	else if (y>max)
		return max;
	else
		return y;
}

/* Generalized uniform random generator of double numbers in range [0;1] */
long double drand(void)
{
//	srand48((long)time(0));
	long double p;
//	do
	{
		double x = drand48();
//		double x = rand();
		p = (long double) x;
	} ;//while (p==0);
	return p;
}


/* Polar form of the Box-Muller transformation */
long double get_random_gaussian(void)
{
	long double x1, x2, w;
	do {
		x1 = 2.0 * drand() - 1.0;
		x2 = 2.0 * drand() - 1.0;
		w = x1 * x1 + x2 * x2;
	} while( w > 1.0);
	return x1 * sqrtl( -2.0 * logl(w) / w );
}


/* Knuth 's algorithm, do NOT use for large lambda values */
int get_random_poisson(void)
{
	int i = 0;
	long double curr = 1;
	long double lim = expl(0.0 - __lambda);
	do {
		i++;
		curr *= drand();
	} while((i<BOUND_RESEARCH_TRIES)&&(curr>=lim));
	return i - 1;
}

long double get_random_weibull(void)
{
	return (__k) * expl( (1.0 / __lambda) * logl( 0.0 - logl( drand() ) ) );
}

long double get_random_exp(void)
{
/* FIXME exp may not work. Use get_random_weibull with adequate parameters instead ? */
	return logl( __lambda / drand() ) / __lambda;
}

long double get_random_pareto()
{
	return __xm / powl( (1.0 - drand()) , 1.0 / __alpha );
}

/* Allocate a struct for DistribData, with "number" items inside */
Pdistribdata malloc_distribdata(unsigned int number)
{
	Pdistribdata result = malloc(sizeof(Tdistribdata));
	result->data = malloc(number * sizeof(myInteger));
	result->size = number;
	memset(result->data, 0, number * sizeof(myInteger));
	return result;
}


/* Free the memory associated with a distribution */
void free_distribdata(Pdistribdata d)
{
	if(d)
	{
		free(d->data);
		free(d);
	}
}

/* See initialize_irand() for explanations */
myInteger mask_max, __max;

/* Initialize the uniform random number generator with the new maximum */
void initialize_irand(myInteger max)
{
	/* Mask_max is the equivalent of max with all LSB set to 1 until the first MSB at 1.
	 * For example:
	 *  init_irand(00001) => mask_max = 00001
	 *  init_irand(00010) => mask_max = 00011
	 *  init_irand(00011) => mask_max = 00011
	 *  init_irand(00101) => mask_max = 00111
	 *  init_irand(00010010101011) => mask_max = 00011111111111
	 */
	double next_integer_after_log2_max_as_double = ceil(log2((double)max));
	int next_integer_after_log2_max_as_int = (int) next_integer_after_log2_max_as_double;
	mask_max = (1 << next_integer_after_log2_max_as_int) - 1;
	__max = max;
}

/* Random generator of integer, uniform distrib between [0..__max], see  initialize_irand() */
myInteger irand(void)
{
	myInteger try;
	do {
		try = random() & mask_max;
	} while (try >= __max);
	return try;
}

/* getDistribution generates "numberTurns" random numbers, conforming to "distrib",
 * and store the result into "result".
 */
Pdistribdata getDistribution(unsigned int numberTurns, Tdistrib distrib, bool don, int *size, int *time)
{
	unsigned int i;
	unsigned int step;
	Pdistribdata  result;

	/* Bound numberTurns with MAX_DATA_IN_DISTRIBDATA */
	if(numberTurns>MAX_DATA_IN_DISTRIBDATA)
		numberTurns = MAX_DATA_IN_DISTRIBDATA;

	/* If a file has been specified, go to the function to read it */
	if(distrib.datafilename != NULL)
		return getDistributionFromFile(numberTurns, distrib.datafilename);

	/* Else, no file has been defined, so we have to compute random values */

	printf("Computation of %d number with a distrib of type=%d...        ",
			numberTurns, distrib.type);

	/* Prepare the uniform random generation */
	initialize_irand(distrib.max - distrib.min);

	/* Prepare the result struct */
	result = malloc_distribdata(numberTurns);

	/* Prepare to compute the numbers */
	__lambda 	= distrib.lambda;
	__k 		= distrib.k;
	__xm 		= distrib.xm;
	__alpha 	= distrib.alpha;
	step = numberTurns / 10;
	if(step<1)
		step = 1;

	/* Make all generation */
	for(i=0;i<numberTurns;i++)
	{
		if(i%step==0)
			printf("\b\b\b\b\b\b\b\b%d0%% ... ", i/step);

		switch(distrib.type) {
		case constant:
			result->data[i] = distrib.max;
			break;
		case uniform:
			result->data[i] = distrib.min + irand();
			break;
		case normal:
			result->data[i] = bound_min_max(distrib.avg + distrib.sigma*get_random_gaussian(),
					distrib.min, distrib.max);
			break;
		case poisson:
		{
			int poiss = get_random_poisson();
			result->data[i] = bound_min_max(( (long double) distrib.min +
					(long double) (distrib.max - distrib.min)
					* (1.0 * poiss / Coeff_Poisson_adjust)),
					distrib.min, distrib.max);
			break;
		}
		case weibull:
			result->data[i] = bound_min_max(get_random_weibull(), distrib.min, distrib.max );
			break;
		case exponential:
			result->data[i] = bound_min_max(get_random_exp(), distrib.min, distrib.max );
			break;
		case pareto:
			result->data[i] = bound_min_max(get_random_pareto(), distrib.min, distrib.max );

			break;
		default:
			exit(-1);
		}

		if (don)
		{
			//Store size of the pkt
			int temp = (int) result->data[i];
			(*size) = temp;
			size++;
		} else
		{
			//store suration of the pkt
			int temp = (int) result->data[i];
			(*time) = temp;
			time++;
		}
	}
	printf("\b\b\b\b\b\b\b\b done.   \n");
	return result;
}

/* printDistributions prints the distrib "result" and its parameter "distrib" on the screen */
void printDistributions(Tdistrib distrib, Pdistribdata result, int don, int printrawdata)
{
	unsigned int i;
	unsigned int count[PRINT_DISTRIB_COUNT_NB+1];
	myInteger distrib_sum = 0;
	int val;

	if(result==NULL)
	{
		printf("\nNo data to be printed\n");
		return ;
	}

	printf("\n\nParameters of the %s generation\n"
			"Type=%d, min=" PercentMyInt ", max=" PercentMyInt "\n"
			"Lambda=%g, k=%g, alpha=%g, xm=%g, avg=%g, sigma=%g\n",
			DonOffStr(don), distrib.type, distrib.min, distrib.max,
			distrib.lambda, distrib.k, distrib.alpha,
			distrib.xm, distrib.avg, distrib.sigma);

	for(i=0;i<result->size;i++)
		distrib_sum = distrib_sum + result->data[i];
	printf("Internal stats of the %s generation: %d values, mean=%g\n",
		DonOffStr(don), result->size, ((double) distrib_sum) / ((double) result->size));


	if(printrawdata)
	{
		printf("\nRaw data of the generation\n");
		if(don)
			for(i=0;i<result->size;i++)
				printf("RAW_Don_bytes;" PercentMyInt "\n", result->data[i]);
		else
			for(i=0;i<result->size;i++)
				printf("RAW_Doff_nsec;" PercentMyInt "\n", result->data[i]);
	}

	printf("\nDistribution of the %s generation\nDistrib_type;Start_of_range;Number_of_times\n", DonOffStr(don));
	for(i=0;i<=PRINT_DISTRIB_COUNT_NB;i++)
		count[i] = 0;
	for(i=0;i<result->size;i++)
	{
		val = (int) (
				((double) (result->data[i] -  distrib.min))
				/((double) (distrib.max - distrib.min))
				*((double) PRINT_DISTRIB_COUNT_NB) );

		if(val <= 0)
			count[0]++;
		else if(val >= PRINT_DISTRIB_COUNT_NB)
			count[PRINT_DISTRIB_COUNT_NB]++;
		else
			count[ val ]++;
	}

	for(i=0;i<=PRINT_DISTRIB_COUNT_NB;i++)
	{
		myInteger Xval = distrib.min + ((myInteger)i)*(distrib.max - distrib.min)
				/PRINT_DISTRIB_COUNT_NB;
		printf("%s;" PercentMyInt ";%d\n", DonOffStr(don), Xval, count[i]);
	}
}



/* Convert a string into an unsigned integer (32 bits).
    string may be concluded with a suffixe like "k" and "M" to multiply by 1000 (kilo)... */
unsigned int stringToUInt(const char * s)
{
	/* We take multiple of bytes (and not time), in order to have k, M, G, T */
	myInteger r = stringTomyInteger(s, true);
	if(r<0)
		return 0;
	else if(r>(unsigned int)(-1))
		return (unsigned int)(-1);
	else
		return (unsigned int) r;
}

/* Convert a string into an myInteger (64 bits).
    string may be concluded with a suffix like "k" to multiply by 1000 (kilo)...
    if (isUnit_Byte) then we use multiples 2**10 instead of 10**3 :
    1 kB = 1024 bytes ; 1 ks = 1000 seconds */
myInteger stringTomyInteger(const char * s, bool isUnit_Byte)
{
	char * invalidCar = NULL;
	myInteger result = strtoll(s, &invalidCar, 0);

	/* Manage the suffix */
	if(invalidCar!=NULL)
	{
		if(isUnit_Byte) {
			switch(invalidCar[0])
			{
			case 0: /* No error ! */
				break;
			case 'b': /* b=bit = (1/8)*Byte */
				result = (result >> 3);
				break;
			case 'B': /* Byte */
				break; /* ignore "Byte" */
			case 'k': /* kilo */
			case 'K':
				result = (result << 10);
				break;
			case 'm':
			case 'M': /* Mega */
				result = (result << 20);
				break;
			case 'g':
			case 'G': /* Giga */
				result = (result << 30);
				break;
			case 't':
			case 'T': /* Tera */
				result = (result << 40);
				break;
			default:
				fprintf(stderr, "conversion error for <%s>: '%c' unknown suffix !\n",
						s, invalidCar[0]);
				exit(6);
			}
		}
		else
		{
			/* ! isUnit_Byte <=> Unit is time! */
			switch(invalidCar[0])
			{
			case 0: /* No error ! */
				break;
			case 'k': /* kilo-seconds */
			case 'K':
				result *= ((myInteger) NS_IN_SECONDS*1000);
				break;
			case 's':
			case 'S': /* second */
				result *= (myInteger) NS_IN_SECONDS;
				break;
			case 'm':
			case 'M': /* millisecond */
				result *= 1000*1000;
				break;
			case 'u':
			case 'U': /* microsecond */
				result *= 1000;
				break;
			case 'n':
			case 'N': /* nanosecond => ignore */
				break;
			default:
				fprintf(stderr, "conversion error for <%s>: '%c' unknown suffix !\n",
						s, invalidCar[0]);
				exit(7);
			}
		}
	}
	return result;
}

/* Return a string like "Don" or "Doff", depending on the param */
char * DonOffStr(int don)
{
	if(don)
		return (char*) "Don";
	else
		return (char*) "Doff";

}

/* getDistributionFromFile reads the file given in parameter and returns the numbers into the resulting distribution. If numberTurns > number of lines of the file, the distribution will be completed with the value "0". If numberTurns < number of lines of the file, the file content will be truncated. If there is an error during the file reading, then all data will be 0. */
Pdistribdata getDistributionFromFile(unsigned int numberTurns, char * filename)
{
	unsigned int i;
	Pdistribdata  result;
	FILE * fh;

	printf("Reading of %d numbers in the file %s...", numberTurns, filename);

	/* Prepare the result struct */
	result = malloc_distribdata(numberTurns);

	/* Open the file */
	fh = fopen(filename, "r");
	if(NULL==fh) {
		fprintf(stderr, "Cannot read the file <%s>!", filename);
		exit(8);
	}

	/* Read the file */
	for(i = 0; i < numberTurns; i++)
	{
		int res;
		res = fscanf(fh, PercentMyInt, &result->data[i]);
		if(res<=0)
		{
			fprintf(stderr, "Erreur %d durant la lecture du fichier <%s>.\n Arrêt avant la fin ou fin atteinte prématurément, les données seront complétées avec la valeur 0...\n", res, filename);
			break;
		}
	}

	/* Conclude the function */
	fclose(fh);
	return result;
}

