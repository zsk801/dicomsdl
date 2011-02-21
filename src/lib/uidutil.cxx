/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "dicom.h"
#include "errormsg.h"

namespace dicom { //------------------------------------------------------

// defines and static variables ------------------------------------------

static struct __uidcount__ {
	long long count;
	__uidcount__() {
		count = 1;
	}

	long long get() {
		return count++;
	}

} _uidcount;

static struct __uidprefix__ {
	char prefix[80];

	__uidprefix__() {
		set();
	}

	void set(const char *u=NULL)
	{
		if (u == NULL)
			set(DICOMSDL_UIDPREFIX);
		else if (*u == '\0' || !is_valid_uid(prefix)) {
			LOG_WARNING_MESSAGE("in set_uid_prefix(..) :"
					"in valid string '%s' for uid"
					"use default uid prefix '%s'", u, DICOMSDL_UIDPREFIX);
			set(DICOMSDL_UIDPREFIX);
		}
		else
		{
			strcpy(prefix, u);
			char *c = prefix + strlen(prefix) - 1;
			if (*c == '.')
				*c = '\0';
		}
	}

	char* get() {
		return prefix;
	}
} _uidprefix;


long long get_time_since_2010()
{
	time_t rawtime;
	time ( &rawtime );
	return (long long)rawtime - 1262271600L; // seconds from 2010.1.1.
}

void strip_long_uid(char *u)
{
	if (!u) return;

	int i, l = strlen(u);
	if (l <= MAX_UID_LEN)
		return;

	i = l-MAX_UID_LEN;
	if (u[i] == '.') i++;

	char *p=u+i, *q=u;
	while (*p) *q++ = *p++;
	*q = '\0';
}

DLLEXPORT int is_valid_uid(char *u)
{
	if (!u)
		return false;
	if (strlen(u) > MAX_UID_LEN)
		return false;
	while (*u) {
		if (!isdigit(*u) || *u != '.')
			return false;
		u++;
	}
	return true;
}

DLLEXPORT void set_uid_prefix(char *u)
{
	_uidprefix.set(u);
}

DLLEXPORT char *get_uid_prefix()
{
	return _uidprefix.get();
}

DLLEXPORT std::string gen_uid(char* base_uid)
{

	char buf[256];

	if (!base_uid)
		base_uid = _uidprefix.get();

	snprintf(buf, 256, "%s.%lld.%lld",
			base_uid, get_time_since_2010(), _uidcount.get());
	strip_long_uid(buf);

	return std::string(buf);
}

} // namespace dicom -----------------------------------------------------
