/* -*- C -*- */
/*
 * COPYRIGHT 2014 SEAGATE LLC
 *
 * THIS DRAWING/DOCUMENT, ITS SPECIFICATIONS, AND THE DATA CONTAINED
 * HEREIN, ARE THE EXCLUSIVE PROPERTY OF SEAGATE LLC,
 * ISSUED IN STRICT CONFIDENCE AND SHALL NOT, WITHOUT
 * THE PRIOR WRITTEN PERMISSION OF SEAGATE TECHNOLOGY LIMITED,
 * BE REPRODUCED, COPIED, OR DISCLOSED TO A THIRD PARTY, OR
 * USED FOR ANY PURPOSE WHATSOEVER, OR STORED IN A RETRIEVAL SYSTEM
 * EXCEPT AS ALLOWED BY THE TERMS OF SEAGATE LICENSES AND AGREEMENTS.
 *
 * YOU SHOULD HAVE RECEIVED A COPY OF SEAGATE'S LICENSE ALONG WITH
 * THIS RELEASE. IF NOT PLEASE CONTACT A SEAGATE REPRESENTATIVE
 * http://www.xyratex.com/contact
 *
 * Original author:  Ganesan Umanesan <ganesan.umanesan@seagate.com>
 * Original creation date: 24-Jan-2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include "c0appz.h"

/*
 ******************************************************************************
 * EXTERN VARIABLES
 ******************************************************************************
 */
extern int perf; /* performance */
extern int qos_total_weight; /* bytes read or written */
extern pthread_mutex_t qos_lock;

/*
 * qos_print_bw(void)
 */
int qos_print_bw(void)
{
	double bw=0;
	bw=(double)qos_total_weight/1000000;
	/* reset total weight */
	pthread_mutex_lock(&qos_lock);
	qos_total_weight=0;
	pthread_mutex_unlock(&qos_lock);
	printf("bw = %06.3f MB/s\n",bw);
	return 0;
}

/* thread function */
void *threadfunc(void *arg)
{
    while(1)
    {
        qos_print_bw();
        sleep(1);
    }
    return 0;
}

/* main */
int main(int argc, char **argv)
{
	int64_t idh;	/* object id high 	*/
	int64_t idl;	/* object is low	*/
	int bsz;		/* block size 		*/
	int cnt;		/* count			*/
	char *fname;	/* input filename 	*/
	struct stat fs;	/* file statistics	*/
	int opt;		/* options			*/

	/* getopt */
	while((opt = getopt(argc, argv, ":p"))!=-1){
		switch(opt){
			case 'p':
				perf = 1;
				break;
			case ':':
				fprintf(stderr,"option needs a value\n");
				break;
			case '?':
				fprintf(stderr,"unknown option: %c\n", optopt);
				break;
			default:
				fprintf(stderr,"unknown default option: %c\n", optopt);
				break;
		}
	}


	/* check input */
	if(argc-optind!=4){
		fprintf(stderr,"Usage:\n");
		fprintf(stderr,"%s [options] idh idl filename bsz\n", basename(argv[0]));
		return -1;
	}

	/* time in */
	c0appz_timein();

	/* c0rcfile
	 * overwrite .cappzrc to a .[app]rc file.
	 */
	char str[256];
	sprintf(str,".%src",basename(argv[0]));
	c0appz_setrc(str);
	c0appz_putrc();

	/* set input */
	idh = atoll(argv[optind+0]);
	idl = atoll(argv[optind+1]);
	fname = argv[optind+2];
	bsz = atoi(argv[optind+3]);

	/* extend */
	stat(fname, &fs);
	cnt = (fs.st_size + bsz - 1)/bsz;
	truncate(fname,fs.st_size + bsz - 1);

	/* initialize resources */
	if(c0appz_init(0)!=0){
		fprintf(stderr,"error! clovis initialization failed.\n");
		return -2;
	}

	/* time out/in */
	if(perf){
		fprintf(stderr,"%4s","init");
		c0appz_timeout(0);
		c0appz_timein();
	}

	if(perf){
		pthread_t tid;
		pthread_create(&tid, NULL, &threadfunc, NULL);
	}

	/* copy */
	if (c0appz_cp(idh,idl,fname,bsz,cnt) != 0) {
		fprintf(stderr,"error! copy object failed.\n");
		c0appz_free();
		return -3;
	};

	/* resize */
	truncate(fname,fs.st_size);
	printf("%s %d\n",fname, (int)fs.st_size);

	/* time out/in */
	if(perf){
		fprintf(stderr,"%4s","i/o");
		c0appz_timeout((uint64_t)bsz * (uint64_t)cnt);
		c0appz_timein();
	}

	/* free resources*/
	c0appz_free();

	/* time out */
	if(perf){
		fprintf(stderr,"%4s","free");
		c0appz_timeout(0);
	}

	/* success */
	fprintf(stderr,"%s success\n",basename(argv[0]));
	return 0;
}

/*
 *  Local variables:
 *  c-indentation-style: "K&R"
 *  c-basic-offset: 8
 *  tab-width: 8
 *  fill-column: 80
 *  scroll-step: 1
 *  End:
 */
