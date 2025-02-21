/************************************************************/
/* Zinger                                                   */
/*                                                          */
/* globals.h                                                */
/*                                                          */
/* generic_ptr        see page 44 in Esakov and Weiss       */
/* status             see page 41 in Esakov and Weiss       */
/* bool               see page 41 in Esakov and Weiss       */
/*                                                          */
/* ABSTRACT_TYPE      added for by Mark A. Sheldon          */
/*                    Spring 2012                           */
/************************************************************/

#ifndef _GLOBALS_H
#define _GLOBALS_H

typedef enum {OK, ERROR} status;
typedef enum {FALSE = 0, TRUE = 1} bool;

typedef void *generic_ptr;

typedef unsigned char byte;

#define ABSTRACT_TYPE(t)    \
	typedef void *(t)

#endif
