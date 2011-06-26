/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include "utilfunc.h"

namespace dicom { //------------------------------------------------------

void cmdopt::adddict(int keyid, const char *key, bool needvalue)
{
	dict[key] = keyid;
	dict0[key] = needvalue;
}

int cmdopt::load(int argc, char **argv)
{
	std::string key, val;
	char *p;

	for (int i = 0; i < argc;) {
		p = argv[i];
		if (*p && p[0] == '-') {
			if (p[1] && p[1] == '-') {
				key = p + 2;
			} else {
				key = p + 1;
			}
			pos = key.find('=');
			if (pos > 0) {
				val = key.substr(pos+1);
				key = key.substr(0,pos);
				i++;

				if (dict.find(key) != dict.end()) {
					opts[dict[key]] = val;
				} else
					goto KEYERROR;
			} else {
				i++;
				if (dict.find(key) != dict.end()) {
					if (dict0[key]) {
						if (i == argc)
							goto VALUEERROR;
						opts[dict[key]] = argv[i++];
					}
					else
						opts[dict[key]] = CMDOPT_ON_STRING;
				} else
					goto KEYERROR;
			}
		} else {
			args.push_back(argv[i++]);
		}
	}
	*errmsg = '\0';
	return 0;
KEYERROR:
	snprintf(errmsg, 256, "unknown option '%s'", key.c_str());
	return -1;
VALUEERROR:
	snprintf(errmsg, 256, "missing an additional option for '%s'", key.c_str());
	return -2;
};


} // namespace dicom -----------------------------------------------------
