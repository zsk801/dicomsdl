/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include <map>
#include "dicom.h"
#include "recordoffset.h"

namespace dicom { //------------------------------------------------------

void recordoffset::from_dataset(dataset *ds)
{
	assoc_ds = ds;
};

dataset* recordoffset::to_dataset()
{
	if (assoc_ds) return assoc_ds;
	if (offset == -1) return NULL;
	assoc_ds = dfobj->dataset_at(offset);
	return assoc_ds;
};

} // namespace dicom -----------------------------------------------------
