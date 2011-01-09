/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include <stdio.h>
#include "dicom.h"

int main(int argc, char **argv)
{
	if (argc < 2) {
		puts("dicomfiledump input.dcm");
		return 0;
	}

	dicom::dicomfile *df;
	df = dicom::open_dicomfile(argv[1]);

	if (df) {
		puts(df->dump_string().c_str());
		delete df;
	} else {
		printf("%s\n", dicom::get_error_message());
	}

	return 0;
}
