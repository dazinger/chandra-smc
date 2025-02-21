/****************************************************************/
/* Programmer: Zinger                                           */
/*                                                              */
/* Program for SMC                                              */
/*                                                              */
/* Table Data Implementation                                    */
/****************************************************************/

#ifndef _DATA_H
#define _DATA_H

#include <stdio.h>
#include "globals.h"

typedef enum { MJD, SIG, FILENAME, FILENUM } data;

extern status enter_data(long double  mjd,        long double sig,
			 long double  coll,        char      *file_name,
			 int          filenum);
extern void   fix_offset(FILE        *fout,       int         filenum);
extern status print_data(generic_ptr  data,       FILE       *fout,
			 FILE        *ferr,       int         numfiles,
			 char        *argv[],     int         argc,
			 int          numsets);

#endif
