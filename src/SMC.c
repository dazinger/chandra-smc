/****************************************************************/
/* Programmer: Zinger                                           */
/*                                                              */
/* Program for SMC                                              */
/****************************************************************/

#include "data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOO_FEW_ARG(c) \
	if (c <= 3) { \
	        printf("\nToo few arguments recieved.\n");   \
	        printf("Enter in the following order:\n\n"); \
	        printf("1.) Significance threshold\n");	     \
	        printf("2.) File(s) to read data from\n");   \
                printf("3.) Name of output file\n\n");	     \
	        exit(1); \
        }

#define WARN_QUASI() printf("\nWARNING: -1 detected in capture history!\n");

#define COLLDATA(filenumber, nfiles, i) (colldata[nfiles - filenumber].coll[i])

#define OFFSET(file) (file + 1 + (argc - 3 - numfiles))

typedef struct bar {
	long double coll;
	int frequency;
} bar;

typedef struct collcolumn {
	long double *coll;
	bool is_included;
} collcolumn;

typedef enum { COMM, RANGE } action;
typedef enum { FILT, UNFILT, MANUAL } mode;

status print_file_table(char        *input,        char        *output,
			char        *argv[],       int          argc,
			int          numfiles);
status comm_and_range  (action       job,          mode         type,  
                        bar         *graph,        int          graphsize,
                        long double *commoncoll,   long double *floor,
		        long double *ceiling);
status mjd_filter      (char        *input,        char        *output,
                        int          numfiles,     int         *numlines,
		        int          targetpulsar, mode         type);
status pulsar_filter   (char        *input,        char        *output,
                        int         *includedfnum, int          targetpulsar,
	                mode         type);
status print_capture_1 (char        *input,        char        *output,
                        int          numfiles,     int          numlines);
status print_capture_2 (char        *input,        char        *output,
                        int          numfiles,     int          numlines);

int main (int argc, char *argv[])
{
        /*
         *  Reads in Significance from argv[1] into counter, and
         *  additional files thereafter. Takes data from files
         *  with Significance greater than or equal to argv[1].
         *  Outputs data to file specified in the last array spot
         *  in argv.
         */
	int i, j = 0;
	int numsets = 0, numfiles = 0, filelineno = 0, inputno = 0, 
		numlines = 0, includedfnum, targetpulsar;
	bool isfirstline;
	char *data_input, *data_output, *table_output, *error_output;
        long double mjd, coll, sig, counter;
	mode type;
        FILE *fin, *fout, *ferr;

	TOO_FEW_ARG(argc);

	for (i = 2; i <= (argc - 2); i++) {
		fin = fopen(argv[i], "r");
	        while ((fscanf(fin, "%Lf", &counter)%7) == 1) {
        	        j++;
                	if ((j%7) == 1) {
	       			if (filelineno == 0)
					isfirstline = TRUE;
				else
					isfirstline = FALSE;
				if (isfirstline == TRUE)
					numfiles++; /* only useful files*/
				mjd = counter;
				inputno++;
                                filelineno++;
			} else if ((j%7) == 2) {
				coll = counter;
				inputno++;
			} else if ((j%7) == 3) {
				sig = counter;
				inputno++;
				numsets++;
				enter_data(mjd, sig, coll, argv[i], numfiles);
			}
        	}
		filelineno = 0;
	}
	fclose(fin);

	data_output = (char *) calloc((strlen(argv[argc - 1]) + 6), 
				      sizeof(char));

	if (data_output == NULL)
		return -1;

	strcat(data_output, "data-");
	strcat(data_output, argv[argc - 1]);

	fout = fopen(data_output, "w");

	error_output = (char *) calloc ((strlen(argv[argc - 1]) + 8),
					 sizeof(char));
	if (error_output == NULL) {
		free(data_output);
		fclose(fout);
		return -1;
	}

	strcat(error_output, "errors-");
	strcat(error_output, argv[argc - 1]);

	ferr = fopen(error_output, "w");

	print_data(0, fout, ferr, numfiles, argv, argc, numsets);

	fclose(fout);
	fclose(ferr);

	free(error_output);

	data_input = (char *) calloc((strlen(argv[argc - 1]) + 14),
				     sizeof(char));

	if (data_input == NULL) {
		free(data_output);
		return -1;
	}

	strcat(data_input, data_output);

	table_output = (char *) calloc((strlen(argv[argc - 1]) + 11),
					sizeof(char));

        if (table_output == NULL) {
		free(data_input);
		free(data_output);
                return -1;
	}

        strcat(table_output, "filetable-");
        strcat(table_output, argv[argc - 1]);

	print_file_table(data_input, table_output, argv, argc, numfiles);

	free(table_output);

	data_input = (char *) calloc((strlen(argv[argc - 1]) + 6),
				     sizeof(char));

	if (data_input == NULL) {
		free(data_output);
		return -1;
	}

	strcat(data_input, data_output);

	free(data_output);

	data_output = (char *) calloc((strlen(argv[argc - 1]) + 14),
				      sizeof(char));

	if (data_output == NULL) {
		free(data_input);
		return -1;
	}

	strcat(data_output, "mjd-filter-1-");
	strcat(data_output, argv[argc - 1]);

	targetpulsar = 52; /*  SXP2.37_g0.2.txt = 31 - SXP2.76_g0.2.txt = 35
			  *  SXP46.6_g0.2.txt = 54
			  */
	type = UNFILT; /* Keep unfiltered range for now!! */

	mjd_filter(data_input, data_output, numfiles, &numlines, targetpulsar,
		   type);

	free(data_input);
	
	data_input = (char *) calloc((strlen(argv[argc - 1]) + 14),
				     sizeof(char));

	if (data_input == NULL) {
		free(data_output);
		return -1;
	}

	strcat(data_input, data_output);

	free(data_output);

	data_output = (char *) calloc((strlen(argv[argc - 1]) + 21),
				      sizeof(char));

	if (data_output == NULL) {
		free(data_input);
		return -1;
	}

	strcat(data_output, "mjd-pulsar-filter-2-");
	strcat(data_output, argv[argc - 1]);

	includedfnum = numfiles;

	pulsar_filter(data_input, data_output, &includedfnum, targetpulsar,
		      type);

	free(data_input);

	data_input = (char *) calloc((strlen(argv[argc - 1]) + 21),
				     sizeof(char));

	if (data_input == NULL) {
		free(data_output);
		return -1;
	}

	strcat(data_input, data_output);

        table_output = (char *) calloc((strlen(argv[argc - 1]) + 20),
				       sizeof(char));

        if (table_output == NULL) {
		free(data_input);
		free(data_output);
                return -1;
	}

        strcat(table_output, "filtered-filetable-");
        strcat(table_output, argv[argc - 1]);

        print_file_table(data_input, table_output, argv, argc, numfiles);

	free(data_input);
        free(table_output);

	data_input = (char *) calloc((strlen(argv[argc - 1]) + 21),
				     sizeof(char));

	if (data_input == NULL) {
		free(data_output);
		return -1;
	}

	strcat(data_input, data_output);

	free(data_output);

	data_output = (char *) calloc((strlen(argv[argc - 1]) + 21),
				       sizeof(char));

	if (data_output == NULL) {
		free(data_input);
		return -1;
	}

	strcat(data_output, "old-capture-history-");
	strcat(data_output, argv[argc - 1]);	     

	print_capture_1(data_input, data_output, includedfnum, numlines);

        free(data_input);
	free(data_output);

        data_input = (char *) calloc((strlen(argv[argc - 1]) + 21),
                                     sizeof(char));

        if (data_input == NULL)
                return -1;

	strcat(data_input, "mjd-pulsar-filter-2-");
        strcat(data_input, argv[argc - 1]);

        data_output = (char *) calloc((strlen(argv[argc - 1]) + 17),
				      sizeof(char));

        if (data_output == NULL) {
                free(data_input);
                return -1;
        }

        strcat(data_output, "capture-history-");
        strcat(data_output, argv[argc - 1]);

        print_capture_2(data_input, data_output, includedfnum, numlines);

	free(data_output);

        return 0;
}

status print_file_table(char *input, char *output, char *argv[], int argc,
			int numfiles)
/* NOTE: This function will only work if the arguments in argv[] ONLY contains
*        files for known pulsars (with the exception of column_definitions.txt,
*        which, by linux standards, will be loaded first due to its alphabetical
*        order)!!
*/
{
	int i = numfiles, j, limit = 1, filedignum, maxdignum = 0, file, num;
	char readchar;
	bool complete = FALSE;
	FILE *fin, *fout;

	while (limit < numfiles)
		limit *= 10;

	while (limit != 0) {
		maxdignum++;
		limit /= 10;
	}

	fin = fopen(input, "r");
	fout = fopen(output, "w");

	while (i > 1) {
		readchar = 'g';
		
		while ((readchar != '#') && (complete == FALSE)) {
			fscanf(fin, "%c", &readchar);
			if (readchar == '\n')
				complete = TRUE;
		}

		if (complete == FALSE) {
			fscanf(fin, "%d", &file);
			
			while (file != i)
				--i;
			
			fprintf(fout, "FILE #%d", file);
			
			filedignum = 0;
			num = file;
			
			while (num != 0) {
				filedignum++;
				num /= 10;
			}
			
			for (j = 0; j < (maxdignum - filedignum); ++j)
				fprintf(fout, " ");
			
			fprintf(fout, "= ");
			fprintf(fout, "%s\n", argv[OFFSET(file)]);
			
		} else
			i = 1;
	}

	fclose(fin);
	fclose(fout);

	return OK;
}

status comm_and_range(action job, mode type, bar *graph, int graphsize,
		      long double *commoncoll, long double *floor,
		      long double *ceiling) {
	/*  If job equals COMM, then function call should be:
	 *  
	 *  comm_and_range(job, type, graph, graphsize, commoncoll, 0, 0)
	 *
	 *  Otherwise, if job equals RANGE and type equals UNFILT, then
         *  function call should be:
	 *
	 *  comm_and_range(job, type, 0, 0, 0, floor, ceiling)
         *
	 *  No function call will have job equaling RANGE and type equaling FILT
	 */
	static long double *colldata = NULL;
	static int size;
	int i, largestfreq = 0;
	
	if (job == COMM) {
		if (type == UNFILT) {
			colldata = (long double *)
				malloc(graphsize * sizeof(long double));
			if (colldata == NULL)
				return ERROR;
			size = graphsize;
		}
		for (i = 0; i < graphsize; ++i) {
			if (type == UNFILT)
				colldata[i] = graph[i].coll;
			if ((graph[i].frequency > largestfreq) &&
			    (graph[i].coll != -1)) {
				largestfreq = graph[i].frequency;
				*commoncoll = graph[i].coll;
			}
		}
	} else if ((job == RANGE) && (type == UNFILT)) {
		if (colldata == NULL)
			return ERROR;
		*floor = 1;
		*ceiling = 0;

		for (i = 0; i < size; ++i) {
			if ((colldata[i] < *floor) && (colldata[i] != -1))
				*floor = colldata[i];
			if ((colldata[i] > *ceiling) && (colldata[i] != -1))
				*ceiling = colldata[i];
		}
		
		free(colldata);
	} else
		/* can't get here */
		return ERROR;

	return OK;
}

status mjd_filter(char *input, char *output, int numfiles, int *numlines,
		  int targetpulsar, mode type)
{
	FILE *fin, *fout;
	char readchar;
	long double *colldata, commoncoll, data;
	bar *graph;
	bool complete = FALSE, coll_added = FALSE;
	int i = 0, j = 0, k;
	int skipno = 0, junk, capt, size = 0, graphsize = 0;

	fin = fopen(input, "r");

        while ((fscanf(fin, "%c", &readchar) == 1) && (skipno != 2))
                if (readchar == '\n')
                        skipno++;

        while (complete == FALSE) {
                if (j == 3) {
                        i++;
                        j = 0;
                        fscanf(fin, "%d", &junk);
			if (i == numfiles) {
                                i = 0;
                                readchar = 'g';
                                while (readchar != '\n')
                                        if (fscanf(fin, "%c", &readchar)
                                            == EOF) {
                                                readchar = '\n';
                                                complete = TRUE;
                                        }
                        }
                } else {
                        j++;
                        fscanf(fin, "%Lf", &data);
                        if ((j == 2) && (i == numfiles - targetpulsar) &&
			    (size == 0)) {
				size++;
				colldata = (long double *)
					realloc(NULL,
						size * sizeof(long double));
				if (colldata == NULL)
					return ERROR;
				colldata[size - 1] = data;
			} else if ((j == 2) &&
				   (i == numfiles - targetpulsar) &&
				   (size != 0)) {
				size++;
				colldata = (long double *)
					realloc(colldata, 
						size * sizeof(long double));
				if (colldata == NULL)
					return ERROR;
				colldata[size - 1] = data;
			}
		}
	}

	for (i = 0; i < size; ++i) {
		if (graphsize == 0) {
			graphsize++;
			graph = (bar *) realloc(NULL, graphsize * sizeof(bar));
			if (graph == NULL) {
				free(colldata);
				return ERROR;
			}
			graph[graphsize - 1].coll = colldata[i];
			graph[graphsize - 1].frequency = 1;
		} else {
			for (k = 0; k < graphsize; ++k) {
				if (colldata[i] == graph[k].coll) {
					graph[k].frequency++;
					coll_added = TRUE;
				}
			}

			if (coll_added == FALSE) {
				graphsize++;
				graph = realloc((generic_ptr) graph,
						graphsize * sizeof(bar));
				if (graph == NULL) {
					free(colldata);
					return ERROR;
				}
				graph[graphsize - 1].coll = colldata[i];
				graph[graphsize - 1].frequency = 1;
			} else
				coll_added = FALSE;
		}
	}

	comm_and_range(COMM, type, graph, graphsize, &commoncoll, 0, 0);

	free(graph);
	fclose(fin);

	/* step 2 */

        fin = fopen(input, "r");
        fout = fopen(output, "w");

	skipno = 0;

        while ((fscanf(fin, "%c", &readchar) == 1) && (skipno != 2)) {
		fprintf(fout, "%c", readchar);
                if (readchar == '\n')
                        skipno++;
	}

	fprintf(fout, "%c", readchar);

	k = 0;

	for (i = 0; i < size; ++i) {
		if (colldata[i] == commoncoll)
			(*numlines)++;
		for (j = numfiles; j > 0; --j) {
			for (k = 0; k < 4; ++k)
				switch (k) 
				{
				case 3:
					fscanf(fin, "%d", &capt);
					if (colldata[i] == commoncoll) {
						fprintf(fout, "%4d", capt);
						fprintf(fout, "%9c", ' ');
						fix_offset(fout, j);
					}
					break;
				case 2:
					fscanf(fin, "%Lf", &data);
					if (colldata[i] == commoncoll) {
						fprintf(fout, "%10Lf  ", data);
						fix_offset(fout, j);
					}
					break;
				case 1:
					fscanf(fin, "%Lf", &data);
					if (colldata[i] == commoncoll) {
						fprintf(fout, "%11Lf  ", data);
						fix_offset(fout, j);
					}
					break;
				case 0:
					fscanf(fin, "%Lf", &data);
					if (colldata[i] == commoncoll) {
						fprintf(fout, "%12Lf  ", data);
						fix_offset(fout, j);
					}
					break;
				}
		}
		if (i != (size - 1)) {
			readchar = 'g';
			while (readchar != '\n')
				fscanf(fin, "%c", &readchar);
			if (colldata[i] == commoncoll)
				fprintf(fout, "%c", readchar);
		}
	}

        fclose(fin);
        fclose(fout);

	free(colldata);

	return OK;
}

status pulsar_filter(char *input, char *output, int *numfiles,
		     int targetpulsar, mode type)
{
	FILE *fin, *fout;
	char readchar;
	long double data, floor, ceiling, coll_min, coll_max;
	collcolumn *colldata;
	int i = 0, j = 0, k;
	int lineno = 0, skipno = 0, size = 0, junk, filenum, num, digitnum;
	bool complete = FALSE;

	/* first part: go through file and put all coll data in it */

	fin = fopen(input, "r");

        while ((fscanf(fin, "%c", &readchar) == 1) && (skipno != 2))
                if (readchar == '\n')
                        skipno++;

	while (complete == FALSE) {
                if (j == 3) {
                        i++;
                        j = 0;
                        fscanf(fin, "%d", &junk);
                        if (i == *numfiles) {
                                lineno++;
				i = 0;
                                readchar = 'g';
                                while (readchar != '\n')
                                        if (fscanf(fin, "%c", &readchar)
                                            == EOF) {
                                                readchar = '\n';
                                                complete = TRUE;
                                        }
                        }
                } else {
                        j++;
                        fscanf(fin, "%Lf", &data);
			if (j == 2) { /* a coll column */
				if ((size == 0) && (i == 0) && (lineno == 0)) {
					size++;
					colldata = (collcolumn *)
						realloc (NULL,
							 size * 
							 sizeof(collcolumn));
					if (colldata == NULL)
						return ERROR;
					colldata[i].coll = (long double *)
						realloc (NULL,
							 sizeof(long double));
					if (colldata[i].coll == NULL) {
						free(colldata);
						return ERROR;
					}
					colldata[i].coll[lineno] = data;
				} else if ((size != 0) && (i != 0) && 
					   (lineno == 0)) {
					size++;
					colldata = (collcolumn *)
						realloc (colldata,
							 size *
							 sizeof(collcolumn));
					if (colldata == NULL)
						return ERROR;
					colldata[i].coll = (long double *)
						realloc (NULL,
							 sizeof(long double));
					if (colldata[i].coll == NULL) {
						free(colldata);
						return ERROR;
					}
					colldata[i].coll[lineno] = data;
				} else if (lineno != 0) {
					colldata[i].coll = (long double *)
						realloc (colldata[i].coll, 
							 (lineno + 1) *
							 sizeof(long double));
					if (colldata[i].coll == NULL) {
						for (k = 0; k < lineno; ++k)
							free(colldata[k].coll);
						free(colldata);
						return ERROR;
					}
					colldata[i].coll[lineno] = data;
				}
			}
		}
	}

	fclose(fin);

	/* second part: go find the range for the third part */

	if (type == UNFILT)
		comm_and_range(RANGE, type, 0, 0, 0, &floor, &ceiling);
	else if (type == FILT) {
		floor = 1;
		ceiling = 0;
		
		for (i = 0; i < (lineno - 1); ++i) {
			if (COLLDATA(targetpulsar, *numfiles, i) < floor)
				floor = COLLDATA(targetpulsar, *numfiles, i);
			if (COLLDATA(targetpulsar, *numfiles, i) > ceiling)
				ceiling = COLLDATA(targetpulsar, *numfiles, i);
		}
	} else if (type != MANUAL)
		/* can't get here */
		return ERROR;

	printf("%Lf = floor, %Lf = ceiling\n", floor, ceiling);

	/* third part: go through all coll data and set is_included */

	for (i = 0; i < size; ++i) {
		if (type != MANUAL) {
			coll_min = 1;
			coll_max = 0;
			for (j = 0; j < (lineno - 1); ++j) {
				if (colldata[i].coll[j] < coll_min)
					coll_min = colldata[i].coll[j];
				if (colldata[i].coll[j] > coll_max)
					coll_max = colldata[i].coll[j];
			}
		} else {
			floor = 0.1;
			ceiling = 1;
			printf("TYPE = MANUAL: ");
			printf("call# i=%d ", i);
			printf("with %Lf = floor, ", floor);
			printf("%Lf = ceiling\n", ceiling);
		}
		if ((coll_min >= floor) && (coll_max <= ceiling))
			colldata[i].is_included = TRUE;
		else
			colldata[i].is_included = FALSE;
	}

	/*  fourth part: go through file and remove rows with is_included as
	 *  FALSE, write everything else into the new file.
	 */

        fin = fopen(input, "r");
        fout = fopen(output, "w");

	skipno = 0;
	*numfiles = 0;

	for (i = 0; i < (lineno + 2); ++i) {
		if (i != 2) {
			filenum = size;
			for (j = 0; j < size; ++j) {
				digitnum = 0;
				num = filenum;
				while (num != 0) {
					digitnum++;
					num = num / 10;
				}

				if (colldata[j].is_included == TRUE) {
					if (i == 0)
						(*numfiles)++;
					for (k = 0; k < (60 +
							 (4 * (digitnum - 1)));
					     ++k) {
						fscanf(fin, "%c", &readchar);
						fprintf(fout, "%c", readchar);
					}
				} else
					for (k = 0; k < (60 +
							 (4 * (digitnum - 1)));
					     ++k)
						fscanf(fin, "%c", &readchar);
				--filenum;
			}
		}
		fscanf(fin, "%c", &readchar);
		if (readchar == '\n')
			fprintf(fout, "%c", readchar);
	}

	/* last part: free all of the dynamic arrays! */

	for (i = 0; i < size; ++i)
                free(colldata[i].coll);
	free(colldata);

	fclose(fin);
	fclose(fout);

	return OK;
}

status print_capture_1(char *input, char *output, int numfiles, int numlines)
{
	FILE *fin, *fout;
	char readchar;
	long double junk;
	int i, j = 0, k = 0, skipno = 0, capt;

	fin = fopen(input, "r");
	fout = fopen(output, "w");

	while ((fscanf(fin, "%c", &readchar) == 1) && (skipno != 2))
		if (readchar == '\n')
			skipno++;

	for (i = 0; i < numlines; ++i) {
                for (j = numfiles; j > 0; --j) {
                        for (k = 0; k < 4; ++k)
				switch (k)
				{
				case 3:
					if (fscanf(fin, "%d", &capt) != -1) {
						if (capt == -1)
							WARN_QUASI();
						fprintf(fout, "%2d", capt);
					}
					break;
				default:
					fscanf(fin, "%Lf", &junk);
					break;
				}
                }
		fprintf(fout, "%c", '\n');
	}

	fclose(fin);
	fclose(fout);

	return OK;
}

status print_capture_2(char *input, char *output, int  numfiles, int numlines)
{
	FILE *fin, *fout;
	char readchar;
	long double junk;
	int i, j, k, l = 0, place = 0, skipno = 0, capt;
	
	fout = fopen(output, "w");

	for (i = numfiles; i > 0; --i) {
		/* open file */
		fin = fopen(input, "r");
		
		while ((fscanf(fin, "%c", &readchar) == 1) && (skipno != 2))
			if (readchar == '\n')
				skipno++;

		for (j = 0; j < numlines; ++j) {
			for (k = numfiles; k > 0; --k)
				switch (l)
				{
				case 3:
					l = 0;
					if (fscanf(fin, "%d", &capt) != -1) {
						if (capt == -1)
							WARN_QUASI();
						if (k == i)
							fprintf(fout, "%2d", 
								capt);
					}
					break;
				default:
					++k;
					++l;
					fscanf(fin, "%Lf", &junk);
					break;
				}
		}
		/* close file and print '\n' */
		++place;
		skipno = 0;
		fprintf(fout, "%c", '\n');
		fclose(fin);
	}

	fclose(fout);

	return OK;
}
