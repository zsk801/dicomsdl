/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#ifndef __MEMUTIL_H__
#define __MEMUTIL_H__

namespace dicom { //------------------------------------------------------

// routines for changing endianess
void swap16(void *src);
void swap16(void *src, int srclen);
void swap32(void *src);
void swap32(void *src, int srclen);
void swap64(void *src);
void swap64(void *src, int srclen);

} // namespace dicom -----------------------------------------------------

#endif // __MEMUTIL_H__
