/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include <string.h>
#include <fstream>
#include <sstream>

#include "dicom.h"
#include "outstream.h"
#include "errormsg.h"

namespace dicom { //------------------------------------------------------


outstream::outstream(char *_filename)
{
	std::ofstream *ofs = new std::ofstream;
	ofs->open(_filename, std::ios::binary);
	if (!ofs->is_open()) {
		build_error_message("in outstream::outstream(): "
			"cannot write to '%s'", _filename);
		delete ofs;
		throw DICOM_WRITE_ERROR;
	}
	os = ofs;
}

outstream::outstream() // to memory stream
{
	std::stringstream *ss = new std::stringstream;
	os = ss;
}

outstream::~outstream()
{
	if (dynamic_cast<std::ofstream *>(os)) {
		std::ofstream *ofs = dynamic_cast<std::ofstream *>(os);
		ofs->close();
		delete ofs;
	} else { // string stream
		std::stringstream *ss = dynamic_cast<std::stringstream *>(os);
		delete ss;
	}
}

void outstream::write(uint8 *data, uint32 datasize)
{
	os->write((char *)data, datasize);
	if (os->bad()) {
		build_error_message(
			"in outstream::write(): "
			"cannot write %d byte(s) into stream", datasize);
		throw DICOM_WRITE_ERROR;
	}
}

void outstream::to_string_a(char **val_a, int *len_a)
{
	*val_a = NULL; *len_a = 0;
	if (dynamic_cast<std::stringstream *>(os)) {
		std::stringstream *ss = dynamic_cast<std::stringstream *>(os);
		*len_a = ss->tellp();
		*val_a = (char *)malloc(*len_a+1);

		if (*val_a)
			ss->read(*val_a, *len_a);
		else {
			build_error_message(
				"in outstream::to_string_a(): "
				"cannot allocate %d bytes", *len_a);
			*len_a = 0;
			throw DICOM_MEMORY_ERROR;
		}
	} else {
		build_error_message(
			"in outstream::to_string_a(): "
			"cannot convert a file object");
		throw DICOM_INTERNAL_ERROR;
	}
}

//------------------------------------------------------------------------

void outstream::reserve_bytes_for_offset_value
			(long objid, int val_siz, int endian, int offsetdelta)
{
	std::list<objofs_maker_struct>::iterator it;

	for (it = objofs_marker_dict.begin();
		 it != objofs_marker_dict.end(); it++)
		if (it->objid == objid && it->objofs != -1) {
			long t;
			if (offsetdelta) { t = tellp(); os->seekp(t+offsetdelta); }
			write(it->objofs, val_siz, endian);
			if (offsetdelta) os->seekp(t);
			return;
		}

	{
		objofs_maker_struct om;
		om.objid = objid;
		om.objofs = -1;
		om.val_pos = tellp() + offsetdelta;
		om.val_endian = endian;
		om.val_siz = val_siz;
		objofs_marker_dict.push_back(om);
	}

	if (offsetdelta == 0) {
		uint32 dummy = 0;
		write((uint8 *)&dummy, val_siz);
	}
}

void outstream::mark_offset(long objid)
{
	std::list<objofs_maker_struct>::iterator it;
	long t;

	for (it = objofs_marker_dict.begin();
		 it != objofs_marker_dict.end(); it++)
		if (it->objid == objid) {
			t = it->objofs = tellp();

			os->seekp(it->val_pos);
			write(it->objofs, it->val_siz, it->val_endian);
			os->seekp(t);
		}

	if (it == objofs_marker_dict.end()) {
		objofs_maker_struct om;
		om.objid = objid;
		om.objofs = tellp();
		om.val_pos = -1;
		objofs_marker_dict.push_back(om);
	}

	LOG_DEBUG_MESSAGE("outstream{%p}::mark_offset(%x) at %08xH\n",
			this, objid, tellp());
};

void outstream::reserve_bytes_for_length_value
				(long id, int val_siz, int endian, int offsetdelta)
{
	length_value_struct lv;
	lv = length_values[id];

	if (lv.end_pos != -1) {
		long t;
		if (offsetdelta) { t = tellp(); os->seekp(t + offsetdelta); }
		write(lv.end_pos - lv.start_pos, val_siz, endian);
		if (offsetdelta) os->seekp(t);
		length_values.erase(id);
	} else {
		lv.val_siz = val_siz;
		lv.endian = endian;
		lv.val_pos = tellp()+offsetdelta;

		length_values[id] = lv;

		if (offsetdelta == 0) {
			uint32 dummy = 0;
			write((uint8 *)&dummy, val_siz);
		}
	}
}

void outstream::mark_start_pos(long id)
{
	length_value_struct lv;
	lv = length_values[id];

	lv.start_pos = tellp();
	length_values[id] = lv;
}

void outstream::mark_end_pos(long id)
{
	length_value_struct lv;
	lv = length_values[id];

	if (lv.val_pos != -1) {
		long backup_pos = tellp();
		os->seekp(lv.val_pos);
		write(backup_pos-lv.start_pos,
			  lv.val_siz, lv.endian);
		os->seekp(backup_pos);
		length_values.erase(id);
	} else {
		lv.end_pos = tellp();
		length_values[id] = lv;
	}
}


} // namespace nccsd ------------------------------------------------------
