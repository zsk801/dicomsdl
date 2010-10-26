/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#ifndef __RECORDOFFSET_H__
#define __RECORDOFFSET_H__

#include "dicom.h"

namespace dicom { //------------------------------------------------------

struct recordoffset {
	uint32 offset;
	dicomfile* dfobj;
	dataset* assoc_ds;

	recordoffset(uint32 _offset, dicomfile *_dfobj)
	{
		offset = _offset;
		dfobj = _dfobj;
		assoc_ds = NULL;
	};

	recordoffset(dataset *ds)
	{
		offset = -1;
		dfobj = NULL;
		assoc_ds = ds;
	};


	recordoffset()
	{
		offset = -1;
		dfobj = NULL;
		assoc_ds = NULL;
	};

	void from_dataset(dataset *ds);
	dataset* to_dataset();
};


} // namespace dicom ------------------------------------------------------

#endif // __RECORDOFFSET_H__
