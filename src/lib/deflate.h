/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#ifndef __DEFLATE_H__
#define __DEFLATE_H__

namespace dicom { //------------------------------------------------------

void inflate_dicomfile_a
	(char *data, int datasize, int offset, char **val_a, int *len_a);
void deflate_dicomfile_a
	(char *data, int datasize, int offset, int level, char **val_a, int *len_a);

} // namespace dicom -----------------------------------------------------

#endif // __DEFLATE_H__
