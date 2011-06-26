/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include "dicom.h"
#include "imagecodec.h"
#include "errormsg.h"
#include <stdio.h>
#include <string.h>

namespace dicom { //------------------------------------------------------

typedef enum {
	CODEC_IMPLICIT_VR_LITTLE_ENDIAN = 0,
	CODEC_EXPLICIT_VR_LITTLE_ENDIAN,
	CODEC_DEFLATED_EXPLICIT_VR_LITTLE_ENDIAN,
	CODEC_EXPLICIT_VR_BIG_ENDIAN,
	CODEC_JPEG_BASELINE_PROCESS_1,
	CODEC_JPEG_EXTENDED_PROCESS_2AND4,
	CODEC_JPEG_LOSSLESS_NON_HIERARCHICAL_PROCESS_14,
	CODEC_JPEG_LOSSLESS_NON_HIERARCHICAL_FIRST_ORDER_PREDICTION_PROCESS_14,
	CODEC_JPEG_LS_LOSSLESS_IMAGE_COMPRESSION,
	CODEC_JPEG_LS_LOSSY_NEAR_LOSSLESS_IMAGE_COMPRESSION,
	CODEC_JPEG_2000_IMAGE_COMPRESSION_LOSSLESS_ONLY,
	CODEC_JPEG_2000_IMAGE_COMPRESSION,
	CODEC_JPEG_2000_PART_2_MULTI_COMPONENT_IMAGE_COMPRESSION_LOSSLESS_ONLY,
	CODEC_JPEG_2000_PART_2_MULTI_COMPONENT_IMAGE_COMPRESSION,
	CODEC_JPIP_REFERENCED,
	CODEC_JPIP_REFERENCED_DEFLATE,
	CODEC_MPEG2_MAIN_PROFILE_MAIN_LEVEL,
	CODEC_MPEG2_MAIN_PROFILE_HIGH_LEVEL,
	CODEC_RLE_LOSSLESS,
	CODEC_UNKNOWN
} TSUID_INDEX;
#define NUMBER_OF_TSUIDTYPE (CODEC_UNKNOWN+1)

static t_pixeldata_encoder encoder_list[NUMBER_OF_TSUIDTYPE];
static t_pixeldata_decoder decoder_list[NUMBER_OF_TSUIDTYPE];

static struct _codec_registry_struct_ codec_list[64] = {
		{NULL, UID_UNKNOWN, NULL, NULL}
};

TSUID_INDEX uid2index(uidtype uid)
{
	switch (uid) {
		case UID_IMPLICIT_VR_LITTLE_ENDIAN:
			return CODEC_IMPLICIT_VR_LITTLE_ENDIAN;
		case UID_EXPLICIT_VR_LITTLE_ENDIAN:
			return CODEC_EXPLICIT_VR_LITTLE_ENDIAN;
		case UID_DEFLATED_EXPLICIT_VR_LITTLE_ENDIAN:
			return CODEC_DEFLATED_EXPLICIT_VR_LITTLE_ENDIAN;
		case UID_EXPLICIT_VR_BIG_ENDIAN:
			return CODEC_EXPLICIT_VR_BIG_ENDIAN;
		case UID_JPEG_BASELINE_PROCESS_1:
			return CODEC_JPEG_BASELINE_PROCESS_1;
		case UID_JPEG_EXTENDED_PROCESS_2AND4:
			return CODEC_JPEG_EXTENDED_PROCESS_2AND4;
		case UID_JPEG_LOSSLESS_NON_HIERARCHICAL_PROCESS_14:
			return CODEC_JPEG_LOSSLESS_NON_HIERARCHICAL_PROCESS_14;
		case UID_JPEG_LOSSLESS_NON_HIERARCHICAL_FIRST_ORDER_PREDICTION_PROCESS_14:
			return CODEC_JPEG_LOSSLESS_NON_HIERARCHICAL_FIRST_ORDER_PREDICTION_PROCESS_14;
		case UID_JPEG_LS_LOSSLESS_IMAGE_COMPRESSION:
			return CODEC_JPEG_LS_LOSSLESS_IMAGE_COMPRESSION;
		case UID_JPEG_LS_LOSSY_NEAR_LOSSLESS_IMAGE_COMPRESSION:
			return CODEC_JPEG_LS_LOSSY_NEAR_LOSSLESS_IMAGE_COMPRESSION;
		case UID_JPEG_2000_IMAGE_COMPRESSION_LOSSLESS_ONLY:
			return CODEC_JPEG_2000_IMAGE_COMPRESSION_LOSSLESS_ONLY;
		case UID_JPEG_2000_IMAGE_COMPRESSION:
			return CODEC_JPEG_2000_IMAGE_COMPRESSION;
		case UID_JPEG_2000_PART_2_MULTI_COMPONENT_IMAGE_COMPRESSION_LOSSLESS_ONLY:
			return CODEC_JPEG_2000_PART_2_MULTI_COMPONENT_IMAGE_COMPRESSION_LOSSLESS_ONLY;
		case UID_JPEG_2000_PART_2_MULTI_COMPONENT_IMAGE_COMPRESSION:
			return CODEC_JPEG_2000_PART_2_MULTI_COMPONENT_IMAGE_COMPRESSION;
		case UID_JPIP_REFERENCED:
			return CODEC_JPIP_REFERENCED;
		case UID_JPIP_REFERENCED_DEFLATE:
			return CODEC_JPIP_REFERENCED_DEFLATE;
		case UID_MPEG2_MAIN_PROFILE_MAIN_LEVEL:
			return CODEC_MPEG2_MAIN_PROFILE_MAIN_LEVEL;
		case UID_MPEG2_MAIN_PROFILE_HIGH_LEVEL:
			return CODEC_MPEG2_MAIN_PROFILE_HIGH_LEVEL;
		case UID_RLE_LOSSLESS:
			return CODEC_RLE_LOSSLESS;
		default:
			return CODEC_UNKNOWN;
	};
}

//#include "raw/raw_codec.h"

#ifdef USE_OPJ_CODEC
#include "opj/opj_codec.h"
#endif

#ifdef USE_IJG_CODEC
#include "ijg/ijg_codec.h"
#endif

#ifdef USE_IPP_CODEC
#include "ipp/ipp_codec.h"
#endif

#ifdef USE_LT_CODEC
#include "lt/lt_codec.h"
#endif

void register_codecs()
{
	memset(codec_list, 0, sizeof(codec_list));
	memset(codec_list, 0, sizeof(encoder_list));
	memset(codec_list, 0, sizeof(decoder_list));

	struct _codec_registry_struct_ *c = codec_list;

//	c += dicom::register_raw_codec(c);

#ifdef USE_IPP_CODEC
	c += dicom::register_ipp_codec(c);
#endif

#ifdef USE_LT_CODEC
	c += dicom::register_lt_codec(c);
#endif

#ifdef USE_IJG_CODEC
	c += dicom::register_ijg_codec(c);
#endif

#ifdef USE_OPJ_CODEC
	c += dicom::register_opj_codec(c);
#endif

	for (c = codec_list; c->tsuid != UID_UNKNOWN; c++) {
		encoder_list[uid2index(c->tsuid)] = c->encoder;
		decoder_list[uid2index(c->tsuid)] = c->decoder;
	}
}

DLLEXPORT int use_decoder(uidtype tsuid, const char *codec_name)
{
	if (codec_list[0].name == NULL) register_codecs();

	struct _codec_registry_struct_ *c = codec_list;
	for (c = codec_list; c->tsuid != UID_UNKNOWN; c++) {
		if (strcmp(codec_name, c->name) == 0) {
			decoder_list[uid2index(c->tsuid)] = c->decoder;
			return DICOM_OK;
		}
	}

	build_error_message("in ::use_decoder(...):"
		" cannot use decoder codec \"%s\" for \"%s\"",
		codec_name, uid_to_uidname(tsuid));
	return DICOM_DECODE_ERROR;
}

DLLEXPORT int use_encoder(uidtype tsuid, const char *codec_name)
{
	if (codec_list[0].name == NULL) register_codecs();

	struct _codec_registry_struct_ *c = codec_list;
	for (c = codec_list; c->tsuid != UID_UNKNOWN; c++) {
		if (strcmp(codec_name, c->name) == 0) {
			encoder_list[uid2index(c->tsuid)] = c->encoder;
			return DICOM_OK;
		}
	}

	build_error_message("in ::use_encoder(...):"
		" cannot use encoder codec \"%s\" for \"%s\"",
		codec_name, uid_to_uidname(tsuid));
	return DICOM_ENCODE_ERROR;
}

int encode_pixeldata(uidtype tsuid,
		char *src, int srcstep, char **dst, int *dstlen,
		struct PIXELDATA_INFO *pi)
{
	if (codec_list[0].name == NULL) register_codecs();

	t_pixeldata_encoder e = encoder_list[uid2index(tsuid)];
	if (e)
		return (*e)(tsuid, src, srcstep, dst, dstlen, pi);
	else {
		build_error_message("in ::encode_pixeldata(...):"
				" error during encoding pixeldata");
		return DICOM_ENCODE_ERROR;
	}
}

int decode_pixeldata(uidtype tsuid,
		char *src, int srclen, char *dst, int dststep,
		struct PIXELDATA_INFO *pi)
{
	if (codec_list[0].name == NULL) register_codecs();

	t_pixeldata_decoder d = decoder_list[uid2index(tsuid)];

	if (d)
		return (*d)(tsuid, src, srclen, dst, dststep, pi);
	else {
		build_error_message("in ::decode_pixeldata(...):"
			" error during decoding pixeldata");
		return DICOM_DECODE_ERROR;
	}
}

// functions for varargs to encoding/decoding options --------------------

void __set_opt(int *opt, OPT_TYPE opt_type, int opt_value)
{
	while (*opt != OPT_NULL) {
		if (*opt == opt_type) {
			opt ++;
			*opt = opt_value;
			return;
		}
		opt+=2;
	}
	*opt++ = opt_type;
	*opt++ = opt_value;
	*opt = OPT_NULL;
	return;
}

int __get_opt(int *opt, OPT_TYPE opt_type, int default_value)
{
	while (*opt != OPT_NULL) {
		if (*opt == opt_type) {
			opt++;
			return *opt;
		}
		opt += 2;
	}
	return default_value;
}

} // namespace dicom -----------------------------------------------------
