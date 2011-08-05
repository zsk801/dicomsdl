/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#ifndef __UTILFUNC_H__
#define __UTILFUNC_H__

#include "dicomcfg.h"
#include <list>
#include <map>
#include <string>
#include <stdio.h>

#define CMDOPT_ON_STRING	"ON"

namespace dicom { //------------------------------------------------------


// Utility functions

typedef std::list<std::string> stringlist;


/*! split string; similar to strtok_r, but source memory is not changed.
 * \code
	const char *sep = ", \t\n\r\v\f";
	char *p, *q, buf[80];
	for (p = split_string(s, sep, &q); p; p = split_string(NULL, sep, &q)) {
		int sz = (q-p > 79? 79 : q-p);
		memcpy(buf, p, sz);
		buf[sz] = '\0';
		printf("[%s]\n", buf);
	}
	\endcode
 */
char *split_string(const char *start, const char *sep, char **next);

stringlist split_string(const char *s, const char *sep=" \t\n\r\v\f");
std::string join_string(stringlist li, const char *sep);

// string trimming functions
char* rtrim(char* s);
char* ltrim(char *s);
char* trim(char *s);
inline const char* trim_right(std::string &s);
inline const char* trim_left(std::string &s);
inline const char* trim(std::string &s);

/*! Find files using POSIX glob() function
 *
 * @param pattern file name pattern for POSIX glob() function
 * @return list of file name string
 */
stringlist find_files(const char *pattern);

class cmdopt {

	std::map<int, std::string> opts;
	stringlist args;
	int pos;

	std::map<std::string, int> dict;
	std::map<std::string, bool> dict0;

	char errmsg[256];

public:
	void adddict(int keyid, const char *key, bool needvalue);


	int load(int argc, char **argv);

	void dump()
	{
		for (std::map<int, std::string>::iterator it_o = opts.begin(); it_o != opts.end(); it_o++)
			printf("OPT '%d' = '%s'\n", it_o->first, it_o->second.c_str());

		for (stringlist::iterator it_a = args.begin(); it_a != args.end(); it_a++)
			printf("ARG '%s'\n", it_a->c_str());
	}

	bool is_avail(int keyid)
	{
		return (opts.find(keyid) != opts.end());
	}

	bool check_opt(int keyid)
	{
		if (is_avail(keyid)) {
			if (opts[keyid] == CMDOPT_ON_STRING)
				return true;
		}
		return false;
	}

	const char* get_opt_value(int keyid)
	{
		if (is_avail(keyid))
			return opts[keyid].c_str();
		else
			return NULL;
	}

	stringlist get_args()
	{
			return args;
	}

	const char* get_error()
	{
		return (const char *)errmsg;
	}
};

} // namespace dicom -----------------------------------------------------

#endif
