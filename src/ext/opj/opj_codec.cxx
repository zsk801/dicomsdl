/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include "imagecodec.h"
#include "opj_codec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libopenjpeg/openjpeg.h"

namespace dicom { //-------------------------------------------------------

static const char *opj_codec_name="OPENJPEG";

int opj_codec_encoder(uidtype tsuid,
		char *src, int srcstep, char **_dst, int *_dstlen,
		struct PIXELDATA_INFO *pi);
int opj_codec_decoder(uidtype tsuid,
		char *src, int srclen, char *dst, int dststep,
		struct PIXELDATA_INFO *pi);

extern "C" int register_opj_codec(struct _codec_registry_struct_ *codec)
{
	int n = 0;

	codec->name = (char *)opj_codec_name;
	codec->tsuid = UID_JPEG_2000_IMAGE_COMPRESSION_LOSSLESS_ONLY;
	codec->encoder = opj_codec_encoder;
	codec->decoder = opj_codec_decoder;
	n++; codec++;

	codec->name = (char *)opj_codec_name;
	codec->tsuid = UID_JPEG_2000_IMAGE_COMPRESSION;
	codec->encoder = opj_codec_encoder;
	codec->decoder = opj_codec_decoder;
	n++; codec++;

	return n;
}

// Error callback functions ----------------------------------------------

void error_callback(const char *msg, void *client_data) {
	FILE *stream = (FILE*)client_data;
	fprintf(stream, "[ERROR] %s", msg);
}

void warning_callback(const char *msg, void *client_data) {
	FILE *stream = (FILE*)client_data;
	fprintf(stream, "[WARNING] %s", msg);
}

void info_callback(const char *msg, void *client_data) {
	(void)client_data;
	fprintf(stdout, "[INFO] %s", msg);
}

// Encoder ---------------------------------------------------------------

opj_image_t* image_to_opj_image(
		unsigned char *src, int srcstep, PIXELDATA_INFO *pi);

int opj_codec_encoder(uidtype tsuid,
		char *src, int srcstep, char **_dst, int *_dstlen,
		PIXELDATA_INFO *pi)
{
	bool bSuccess;
	opj_cparameters_t parameters;	/* compression parameters */
	opj_event_mgr_t event_mgr;		/* event manager */
	opj_image_t *image = NULL;

	char *dst = NULL;
	int dstlen = 0;

	memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
	event_mgr.error_handler = error_callback;
	event_mgr.warning_handler = warning_callback;
	//event_mgr.info_handler = info_callback;

	opj_set_default_encoder_parameters(&parameters);

	parameters.cp_comment = (char *)"enc with opj";
	parameters.tcp_mct = pi->ncomps == 3 ? 1 : 0;

	if (tsuid == UID_JPEG_2000_IMAGE_COMPRESSION_LOSSLESS_ONLY)
		pi->opt[OPT_JPEG2K_QUALITY] = 0;
	parameters.tcp_rates[0] = (float)(pi->opt[OPT_JPEG2K_QUALITY]);

	if (pi->opt[OPT_JPEG2K_QUALITY] == 0)
		parameters.irreversible = 0;	// use reversible DWT 5-3

	parameters.tcp_numlayers = 1;
	parameters.cp_disto_alloc = 1;

	image = image_to_opj_image((unsigned char *)src, srcstep, pi);
	if (image) {
		// ref. PS3.5-2009, A.4.4 JPEG 2000 image compression
		// The optional JP2 file format header shall NOT be included.
		// The role of the JP2 file format header is fulfilled by the
		// non-pixel data attributes in the DICOM data set.

		opj_cio_t *cio = NULL;

		opj_cinfo_t* cinfo = opj_create_compress(CODEC_J2K);
		opj_set_event_mgr((opj_common_ptr)cinfo, &event_mgr, stderr);
		opj_setup_encoder(cinfo, &parameters, image);
		cio = opj_cio_open((opj_common_ptr)cinfo, NULL, 0);

		bSuccess = opj_encode(cinfo, cio, image, parameters.index);
		if (bSuccess) {
			dstlen = cio_tell(cio);
			dst = (char *)malloc(dstlen);
			memcpy(dst, cio->buffer, dstlen);
		} else {
			dstlen = 0;
			dst = NULL;
		}

		opj_cio_close(cio);
		opj_destroy_compress(cinfo);

		opj_image_destroy(image);
	}
	*_dst = dst;
	*_dstlen = dstlen;

	return DICOM_OK;
}

template <class T> void __copyto (T *src, int srcstep, int *dst, int w, int h)
{
	int wc;
	char *ptr = (char *)src;

	while (h--) {
		src = (T *)ptr;
		wc = w;
		while (wc --) *dst++ = *src++;
		ptr += srcstep;
	}
}

opj_image_t* image_to_opj_image
	(unsigned char *src, int srcstep, struct PIXELDATA_INFO *pi)
{

	int i, numcomps, w, h, precision, signedness;
	numcomps = pi->ncomps;
	w = pi->cols;
	h = pi->rows;
	precision = pi->prec;
	signedness = pi->sgnd;
	OPJ_COLOR_SPACE color_space
		= (numcomps == 1 ? CLRSPC_GRAY : CLRSPC_SRGB);

	opj_image_cmptparm_t cmptparm[3];	/* maximum of 3 components */
	memset(&cmptparm[0], 0, 3 * sizeof(opj_image_cmptparm_t));

	for(i = 0; i < numcomps; i++) {
		cmptparm[i].prec = precision;
		cmptparm[i].bpp = precision;
		cmptparm[i].sgnd = signedness;
		cmptparm[i].dx = 1; // set subsampling_dx as  1
		cmptparm[i].dy = 1; // set subsampling_dy as  1
		cmptparm[i].w = w;
		cmptparm[i].h = h;
	}

	opj_image_t * image =
			opj_image_create(numcomps, &cmptparm[0], color_space);
	if(!image) return NULL;

	image->x0 = 0;
	image->y0 = 0;
	image->x1 = w;
	image->y1 = h;

	if (srcstep < 0) src += -srcstep*(h-1);

	if (numcomps == 1) {
		if (precision > 8) {
			signedness?
			__copyto((short *)src, srcstep,
					image->comps[0].data, w, h):
			__copyto((unsigned short *)src, srcstep,
					image->comps[0].data, w, h);
		} else { // bpp == 1
			signedness?
			__copyto((char *)src, srcstep,
					image->comps[0].data, w, h):
			__copyto((unsigned char *)src, srcstep,
					image->comps[0].data, w, h);
		}
	} else {
		int *r = image->comps[0].data,
			*g = image->comps[1].data,
			*b = image->comps[2].data;
		unsigned char *p;
		int wc, hc = h;

		while (hc --) {
			p = src, wc = w;
			while (wc --) {
				*r++ = *p++; *g++ = *p++; *b++ = *p++;
			}
			src += srcstep;
		}
	}

	return image;
}


// Decoder ---------------------------------------------------------------

int opj_image_to_image(opj_image_t *image,
		unsigned char *dst, int dststep, struct PIXELDATA_INFO *pi);

opj_image_t * __decode_opj_jpeg2k(char *src, int srclen);

int opj_codec_decoder(uidtype tsuid,
		char *src, int srclen, char *dst, int dststep,
		struct PIXELDATA_INFO *pi)
{
	int ret;
	opj_image_t *image = __decode_opj_jpeg2k(src, srclen);

	if (image) {
		ret = opj_image_to_image(image, (unsigned char *)dst, dststep, pi);
		opj_image_destroy(image);
	}
	else
		ret = DICOM_ERROR;

	return ret;
}

// Decoder : subroutines -------------------------------------------------

void dump_components(opj_image_t *image)
{
	printf("> image has %d component(s).\n", image->numcomps);
	printf("> (%d,%d - %d,%d), colorspace=%d\n",
			image->x0, image->y0, image->x1, image->y1,
			image->color_space);

	for (int i = 0; i < image->numcomps; i++) {
		printf(">> component %d: ", i);
		printf(">> dx=%d, dy=%d, w=%d, h=%d, x0=%d, y0=%d, "
				"prec=%d, bpp=%d, sgnd=%d, resno_decoded=%d, factor=%d\n",
				image->comps[i].dx,		image->comps[i].dy,
				image->comps[i].w,		image->comps[i].h,
				image->comps[i].x0,		image->comps[i].y0,
				image->comps[i].prec,	image->comps[i].bpp,
				image->comps[i].sgnd,	image->comps[i].resno_decoded,
				image->comps[i].factor);
	}
}

bool is_supported_gray_format(opj_image_t *image)
{
	return (image->numcomps == 1 && image->comps[0].factor == 0);
}

bool is_supported_rgb_format(opj_image_t *image)
{
	if (image->numcomps != 3) return false;
	return (image->comps[0].w == image->comps[1].w
			&& image->comps[1].w == image->comps[2].w
			&& image->comps[0].h == image->comps[1].h
			&& image->comps[1].h == image->comps[2].h
			&& image->comps[0].dx == image->comps[1].dx
			&& image->comps[1].dx == image->comps[2].dx
			&& image->comps[0].dy == image->comps[1].dy
			&& image->comps[1].dy == image->comps[2].dy
			&& image->comps[0].prec == 8
			&& image->comps[1].prec == 8
			&& image->comps[2].prec == 8
			&& image->comps[0].factor == 0
			&& image->comps[1].factor == 0
			&& image->comps[2].factor == 0);
}

template <class T> void __copyfrom (T *dst, int dststep, int *src, int w, int h)
{
	int wc;
	char *ptr = (char *)dst;

	while (h--) {
		dst = (T *)ptr;
		wc = w;
		while (wc --) *dst++ = *src++;
		ptr += dststep;
	}
}

int opj_image_to_image
	(opj_image_t *image, unsigned char *dst, int dststep,
		struct PIXELDATA_INFO *pi)
{
	int width = 0, height = 0, precision = 0;
	int ncomponent = image->numcomps;
	int signedness = 0;

	// Gray scale image
	if (is_supported_gray_format(image))
	{
		width = image->comps[0].w;
		height = image->comps[0].h;
		precision = image->comps[0].prec;
		signedness = image->comps[0].sgnd;

		if (dststep < 0) dst += -dststep*(height-1);
		if (precision > 8) {
			signedness?
			__copyfrom((short *)dst, dststep,
					image->comps[0].data, width, height):
			__copyfrom((unsigned short *)dst, dststep,
					image->comps[0].data, width, height);
		} else { // bpp == 1
			signedness?
			__copyfrom((char *)dst, dststep,
					image->comps[0].data, width, height):
			__copyfrom((unsigned char *)dst, dststep,
					image->comps[0].data, width, height);
		}
	}

	// 24bit RGB image
	else if (is_supported_rgb_format(image))
	{
		width = image->comps[0].w;
		height = image->comps[0].h;
		precision = image->comps[0].prec; // 8
		signedness = image->comps[0].sgnd;

		if (dststep < 0) dst += -dststep*(height-1);
		int *r = image->comps[0].data,
			*g = image->comps[1].data,
			*b = image->comps[2].data;
		unsigned char *q;
		int wc, h = height;

		while (h --) {
			q = dst, wc = width;
			while (wc --) {
				*q++ = *r++; *q++ = *g++; *q++ = *b++;
			}
			dst += dststep;
		}
	}

	// Can't decode image
	else {
		puts("Can't decode image with this format");
		dump_components(image);
		return DICOM_ERROR;
	}

	pi->cols = width;
	pi->rows = height;
	pi->ncomps = ncomponent;
	pi->prec = precision;
	pi->sgnd = signedness;

	return DICOM_OK;
}

int is_jp2 (char *src, int srclen)
{
	if (srclen < 8) return -1;
	if (!memcmp(src+4, "jP  ", 4))
		return 1; // jp2
	return 0; // try codestream
}

opj_image_t * __decode_opj_jpeg2k(char *src, int srclen)
{
	opj_dparameters_t parameters;	/* decompression parameters */
	opj_event_mgr_t event_mgr;		/* event manager */
	opj_dinfo_t* dinfo = NULL;	/* handle to a decompressor */
	opj_cio_t *cio = NULL;
	opj_image_t *image = NULL;

	memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
	event_mgr.error_handler = error_callback;
	event_mgr.warning_handler = warning_callback;
	//event_mgr.info_handler = info_callback;

	opj_set_default_decoder_parameters(&parameters);

	if (is_jp2(src, srclen) > 0)
	{ // -- JP2
		dinfo = opj_create_decompress(CODEC_JP2);
		opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, stderr);
		opj_setup_decoder(dinfo, &parameters);
		cio = opj_cio_open((opj_common_ptr)dinfo,
				(unsigned char *) src, srclen);
		image = opj_decode(dinfo, cio);
		opj_cio_close(cio);
	} else { // -- try codestream
		dinfo = opj_create_decompress(CODEC_J2K);
		opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, stderr);
		opj_setup_decoder(dinfo, &parameters);
		cio = opj_cio_open((opj_common_ptr)dinfo,
				(unsigned char *) src, srclen);
		image = opj_decode(dinfo, cio);
		opj_cio_close(cio);
	}

	if (dinfo) opj_destroy_decompress(dinfo);

	return image;
}

} // namespace dicom ------------------------------------------------------
