/* -*- C -*- */
/*
 * Copyright (c) 2017-2020 Seagate Technology LLC and/or its Affiliates
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For any questions about this software or licensing,
 * please email opensource@seagate.com or cortx-questions@seagate.com.
 *
 * Original author:  Ganesan Umanesan <ganesan.umanesan@seagate.com>
 * Original creation date: 10-Jan-2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "c0appz.h"
#include "c0appz_isc.h"
#include "fid/fid.h"
#include "lib/buf.h"
#include "lib/misc.h"
#include "iscservice/isc.h"
#include "rpc/link.h"

char *prog;

/* main */
int main(int argc, char **argv)
{
	int rc = 0;

	prog = basename(strdup(argv[0]));

	/* check input */
	if (argc != 2) {
		fprintf(stderr,"Usage:\n");
		fprintf(stderr,"%s libpath\n", prog);
		return -1;
	}

	/* time in */
	c0appz_timein();

	c0appz_setrc(prog);
	c0appz_putrc();

	/* initialize resources */
	rc = c0appz_init(0);
	if (rc != 0) {
		fprintf(stderr,"error! c0appz_init() failed: %d\n", rc);
		return -2;
	}
	rc = c0appz_isc_api_register(argv[1]);
	if ( rc != 0)
		fprintf(stderr, "error! loading of library from %s failed. \n",
			argv[1]);
	/* free resources*/
	c0appz_free();

	/* time out */
	c0appz_timeout(0);

	/* success */
	if (rc == 0)
		fprintf(stderr,"%s success\n", prog);
	else
		fprintf(stderr,"%s fail\n", prog);
	return rc;
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
