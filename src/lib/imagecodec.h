/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include "dicom.h"

#ifndef __IMAGECODEC_H__
#define __IMAGECODEC_H__

namespace dicom { //------------------------------------------------------

/* PIXELDATA_INFO usage
 *
 * PIXELDATA_INFO pi;
 * memset(pi, 0, sizeof(PIXELDATA_INFO));
 * pi.opt[OPT_JPEG2K_MODE] = JPEG2K_LOSSY;
 * pi.opt[OPT_JPEG2K_QUALITY] = 10;
 * encode_pixeldata(dicomuid, ..., &pi);
 */

typedef enum {
	OPT_NULL = 0,
	OPT_JPEG2K_MODE,
	OPT_JPEG2K_QUALITY,
	OPT_JPEG_MODE,
	OPT_JPEG_QUALITY,
	NUMBER_OF_OPT_TYPE
} OPT_TYPE;

const int MODE_UNKNOWN	= 0;
const int MODE_GRAY		= 1;
const int MODE_RGB		= 2;

struct PIXELDATA_INFO {
	int prec;	// bits per sample
	int rows;
	int cols;
	int sgnd;	// signedness; signed = 1, unsigned = 0
	int ncomps; // number of components
	int mode;	// MODE_RGB, MODE_GRAY
	int opt[NUMBER_OF_OPT_TYPE];
};

typedef int (*t_pixeldata_encoder)	(uidtype,
		char *, int, char **, int *, PIXELDATA_INFO *);
typedef int (*t_pixeldata_decoder)	(uidtype,
		char *, int, char *, int, PIXELDATA_INFO *);

struct _codec_registry_struct_ {
	char *name;
	uidtype tsuid;
	t_pixeldata_encoder encoder;
	t_pixeldata_decoder decoder;
};

int encode_pixeldata(uidtype tsuid,
		char *src, int srcstep,
		char **dst, int *dstlen,
		struct PIXELDATA_INFO *pi);

int decode_pixeldata(uidtype tsuid,
		char *src, int srclen,
		char *dst, int dststep,
		struct PIXELDATA_INFO *pi);

int register_codec(uidtype *tsuid, void *encoder, void *decoder);

typedef enum {
	JPEG_UNKNOWN = 0,
	JPEG_BASELINE = 1,
	JPEG_EXTENDED = 2,
	JPEG_LOSSLESS = 14,
	JPEG_LOSSLESS_SV1 = 70
} JPEG_MODE;

typedef enum {
	JPEG2K_UNKNOWN = 0,
	JPEG2K_LOSSY = 1,
	JPEG2K_LOSSLESS = 2
} JPEG2K_MODE;

} // namespace dicom -----------------------------------------------------

#endif // __IMAGECODEC_H__
