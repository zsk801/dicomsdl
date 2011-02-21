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

char *build_error_message (const char * format, ...);
char *append_error_message(const char * format, ...);

void clear_error_message();

DLLEXPORT extern int display_debug_message;
DLLEXPORT extern int display_warning_message;
DLLEXPORT extern int display_error_message;

#define LOG_DEBUG_MESSAGE(...) do { \
	if (display_debug_message) _debug_message(__VA_ARGS__); \
	} while(0)
#define LOG_ERROR_MESSAGE(...) do { \
	if (display_error_message) _error_message(__VA_ARGS__); \
	} while (0)
#define LOG_WARNING_MESSAGE(...) do {\
	if (display_warning_message) _warning_message(__VA_ARGS__); \
	} while (0)

DLLEXPORT void _debug_message(const char * format, ...);
DLLEXPORT void _error_message(const char * format, ...);
DLLEXPORT void _warning_message(const char * format, ...);

} // namespace nccsd -----------------------------------------------------

#endif // __ERROR_H__
