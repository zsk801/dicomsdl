/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include "dicom.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	if (argc < 4) {
		puts("changetsuid input.dcm output.dcm tsuid_number [optional quality]");
		puts("tsuid 0 = UID_IMPLICIT_VR_LITTLE_ENDIAN");
		puts("tsuid 1 = UID_EXPLICIT_VR_LITTLE_ENDIAN");
		puts("tsuid 2 = UID_DEFLATED_EXPLICIT_VR_LITTLE_ENDIAN");
		puts("tsuid 3 = UID_EXPLICIT_VR_BIG_ENDIAN");
		puts("tsuid 4 = UID_JPEG_BASELINE_PROCESS_1");
		puts("tsuid 5 = UID_JPEG_EXTENDED_PROCESS_2AND4");
		puts("tsuid 6 = UID_JPEG_LOSSLESS_NON_HIERARCHICAL_FIRST_ORDER_PREDICTION_PROCESS_14");
		puts("tsuid 7 = UID_JPEG_2000_IMAGE_COMPRESSION_LOSSLESS_ONLY");
		puts("tsuid 8 = UID_JPEG_2000_IMAGE_COMPRESSION");
		return 0;
	}

	dicom::use_decoder(dicom::UID_JPEG_2000_IMAGE_COMPRESSION_LOSSLESS_ONLY, "IPPUIC");
	dicom::use_decoder(dicom::UID_JPEG_2000_IMAGE_COMPRESSION, "IPPUIC");
	try {
		dicom::dicomfile df;

		df.load_from_file(argv[1]);

		dicom::uidtype tsuid;
		switch (argv[3][0]) {
		case '0':
			tsuid = dicom::UID_IMPLICIT_VR_LITTLE_ENDIAN;
			break;
		case '1':
			tsuid = dicom::UID_EXPLICIT_VR_LITTLE_ENDIAN;
			break;
		case '2':
			tsuid = dicom::UID_DEFLATED_EXPLICIT_VR_LITTLE_ENDIAN;
			break;
		case '3':
			tsuid = dicom::UID_EXPLICIT_VR_BIG_ENDIAN;
			break;
		case '4':
			tsuid = dicom::UID_JPEG_BASELINE_PROCESS_1;
			break;
		case '5':
			tsuid = dicom::UID_JPEG_EXTENDED_PROCESS_2AND4;
			break;
		case '6':
			tsuid = dicom::UID_JPEG_LOSSLESS_NON_HIERARCHICAL_FIRST_ORDER_PREDICTION_PROCESS_14;
			break;
		case '7':
			tsuid = dicom::UID_JPEG_2000_IMAGE_COMPRESSION_LOSSLESS_ONLY;
			break;
		case '8':
			tsuid = dicom::UID_JPEG_2000_IMAGE_COMPRESSION;
			break;
		default:
			tsuid = dicom::UID_EXPLICIT_VR_LITTLE_ENDIAN;
		}

		int quality = 0;
		if (argc > 4) {
			quality = atoi(argv[4]);
		}

		if (df.change_pixelencoding(tsuid, quality)) {
			throw ("Error while changing encoding %s...", argv[1]);
		}

		char *val; int len;
		df.save_a(&val, &len);
		{
			FILE *fout = fopen(argv[2], "wb");
			fwrite(val, len, 1, fout);
			fclose(fout);
		}
		free(val);

	} catch (char *err) {
		printf("%s\n", err);
	}

	return 0;
}
