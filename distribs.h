/*
 * File distribs.h
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
 * distribs.h
 *
 *  Created on: 5 oct. 2012
 *      Author: avaret
 */

#ifndef DISTRIBS_H_
#define DISTRIBS_H_

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdbool.h"

#include "on_off_common.h"

/* Number of ranges to compute the distribution printing */
#define PRINT_DISTRIB_COUNT_NB 100

/* getDistribution generates "numberTurns" random numbers, conforming to "distrib",
 * and store the result into "result".
 */
Pdistribdata getDistribution(unsigned int numberTurns, Tdistrib distrib, bool don, int *size, int *time);

/* printDistributions prints the distrib "result" and its parameter "distrib" on the screen */
void printDistributions(Tdistrib distrib, Pdistribdata result, int don, int printrawdata);

/* Allocate a struct for DistribData, with "number" items inside */
Pdistribdata malloc_distribdata(unsigned int number);

/* Free the memory associated with a distribution */
void free_distribdata(Pdistribdata d);

/* Convert a string into an unsigned integer (32 bits).
    string may be concluded with a suffixe like "k" and "M" to multiply by 1000 (kilo)... */
unsigned int stringToUInt(const char * s);

/* Convert a string into an myInteger (64 bits).
    string may be concluded with a suffixe like "k" and "M" to multiply by 1000 (kilo)... */
myInteger stringTomyInteger(const char * s, bool isUnit_Byte);


/*** internal ***/

/* Returns x if (min < x < max), else return min or max (nearest of x) */
myInteger bound_min_max(long double x, myInteger min, myInteger max);

/* Random generator of double numbers in range [0;1] */
long double drand(void);

/* Initialize the uniform random number generator with the new maximum */
void initialize_irand(myInteger max);

/* Random generator of integer, uniform distrib between [0..__max], see  initialize_irand() */
myInteger irand(void);

/* Polar form of the Box-Muller transformation */
long double get_random_gaussian(void);

/* Knuth 's algorithm, do NOT use for large lambda values */
int get_random_poisson(void);

long double get_random_weibull(void);
long double get_random_exp(void);
long double get_random_pareto(void);

/* Return a string like "Don" or "Doff", depending on the param */
char * DonOffStr(int don);

/* getDistributionFromFile reads the file given in parameter and returns the numbers into the resulting distribution. If numberTurns > number of lines of the file, the distribution will be completed with the value "0". If numberTurns < number of lines of the file, the file content will be truncated. If there is an error during the file reading, then all data will be 0. */
Pdistribdata getDistributionFromFile(unsigned int numberTurns, char * filename);

#endif /* DISTRIBS_H_ */
