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

#ifndef __IPP_CODEC_H__
#define __IPP_CODEC_H__

namespace dicom { //------------------------------------------------------

extern "C" int register_ipp_codec(struct _codec_registry_struct_ *codec);

} // namespace dicom -----------------------------------------------------

#endif
