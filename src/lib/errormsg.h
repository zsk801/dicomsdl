/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#ifndef __DICOMERROR_H__
#define __DICOMERROR_H__

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "dicom.h"

namespace dicom { //------------------------------------------------------

char *build_error_message (char * format, ...);
char *append_error_message(char * format, ...);

void clear_error_message();

} // namespace nccsd -----------------------------------------------------

#endif // __ERROR_H__
