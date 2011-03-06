/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#ifndef __INSTREAMOBJ_H__
#define __INSTREAMOBJ_H__

#include "dicom.h"
#include "minizip/unzip.h"

namespace dicom { //------------------------------------------------------

class instreamobj {
public:
	long filesize;
	virtual bool is_valid() { return false; };
	long get_filesize() { return filesize; };
	virtual char* get_filename() { return NULL; };
	virtual void close() {};
	virtual long read(uint8 *dst, long bytes_to_read) { return -1; };
};

// reading data from file ------------------------------------------

class fileinstreamobj: public instreamobj {
	FILE *fp;
	char *fn;

public:
	fileinstreamobj(const char *_fn);
	~fileinstreamobj() { close(); };

	bool is_valid();
	char *get_filename() { return fn; };
	void close();
	long read(uint8 *dst, long bytes_to_read);
};

// reading data from zipfile ----------------------------------------

class zipfileinstreamobj: public instreamobj {
	unzFile uf;
	std::string longfn;
	std::string zipfn;
	std::string fn;
//	char *zipfn;
//	char *fn;

public:
	zipfileinstreamobj(std::string _zipfn, std::string _fn);
	~zipfileinstreamobj() { close(); };

	bool is_valid();
	char *get_filename() { return (char *)(longfn.c_str()); };
	void close();
	long read(uint8 *dst, long bytes_to_read);
};

} // namespace dicom ------------------------------------------------------

#endif // __INSTREAMOBJ_H__
