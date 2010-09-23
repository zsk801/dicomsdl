/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "imagecodec.h"
#include "ijg_codec.h"

namespace dicom { //------------------------------------------------------


static const char *ijg_codec_name="IJGLIB";

int ijg_codec_encoder(uidtype tsuid,
		char *src, int srcstep, char **_dst, int *_dstlen,
		PIXELDATA_INFO *pi);
int ijg_codec_decoder(uidtype tsuid,
		char *src, int srclen, char *dst, int dststep,
		PIXELDATA_INFO *pi);

extern "C" int register_ijg_codec(struct _codec_registry_struct_ *codec)
{
	int n = 0;

	codec->name = (char *)ijg_codec_name;
	codec->tsuid = UID_JPEG_BASELINE_PROCESS_1;
	codec->encoder = ijg_codec_encoder;
	codec->decoder = ijg_codec_decoder;
	n++; codec++;

	codec->name = (char *)ijg_codec_name;
	codec->tsuid = UID_JPEG_EXTENDED_PROCESS_2AND4;
	codec->encoder = ijg_codec_encoder;
	codec->decoder = ijg_codec_decoder;
	n++; codec++;

	codec->name = (char *)ijg_codec_name;
	codec->tsuid = UID_JPEG_LOSSLESS_NON_HIERARCHICAL_PROCESS_14;
	codec->encoder = ijg_codec_encoder;
	codec->decoder = ijg_codec_decoder;
	n++; codec++;

	codec->name = (char *)ijg_codec_name;
	codec->tsuid = UID_JPEG_LOSSLESS_NON_HIERARCHICAL_FIRST_ORDER_PREDICTION_PROCESS_14;
	codec->encoder = ijg_codec_encoder;
	codec->decoder = ijg_codec_decoder;
	n++; codec++;

	return n;
}



#define GETWORD(c, n)	if (p+1<q) \
					{ c = p; n = ((unsigned char)p[0]) * 256 + \
								  (unsigned char)p[1]; p+=2; } \
					else goto EXITLOOP;
#define GETBYTE(c, n)	if (p<q) \
					{ c = p; p++;  n = (unsigned char )*c; } \
					else goto EXITLOOP;
#define SKIP(n)		if (p+n<=q) { p += n; } \
					else goto EXITLOOP;

JPEG_MODE scan_jpeg_header(PIXELDATA_INFO *pi, char *src, int srclen)
{
	int n;
	unsigned char *c;
	unsigned char *p = (unsigned char *)src,
			      *q = (unsigned char *)src+srclen;
	memset(pi, 0, sizeof(struct PIXELDATA_INFO));
	JPEG_MODE jmode = JPEG_UNKNOWN;

	GETWORD(c, n);
	if (n != 0xffd8) goto EXITLOOP; // check SOI
	for (;;) {
		GETBYTE(c, n);
		if (n != 0xff) break; // Not a JPEG file
		GETBYTE(c, n);
		switch (n) {
		case 0xc0: // SOF0
		case 0xc1: // SOF1
		case 0xc3: // SOF3
			// itu-t81.pdf, Figure B.3 - Frame header syntax
			if (*c == 0xc0)		jmode = JPEG_BASELINE;
			else if (*c == 0xc1)	jmode = JPEG_EXTENDED;
			else					jmode = JPEG_LOSSLESS; // 0xc3

			GETWORD(c, n);
			GETBYTE(c, pi->prec);
			GETWORD(c, pi->rows);
			GETWORD(c, pi->cols);
			GETBYTE(c, pi->ncomps);
			break;

		// Other SOFs
		case 0xc2:	case 0xc5:	case 0xc6:	case 0xc7:
		case 0xc8:	case 0xc9:	case 0xca:	case 0xcb:
		case 0xcd:	case 0xce:	case 0xcf: // SOF15
			goto EXITLOOP; // Can't handle other than SOF0, SOF1 and SOF3
			break;

		case 0xc4: /* DHT */ case 0xcc: /* DAC */ case 0xda: /* SOS */
		case 0xdb: /* DQT */ case 0xdc: /* DNL */ case 0xdd: /* DRI */
		case 0xde: /* DHP */ case 0xdf: /* EXP */ case 0xfe: /* COM */
		case 0xe0:	case 0xe1:	case 0xe2:	case 0xe3:
		case 0xe4:	case 0xe5:	case 0xe6:	case 0xe7:
		case 0xe8:	case 0xe9:	case 0xea:	case 0xeb:
		case 0xec:	case 0xed:	case 0xee:	case 0xef:	// APPn
			GETWORD(c, n);	// length
			SKIP(n-2);
			break;

		case 0xd0:	case 0xd1:	case 0xd2:	case 0xd3:
		case 0xd4:	case 0xd5:	case 0xd6:	case 0xd7:	// RSTn
		case 0x01: /* TEM */ case 0xd9: /* EOI */
			break;	// no parameter
		}

		if (pi->prec)
			break;
	}
	return jmode;

	EXITLOOP:
	printf("Error in scan_jpeg_header()");
	return jmode;
};


int encode_ijg_jpeg8
	(char *src, int srcstep, char **_dst, int *_dstlen,
		PIXELDATA_INFO *pi, JPEG_MODE jmode, int quality );
int encode_ijg_jpeg12
	(char *src, int srcstep, char **_dst, int *_dstlen,
		PIXELDATA_INFO *pi, JPEG_MODE jmode, int quality );
int encode_ijg_jpeg16
	(char *src, int srcstep, char **_dst, int *_dstlen,
		PIXELDATA_INFO *pi, JPEG_MODE jmode, int quality );


int ijg_codec_encoder(uidtype tsuid,
		char *src, int srcstep, char **_dst, int *_dstlen,
		PIXELDATA_INFO *pi)
{
	int ret;

	JPEG_MODE jmode;
	int quality;

	switch (tsuid) {
	case UID_JPEG_BASELINE_PROCESS_1:
		jmode = JPEG_BASELINE;
		break;
	case UID_JPEG_EXTENDED_PROCESS_2AND4:
		jmode = JPEG_EXTENDED;
		break;
	case UID_JPEG_LOSSLESS_NON_HIERARCHICAL_PROCESS_14:
		jmode = JPEG_LOSSLESS;
		break;
	case UID_JPEG_LOSSLESS_NON_HIERARCHICAL_FIRST_ORDER_PREDICTION_PROCESS_14:
	default:
		jmode = JPEG_LOSSLESS_SV1;
		break;
	}

	quality = pi->opt[OPT_JPEG_QUALITY];
	if (!quality) quality = 100;

	if (pi->prec > 12)
		ret = encode_ijg_jpeg16
					(src, srcstep, _dst, _dstlen, pi, jmode, quality);
	else if (pi->prec > 8)
		ret = encode_ijg_jpeg12
					(src, srcstep, _dst, _dstlen, pi, jmode, quality);
	else
		ret = encode_ijg_jpeg8
					(src, srcstep, _dst, _dstlen, pi, jmode, quality);

	return ret;
}

int decode_ijg_jpeg8
	(char *src, int srclen,char *dst, int dststep, PIXELDATA_INFO *pi);
int decode_ijg_jpeg12
	(char *src, int srclen,char *dst, int dststep, PIXELDATA_INFO *pi);
int decode_ijg_jpeg16
	(char *src, int srclen,char *dst, int dststep, PIXELDATA_INFO *pi);

int ijg_codec_decoder(uidtype tsuid,
		char *src, int srclen, char *dst, int dststep,
		PIXELDATA_INFO *pi)
{
	int ret;
	JPEG_MODE jmode;

	// scan jpeg header
	jmode = scan_jpeg_header(pi, src, srclen);

	if (jmode != JPEG_UNKNOWN) {
		if (pi->prec > 12)
			ret = decode_ijg_jpeg16 (src, srclen, dst, dststep, pi);
		else if (pi->prec > 8)
			ret = decode_ijg_jpeg12 (src, srclen, dst, dststep, pi);
		else
			ret = decode_ijg_jpeg8 (src, srclen, dst, dststep, pi);
	} else
		ret = DICOM_ERROR;  // not a jpeg file
	return ret;
}

} // namespace dicom ------------------------------------------------------
