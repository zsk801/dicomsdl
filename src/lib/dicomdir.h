/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#ifndef __DICOMDIR_H__
#define __DICOMDIR_H__

#include <map>
#include <list>
#include <string>
#include "dicom.h"

namespace dicom { //------------------------------------------------------

struct __compare_string {
  bool operator() (const char *p, const char *q) const
  { return strcmp(p, q) < 0;  }
};

typedef std::list<char *> list_pchar_t;
typedef std::map<char *, list_pchar_t, __compare_string> map_pchar_t;


// ------
// define dataelement's tags to be saved into DICOMDIR
// for each directory record type

struct dicomdir_taglist_t {
	map_pchar_t taglist;

	// add dataelement's tags
	void add_tags(char **drtype_taglist);

	// get tag list for a directory record type
	list_pchar_t* get_tags(const char *drtype);

	void reset_tags();
	void free_tags();

	dicomdir_taglist_t()
		{ reset_tags(); }
	~dicomdir_taglist_t()
		{ free_tags(); };
};

} // namespace dicom -----------------------------------------------------

#endif // __DICOMDIR_H__
