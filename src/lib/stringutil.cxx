/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include "utilfunc.h"
#include <ctype.h>

namespace dicom { //------------------------------------------------------

char *split_string(const char *start, const char *sep, char **next)
{
	char *p = (start? (char *)start: *next);

	while (*p && strchr(sep, *p)) p++;

	if (!*p)
		return NULL;

	*next = p + strcspn(p, sep);
	return p;
}

stringlist split_string(const char *s, const char *sep)
{
	stringlist li;
	char *p, *q;
	for (p = split_string(s, sep, &q); p; p = split_string(NULL, sep, &q))
		li.push_back(std::string(p, q-p));
	return li;
}

std::string join_string(stringlist li, const char *sep)
{
	size_t len = 0;
	stringlist::iterator it;
	std::string ret;

	if (li.size() < 1)
		return ret;

	for (it = li.begin(); it != li.end(); it++)
		len += (*it).size();
	len += strlen(sep)*(li.size()-1);

	ret.reserve(len);

	it = li.begin();
	ret.append(*it);

	for (; it != li.end(); it++) {
		ret.append(sep);
		ret.append(*it);
	}

	return ret;
}

char* rtrim(char* s)
{
	char *q;
	q = s + strlen(s) - 1;

	while (q >= s && isspace(*q)) q--;
	q++;
	*q = '\0';
	return s;
}

char* ltrim(char *s)
{
	char *p = s, *q;
	while (*p && isspace(*p)) p++;

	if (!*p) {
		*s = '\0';
		return s;
	}
	q = p + strlen(p);
	memmove(s, p, q-p+1);
	return s;
}

char* trim(char *s)
{
	char *p = s, *q;
	while (*p && isspace(*p)) p++;

	if (!*p) {
		*s = '\0';
		return s;
	}

	q = p + strlen(p) - 1;

	while (q >= p && isspace(*q)) q--;
	q++;

	memmove(s, p, q-p);
	s[q-p] = '\0';
	return s;
}

inline const char* trim_right(std::string &s)
{
	s.erase(s.find_last_not_of(", \t\n\r\v\f")+1);
	return s.c_str();
}

inline const char* trim_left(std::string &s)
{
	s.erase(0, s.find_first_not_of(", \t\n\r\v\f"));
	return s.c_str();
}

inline const char* trim(std::string &s)
{
	s.erase(s.find_last_not_of(", \t\n\r\v\f")+1);
	s.erase(0, s.find_first_not_of(", \t\n\r\v\f"));
	return s.c_str();
}

} // namespace dicom -----------------------------------------------------
