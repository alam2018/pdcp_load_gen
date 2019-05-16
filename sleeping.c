/*
 * File sleeping.c
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
 * sleeping.c
 *
 *  Created on: 5 oct. 2012
 *      Author: avaret
 */

#include "sleeping.h"


/* Wait for the asked duration reduced by the compensation,
 * computed from "begin". Check the real spent duration: it is the compensation
 * for the next call of the function!
 * manual_begin may be NULL, then an internally measured value is used instead.
 */
void nanosleep_manually_compensated(myInteger expected_duration)
{
	struct timespec curr, rem, now, begin;
	static long long compensation = 0;

/*	if(manual_begin)
		begin = *manual_begin;
	else*/
	clock_gettime(CLOCK_MONOTONIC, &begin);

//	expected_duration -= compensation;

	if(expected_duration>0)
	{
		curr.tv_sec = (time_t) (expected_duration / NS_IN_SECONDS);
		curr.tv_nsec = (long) (expected_duration % NS_IN_SECONDS);

		while ((nanosleep(&curr, &rem) == -1)&&(errno == EINTR))
			curr = rem;
	}
//	clock_gettime(CLOCK_MONOTONIC, &now);
/*	compensation = ((long long) (now.tv_nsec - begin.tv_nsec))
							+ ((long long) (now.tv_sec - begin.tv_sec)) * NS_IN_SECONDS
							- expected_duration;*/
}

/* Return the date/time, in ns */
myInteger clock_now(void)
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return ((long long) now.tv_sec * NS_IN_SECONDS + (long long) now.tv_nsec);
}
