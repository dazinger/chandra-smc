/****************************************************************/
/* Programmer: Zinger                                           */
/*                                                              */
/* Program for SMC                                              */
/*                                                              */
/* Table Data Implementation                                    */
/****************************************************************/

#include "globals.h"
#include "data.h"
#include "trees.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct table_data_rep {
        long double  MJD;
        long double  significance;
	long double  collimator;
        char        *filename;
        int          filenum;
} table_data_rep, *data_ptr;

#define TOGENPTR(table_data_ptr) ((generic_ptr) table_data_ptr)

#define MJD(table_data_ptr)      ((table_data_ptr)->MJD)
#define SIG(table_data_ptr)      ((table_data_ptr)->significance)
#define COLL(table_data_ptr)     ((table_data_ptr)->collimator)
#define FILENAME(table_data_ptr) ((table_data_ptr)->filename)
#define FILENUM(table_data_ptr)  ((table_data_ptr)->filenum)

#define ERROR(new_struct, errno, ferr)					\
	fprintf(ferr, "  **************************\n");                \
	fprintf(ferr, "  *     ERROR #%-2d!         *\n", errno);     	\
	fprintf(ferr, "  **************************\n");                \
	fprintf(ferr, "Filename:%s\n", FILENAME(new_struct));		\
	fprintf(ferr, "  **************************\n");		\
	fprintf(ferr, "  * Duplicate data!        *\n");		\
	fprintf(ferr, "  * MJD: %-16Lf  *\n", MJD(new_struct));		\
	fprintf(ferr, "  * Sig: %-16Lf  *\n", SIG(new_struct));		\
	fprintf(ferr, "  * Coll: %-15Lf  *\n", COLL(new_struct));	\
	fprintf(ferr, "  **************************\n");                \
	fprintf(ferr, "\n\n\n");					\

#define OFFSET(i) (i - 1 - ((ARGC - 3) - FNUM))

static tree top;

static status insert_data(data_ptr new_struct, tree T)
{
	/*  new_top is the tree that includes the left
	 *  or right tree branches where we will be
	 *  placing the new_struct in.  if it is
	 *  not the correct new_top, then set new_top
	 *  to be equal to the left or right branch.
	 *
	 *  if calltime is zero, then this means that
	 *  we must set new_top, the temporary top of
	 *  the tree to be equal to top.  This is
	 *  because of the fact that we traverse the
	 *  tree starting at the root at the top of
	 *  the tree.
	 *
	 *  If calltime is not zero, then this means
	 *  that this function has been called already,
	 *  and that we must find the correct path
	 *  (left or right) to go through the tree.
	 *  This allows us to be able to find the
	 *  correct value of new_top.
	 */

        /*  First case: smaller mjd:
         *  Base case 1: if left tree is empty, then
         *  allocate left tree and put mjd in it
         *
         *  else recurse on left tree
         */

        if (MJD(new_struct) < MJD((data_ptr) (getdata(T)))) {
                if (empty_tree(getleft(T)) == TRUE) {
			if (make_root(getleftadr(T), TOGENPTR(new_struct))
                            == ERROR)
                                return ERROR;
                        return OK;
                }
                else
                        return insert_data(new_struct, getleft(T));
	}

        /*  Second case: larger or equal mjd:
         *  Base case 2: if right tree is empty, then
         *  allocate right tree and put mjd in it
         *
         *  else recurse on right tree
         */

	if (MJD(new_struct) == MJD((data_ptr) (getdata(T)))) {
                if (empty_tree(getleft(T)) == TRUE) {
			if (make_root(getleftadr(T), TOGENPTR(new_struct))
			    == ERROR)
				return ERROR;
			return OK;
		}
                else
                        return insert_data(new_struct, getleft(T));
	}

        if (MJD(new_struct) > MJD((data_ptr) (getdata(T)))) {
                if (empty_tree(getright(T)) == TRUE) {
			if (make_root(getrightadr(T), TOGENPTR(new_struct))
                            == ERROR)
                                return ERROR;
                        return OK;
                }
                else
			return insert_data(new_struct, getright(T));
	}
	/* can't get here */
	printf("Fatal error in function, function close");
	return ERROR;
}

status enter_data(long double mjd, long double sig,
		  long double coll, char *file_name,
		  int filenum)
{
	/*
	 *  Creates a struct table_data_rep and initializes
	 *  it with the values mjd, sig, file_name, and order.
	 */
	static int calltime = 0;
	data_ptr new_struct; 

	new_struct = (data_ptr) malloc(sizeof(table_data_rep));

	if (new_struct == NULL)
		return ERROR;

	MJD(new_struct) = mjd;
	SIG(new_struct) = sig;
	COLL(new_struct) = coll;
	FILENAME(new_struct) = file_name;
	FILENUM(new_struct) = filenum;

	if (calltime == 0) { /* When first call, make first tree */
		init_tree(&top);
		if (make_root(&top, TOGENPTR(new_struct)) == ERROR)
			return ERROR;
		calltime++;
	}

	/*  On any other call, sort through tree to put data in the
	 *  appropiate spot by calling insert_data(new_struct).
	 */

	else
		insert_data(new_struct, top);

	return OK;
}

void fix_offset(FILE *fout, int filenum)
{
	int i, digitnum = 0;

	/*  for 2 digits, add 3 spaces each, for 3 digits, add 4 spaces each,
	 *  for 1 digit, add 2 spaces each
	 */

	while (filenum != 0) {
	        digitnum++;
	        filenum = filenum / 10;
	}


	for (i = 0; i <= digitnum; i++)
	        fprintf(fout, " ");
}

status print_data(generic_ptr data, FILE *fout, FILE *ferr, int numfiles,
		  char *argv[], int argc, int numsets)
{
	static int FNUM;
	static int STHR;
	static int ARGC;
	static int n;
	static int setn;
	static int errno = 0;
	static long double mjd = 0;
	static FILE  *FOUT, *FERR;
	static char **ARGV;
	static bool begin;
	data_ptr new_struct;
	int i, j, num, digitnum = 0;

	new_struct = (data_ptr) data;

	if (data != 0) {
		if (begin == TRUE) {
			/*  First line, so we write out the names
			 *  of the files now.
			 */
			for (i = ARGC - 2; i >= (ARGC - (FNUM + 1)); --i) {
				fprintf(FOUT, "File #%d", OFFSET(i));

				num = OFFSET(i);

				while (num != 0) {
					digitnum++;
					num = num / 10;
				}

				if ((6 + digitnum) < (56 + (4 * digitnum))) {
					j = 6 + digitnum - (4 * digitnum);
					while (j < 56) {
						fprintf(FOUT, " ");
						j++;
					}
				}

				digitnum = 0;
			}

			fprintf(FOUT, "\n");

			for (i = ARGC - 2; i >= (ARGC - (FNUM + 1)); --i) {
				fprintf(FOUT, "MJD_%d           ", OFFSET(i));
				fprintf(FOUT, "Coll_%d         ", OFFSET(i));
				fprintf(FOUT, "Sig_%d         ", OFFSET(i));
				fprintf(FOUT, "Capture_%d ---- ", OFFSET(i));
			}

			begin = FALSE;
			n = 0;
			fprintf(FOUT, "\n");
		}
		if (MJD(new_struct) != mjd) {
			while (n != 0) {
				fprintf(FOUT, "%12Lf  ", (long double) -1);
				fix_offset(FOUT, n);
				fprintf(FOUT, "%11Lf  ", (long double) -1);
				fix_offset(FOUT, n);
                                fprintf(FOUT, "%10Lf  ", (long double) -1);
				fix_offset(FOUT, n);
                                fprintf(FOUT, "%4d", (int) -1);
				fprintf(FOUT, "%9c", (char) ' ');
				fix_offset(FOUT, n);
				--n;
			}
                        n = FNUM;
                        fprintf(FOUT, "\n");
                }
		if (FILENUM(new_struct) > n) {
			++errno;
			ERROR(new_struct, errno, FERR);
		} else {
			while (FILENUM(new_struct) != n) {
				fprintf(FOUT, "%12Lf  ", (long double) -1);
				fix_offset(FOUT, n);
				fprintf(FOUT, "%11Lf  ", (long double) -1);
				fix_offset(FOUT, n);
				fprintf(FOUT, "%10Lf  ", (long double) -1);
				fix_offset(FOUT, n);
				fprintf(FOUT, "%4d", (int) -1);
				fprintf(FOUT, "%9c", (char) ' ');
				fix_offset(FOUT, n);
				--n;
			}
			mjd = MJD(new_struct);
			fprintf(FOUT, "%12Lf  ", MJD(new_struct));
			fix_offset(FOUT, FILENUM(new_struct));
			fprintf(FOUT, "%11Lf  ", COLL(new_struct));
			fix_offset(FOUT, FILENUM(new_struct));
			fprintf(FOUT, "%10Lf  ", SIG(new_struct));
			fix_offset(FOUT, FILENUM(new_struct));
			if (SIG(new_struct) >= STHR) {
				fprintf(FOUT, "%4d", (int) 1);
				fprintf(FOUT, "%9c", (int) ' ');
				fix_offset(FOUT, FILENUM(new_struct));
			} else {
				fprintf(FOUT, "%4d", (int) 0);
				fprintf(FOUT, "%9c", (char) ' ');
				fix_offset(FOUT, FILENUM(new_struct));
			}
			--n;
		}
		setn--;
		if (setn == 0)
			while (n != 0) {
                                fprintf(FOUT, "%12Lf  ", (long double) -1);
				fix_offset(FOUT, n);
				fprintf(FOUT, "%11Lf  ", (long double) -1);
				fix_offset(FOUT, n);
                                fprintf(FOUT, "%10Lf  ", (long double) -1);
				fix_offset(FOUT, n);
                                fprintf(FOUT, "%4d", (int) -1);
				fprintf(FOUT, "%9c", (char) ' ');
				fix_offset(FOUT, n);
                                --n;
                        }
        } else {
                /*  First run through the loop, initialize FNUM and STHR and
                 *  call traverse_tree with top, print_data, and INORDER as
                 *  the arguments.
		 */
		setn = numsets;
		FNUM = numfiles;
		STHR = atoi(argv[1]);
		ARGC = argc;
		FOUT = fout;
		FERR = ferr;
		ARGV = argv;
		begin = TRUE;
		traverse_tree(top, print_data, INORDER);
	}

	return OK;
}
