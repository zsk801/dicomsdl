/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include <stdio.h>
#include "dicom.h"
#include "errormsg.h"
#include <ctype.h>

namespace dicom { //------------------------------------------------------

/* -----------------------------------------------------------------------
 * logger functions
 */

int display_debug_message = 0;
int display_warning_message = 1;
int display_error_message = 0;

void set_display_debug_message(int b)
{
	display_debug_message = b;
}

void set_display_warning_message(int b)
{
	display_warning_message = b;
}

void set_display_error_message(int b)
{
	display_error_message = b;
}

void default_debuglogfunc(char  *msg)
{
	fprintf(stdout, "%s", msg);
}

void default_warninglogfunc(char  *msg)
{
	fprintf(stderr, "%s", msg);
}

void default_errorlogfunc(char  *msg)
{
	fprintf(stderr, "%s", msg);
}

static logfunc __debuglogfunc   = default_debuglogfunc;
static logfunc __warninglogfunc = default_warninglogfunc;
static logfunc __errorlogfunc   = default_errorlogfunc;

DLLEXPORT void set_debug_logger(logfunc func)
{
	__debuglogfunc = func;
}

DLLEXPORT void set_warning_logger(logfunc func)
{
	__warninglogfunc = func;
}

DLLEXPORT void set_error_logger(logfunc func)
{
	__errorlogfunc = func;
}

#define _MSGBUFSIZ	8192
#define _DEBUGPREFIX	"debug: "
#define _DEBUGPREFIXLEN 7
#define _WARNINGPREFIX	"warning: "
#define _WARNINGPREFIXLEN 9
#define _ERRORPREFIX	"error: "
#define _ERRORPREFIXLEN 7

DLLEXPORT void _warning_message(const char * format, ...)
{
	char buf[_MSGBUFSIZ+_WARNINGPREFIXLEN];
	memcpy(buf, _WARNINGPREFIX, _WARNINGPREFIXLEN);

	va_list args;
	va_start (args, format);

	if (isspace(*format))
		vsnprintf (buf, _MSGBUFSIZ+_WARNINGPREFIXLEN, format, args);
	else
		vsnprintf (buf+_WARNINGPREFIXLEN, _MSGBUFSIZ, format, args);
	va_end (args);

	buf[_MSGBUFSIZ+_WARNINGPREFIXLEN-1] = '\0';

	(*__debuglogfunc)(buf);
}

DLLEXPORT void _debug_message(const char * format, ...)
{
	char buf[_MSGBUFSIZ+_DEBUGPREFIXLEN];
	memcpy(buf, _DEBUGPREFIX, _DEBUGPREFIXLEN);

	va_list args;
	va_start (args, format);

	if (isspace(*format))
		vsnprintf (buf, _MSGBUFSIZ+_DEBUGPREFIXLEN, format, args);
	else
		vsnprintf (buf+_DEBUGPREFIXLEN, _MSGBUFSIZ, format, args);
	va_end (args);

	buf[_MSGBUFSIZ+_DEBUGPREFIXLEN-1] = '\0';

	(*__debuglogfunc)(buf);
}

DLLEXPORT void _error_message(const char * format, ...)
{
	char buf[_MSGBUFSIZ+_ERRORPREFIXLEN];
	memcpy(buf, _ERRORPREFIX, _ERRORPREFIXLEN);

	va_list args;
	va_start (args, format);

	if (isspace(*format))
		vsnprintf (buf, _MSGBUFSIZ+_ERRORPREFIXLEN, format, args);
	else
		vsnprintf (buf+_ERRORPREFIXLEN, _MSGBUFSIZ, format, args);
	va_end (args);

	buf[_MSGBUFSIZ+_ERRORPREFIXLEN-1] = '\0';

	(*__errorlogfunc)(buf);
}

/* -----------------------------------------------------------------------
 * build error message
 */

static char __error_message_buf[_MSGBUFSIZ];

char * build_error_message(const char * format, ...)
{
	char buf[_MSGBUFSIZ];
	va_list args;
	va_start (args, format);
	vsnprintf (buf, _MSGBUFSIZ, format, args);
	va_end (args);
	strncpy(__error_message_buf, buf, _MSGBUFSIZ);

	__error_message_buf[_MSGBUFSIZ-1] = '\0';

	return __error_message_buf;
}

char *append_error_message(const char * format, ...)
{
	char buf[_MSGBUFSIZ];
	va_list args;
	va_start (args, format);
	vsnprintf (buf, _MSGBUFSIZ, format, args);
	va_end (args);

	char tmp[_MSGBUFSIZ];
	snprintf(tmp, _MSGBUFSIZ, "%s\n.. %s", __error_message_buf, buf);

	strncpy(__error_message_buf, tmp, _MSGBUFSIZ);
	__error_message_buf[_MSGBUFSIZ-1] = '\0';

	return __error_message_buf;
}

void clear_error_message()
{
	__error_message_buf[0] = '\0';
}

DLLEXPORT char *get_error_message()
{
	return __error_message_buf;
}

} // namespace nccsd -----------------------------------------------------
