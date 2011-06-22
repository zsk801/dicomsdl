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
#include "utilfunc.h"


typedef struct {
	int optid;
	const char *optstr;
	bool needarg;
} t_cmdopt;

/*! Option parser
 *
 * Accepted forms are
 * <code>
 *  -o, --option,
 *  -o [arg], --option=[arg]
 * </code>
 */

class optparser {
	std::map<const char *, const char *> flag_opts;
	std::map<const char *, const char *> arg_opts;
	std::map<const char *, bool> flag_values;
	std::map<const char *, const char *> arg_values;

public:
	/// add an option that comes without argument
	void add_flagopt(const char *opt_str, const char* attr);

	/// add an option that comes with argument
	void add_argopt(const char *opt_str, const char* attr);



	bool get_optflag(const char* attr, bool defaultvalue=false);
	const char* get_optarg(const char* attr, const char* defaultvalue=NULL);

};
// opt
// argopt
// flagopt

class cmdopt {
public:
	cmdopt(const t_cmdopt *c)
	{
		for (; c->optstr; c++) {
			printf("LOAD %d, %s, %d\n", c->optid, c->optstr, c->needarg);
		}
	}
};

int main(int argc, char **argv)
{
/*	optparser opt;

	opt.add_flagopt("v", "verbose", "verbose");
	opt.add_flagopt("q", "quiet", "quiet");
	opt.add_flagopt("e", "extract", "extract");
	opt.add_argopt("k", "keyword", "keyword");
*/

	if (argc < 2) {
		puts("dicomfiledump input.dcm");
		return 0;
	}

	dicom::dicomfile *df;
	df = dicom::open_dicomfile(argv[1]);

	if (df) {
		puts(df->dump_string().c_str());
		delete df;
	} else {
		printf("%s\n", dicom::get_error_message());
	}

	return 0;
}
