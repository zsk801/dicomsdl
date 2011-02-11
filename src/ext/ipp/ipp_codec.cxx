/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

/*
 * This code is a modification in the example from Intel's ipp-sample.
 */

#include "imagecodec.h"
#include "ipp_codec.h"

#include "ipp.h"
#include "membuffin.h"
#include "uic_jp2_dec.h"
#include "uic_jpeg2000_dec.h"

using namespace UIC;

typedef unsigned int uint;

namespace dicom { //-------------------------------------------------------

static const char *opj_codec_name="IPPUIC";

int ipp_codec_encoder(uidtype tsuid,
		char *src, int srcstep, char **_dst, int *_dstlen,
		struct PIXELDATA_INFO *pi);
int ipp_codec_decoder(uidtype tsuid,
		char *src, int srclen, char *dst, int dststep,
		struct PIXELDATA_INFO *pi);

extern "C" int register_ipp_codec(struct _codec_registry_struct_ *codec)
{
	int n = 0;

	codec->name = (char *)opj_codec_name;
	codec->tsuid = UID_JPEG_2000_IMAGE_COMPRESSION_LOSSLESS_ONLY;
	//codec->encoder = ipp_codec_encoder;
	codec->decoder = ipp_codec_decoder;
	n++; codec++;

	codec->name = (char *)opj_codec_name;
	codec->tsuid = UID_JPEG_2000_IMAGE_COMPRESSION;
	//codec->encoder = ipp_codec_encoder;
	codec->decoder = ipp_codec_decoder;
	n++; codec++;

	return n;
}

// Encoder ---------------------------------------------------------------

int ipp_codec_encoder(uidtype tsuid,
		char *src, int srcstep, char **_dst, int *_dstlen,
		struct PIXELDATA_INFO *pi)
{
	return 0;
}

// Decoder ---------------------------------------------------------------

int IsJP2(BaseStreamInput& in)
{
  unsigned char buf[4];
  UIC::BaseStream::TSize cnt;

  if(UIC::BaseStream::StatusOk != in.Seek(4, UIC::BaseStreamInput::Beginning))
    return -1;

  if(UIC::BaseStream::StatusOk != in.Read(buf, 4*sizeof(char),cnt))
    return -1;

  if(UIC::BaseStream::StatusOk != in.Seek(0, UIC::BaseStreamInput::Beginning))
    return -1;

  if(buf[0] == 0x6a && buf[1] == 0x50 && buf[2] == 0x20 && buf[3] == 0x20)
    return 1;

  return 0;
} // IsJP2()


int ipp_codec_decoder(uidtype tsuid,
		char *src, int srclen, char *dst, int dststep,
		struct PIXELDATA_INFO *pi)
{
	int	i, du, isJP2;
	Image                 imagePn;
	ImageColorSpec        colorSpec;
	ImageDataOrder        dataOrderPn;
	BaseStreamDiagn       diagnOutput;
	ImageSamplingGeometry geometry;
	JP2Decoder JP2decoder;
	JPEG2000Decoder JP2Kdecoder;
	CMemBuffInput mi;

	mi.Open((const Ipp8u *)src, srclen);

	isJP2 = IsJP2(mi);
	if(isJP2 < 0)
		return DICOM_DECODE_ERROR;


	// Decode ---------------------------------------

	if (isJP2) {
		if(ExcStatusOk != JP2decoder.Init())
			return DICOM_DECODE_ERROR;

		JP2decoder.SetNOfThreads(1); // param.nthreads

		if(ExcStatusOk != JP2decoder.AttachStream(mi))
			return DICOM_DECODE_ERROR;

		JP2decoder.AttachDiagnOut(diagnOutput);

		if(ExcStatusOk != JP2decoder.ReadHeader(colorSpec, geometry))
			return DICOM_DECODE_ERROR;

		pi->ncomps = geometry.NOfComponents();

		dataOrderPn.SetDataType(T32s);
		du = sizeof(Ipp32s);
		dataOrderPn.ReAlloc(Plane, pi->ncomps);

		for(i = 0; i < pi->ncomps; i++) {
			dataOrderPn.PixelStep()[i] = NOfBytes(dataOrderPn.DataType());
			dataOrderPn.LineStep() [i] = geometry.RefGridRect().Width() * du;
		}

		geometry.SetEnumSampling(S444);

		imagePn.Buffer().ReAlloc(dataOrderPn, geometry);

		imagePn.ColorSpec().ReAlloc(pi->ncomps);
		imagePn.ColorSpec().SetColorSpecMethod(Enumerated);
		imagePn.ColorSpec().SetComponentToColorMap(Direct);

		ImageEnumColorSpace in_color;
		in_color = colorSpec.EnumColorSpace();
		imagePn.ColorSpec().SetEnumColorSpace(in_color);

		if(ExcStatusOk != JP2decoder.ReadData(imagePn.Buffer().DataPtr(),dataOrderPn))
			return DICOM_DECODE_ERROR;
	}
	else {
		if(ExcStatusOk != JP2Kdecoder.Init())
			return DICOM_ERROR;

		JP2Kdecoder.SetNOfThreads(1);

	    if(ExcStatusOk != JP2Kdecoder.AttachStream(mi))
			return DICOM_ERROR;

	    JP2Kdecoder.AttachDiagnOut(diagnOutput);

	    if(ExcStatusOk != JP2Kdecoder.ReadHeader(colorSpec, geometry))
	    	return DICOM_DECODE_ERROR;

	    pi->ncomps = geometry.NOfComponents();

	    dataOrderPn.SetDataType(T32s);
	    du = sizeof(Ipp32s);
	    dataOrderPn.ReAlloc(Plane, pi->ncomps);

	    for(i = 0; i < pi->ncomps; i++)
	    {
	      dataOrderPn.PixelStep()[i] = NOfBytes(dataOrderPn.DataType());
	      dataOrderPn.LineStep() [i] = geometry.RefGridRect().Width() * du;
	    }

	    geometry.SetEnumSampling(S444);

	    imagePn.Buffer().ReAlloc(dataOrderPn, geometry);

	    imagePn.ColorSpec().ReAlloc(pi->ncomps);
	    imagePn.ColorSpec().SetColorSpecMethod(Enumerated);
	    imagePn.ColorSpec().SetComponentToColorMap(Direct);

	    ImageEnumColorSpace in_color;
	    in_color = colorSpec.EnumColorSpace();
	    imagePn.ColorSpec().SetEnumColorSpace(in_color);

	    if(ExcStatusOk != JP2Kdecoder.ReadData(imagePn.Buffer().DataPtr(),dataOrderPn))
	    	return DICOM_DECODE_ERROR;
	}

	// Copy Decoded Image ---------------------------------------

	int       tmpRowSize;
	IppiSize  sz;
	IppiSize  size;

	size.width  = pi->cols = imagePn.Buffer().BufferFormat().SamplingGeometry().RefGridRect().Width();
	size.height = pi->rows = imagePn.Buffer().BufferFormat().SamplingGeometry().RefGridRect().Height();
	pi->prec = colorSpec.DataRange()->BitDepth() + 1;
	pi->sgnd = colorSpec.DataRange()->IsSigned();

	switch(colorSpec.EnumColorSpace())
	{
		case RGB:       pi->mode = MODE_RGB; break;
		case Grayscale: pi->mode = MODE_GRAY; break;
		default:        pi->mode = MODE_UNKNOWN;
	}

	switch(pi->ncomps)
	{
	case 1:
	{
		if(pi->prec <= 8)
		{
			ippiConvert_32s8u_C1R(imagePn.Buffer().DataPtr()[0].p32s,
					dataOrderPn.LineStep()[0],
					(Ipp8u*)dst, dststep, size);
		}
		else
		{
			int i, j;
			Ipp32s* pSrc = imagePn.Buffer().DataPtr()[0].p32s;
			Ipp16u* pDst = (Ipp16u*)dst;

			for(i = 0; i < pi->rows; i++)
			{
				for(j = 0; j < pi->cols; j++)
					pDst[j] = (Ipp16u)pSrc[j];
				pSrc = (Ipp32s*)((Ipp8u*)pSrc + dataOrderPn.LineStep()[0]);
				pDst = (Ipp16u*)((Ipp8u*)pDst + dststep);
			}
		}
	}
	break;
	case 3:
	{
		if(pi->prec <= 8)
		{
			Ipp32s* p[3];

			sz.height = 1;
			sz.width  = pi->cols;

			p[0] = imagePn.Buffer().DataPtr()[0].p32s;
			p[1] = imagePn.Buffer().DataPtr()[1].p32s;
			p[2] = imagePn.Buffer().DataPtr()[2].p32s;

			tmpRowSize = size.width * pi->ncomps * sizeof(Ipp32s);
			Ipp32s* tmpRow = (Ipp32s*)ippMalloc(tmpRowSize);

			Ipp8u* ptr = (Ipp8u*) dst;

			for(int i = 0; i < size.height; i++)
			{
				ippiCopy_32s_P3C3R(p, dataOrderPn.LineStep()[0], tmpRow, tmpRowSize, sz);
				ippiConvert_32s8u_C3R(tmpRow, tmpRowSize, ptr, dststep, sz);

				ptr += dststep;

				p[0] = (Ipp32s*)((Ipp8u*)p[0] + dataOrderPn.LineStep()[0]);
				p[1] = (Ipp32s*)((Ipp8u*)p[1] + dataOrderPn.LineStep()[1]);
				p[2] = (Ipp32s*)((Ipp8u*)p[2] + dataOrderPn.LineStep()[2]);
			}
		}
	}
	default:
	break;
	}
	mi.Close();
	return DICOM_OK;
}

} // namespace dicom ------------------------------------------------------
