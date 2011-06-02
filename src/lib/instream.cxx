/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include <string.h>

#include "dicom.h"
#include "instream.h"
#include "instreamobj.h"
#include "errormsg.h"
#include "deflate.h"

namespace dicom { //------------------------------------------------------

// ctor ------------------------------------------------------------------

instream::instream(const char *_filename)
{
	data = NULL;
	basestream = NULL;

	// check _filename contains .zip -------------------------
	std::string zipfn, fn_in_zipfn;
	if (strlen(_filename) > 8 && memcmp(_filename, "zip:///", 7) == 0) {
		char *_zipfn = (char *)_filename + 7;
		char *_fn = strrchr(_zipfn, ':');
		if (_fn) _fn++;

		if (_fn && _fn > _zipfn) {
			zipfn = std::string(_zipfn, _fn-_zipfn-1);
			fn_in_zipfn = std::string(_fn);
		}
	}

	// open file ---------------------------------------------
	long filesize = 0;
	in = NULL;

	if (zipfn.size()) {
		// _filename is located in zip file
		zipfileinstreamobj* zipfin;
		zipfin = new zipfileinstreamobj(zipfn, fn_in_zipfn);
		if (zipfin->is_valid()) {
			filesize = zipfin->get_filesize();
			in = zipfin;
		} else
			delete zipfin;
	} else {
		fileinstreamobj* fin;
		fin = new fileinstreamobj(_filename);
		if (fin->is_valid()) {
			filesize = fin->get_filesize();
			in = fin;
		} else
			delete fin;
	}

	if (in == NULL)
		goto L_FILEERROR; // error in opening file

	// attach stream -----------------------------------------

	data = (uint8 *)malloc(filesize);
	if (!data) {
		in->close();
		delete in;
		goto L_MEMERROR;
	}
	p = _q = data;
	q = p + filesize;

	basestream = this;
	own_memory = true;

	LOG_DEBUG_MESSAGE("instream{%p}::instream('%s') - "
			"new %d bytes\n", this, in->get_filename(), filesize);

	return; // OK

L_MEMERROR:
	build_error_message("in instream::instream(char *): "
			"cannot malloc %d bytes", filesize);
	throw DICOM_MEMORY_ERROR;
L_FILEERROR:
	build_error_message("in instream::instream(char *): "
			"cannot open '%s'", _filename);
	throw DICOM_FILE_ERROR;
}

instream::instream(uint8 *_data, size_t datasize, bool copydata)
{
	data = NULL;
	basestream = NULL;
	in = NULL;

	if (_data && datasize) {
		if (copydata) {
			data = (uint8 *)malloc(datasize);
			if (!data) goto L_MEMERROR;
			memcpy(data, _data, datasize);
			p = data;
			own_memory = true;
		} else {
			data = p = _data;
			own_memory = false;
		}
		q = _q = p + datasize;
	} else {
		data = p = q = _q = NULL;
		own_memory = false;
	}

	basestream = this;

	if (copydata)
		LOG_DEBUG_MESSAGE("instream{%p}::instream(), "
				"new %d bytes\n", this, datasize);
	else
		LOG_DEBUG_MESSAGE("instream{%p}::instream()\n", this);

	return;

L_MEMERROR:
	build_error_message(
			"in instream::instream(char*, size_t, bool): "
			"cannot malloc %d bytes", datasize);
	throw DICOM_MEMORY_ERROR;
}

instream::instream(instream* _basestream, int subsize)
{
	data = NULL;
	in = NULL; 			// not use in substream

	if (!_basestream) {
		build_error_message(
				"in instream::instream(instream*, int): "
				"basestream is null");
		throw DICOM_INTERNAL_ERROR;
	}

	basestream = _basestream->basestream;

	data = _basestream->data;
	p = _basestream->p;

	if (subsize >= 0)
        q = p+subsize;
	else // in case of -1 i.e. unknown length
		q = _basestream->q;
	if (q >= _basestream->q)
		q = _basestream->q;

	_q = q;				// not use in substream
	own_memory = false;

	LOG_DEBUG_MESSAGE(
			"instream{%p}::instream("
			"base=instream{%p}, base_base=instream{%p})\n",
			this, _basestream, _basestream->basestream);
}


// dtor ------------------------------------------------------------------

instream::~instream()
{

	if (in)
		LOG_DEBUG_MESSAGE("instream{%p}::~instream('%s')",this,in->get_filename());
	else
		LOG_DEBUG_MESSAGE("instream{%p}::~instream()",this);

	if (own_memory)
		LOG_DEBUG_MESSAGE(" - free bytes\n");
	else
		LOG_DEBUG_MESSAGE(" \n");

	if (in) {
		in->close();
		delete in;
	}

	if (own_memory) {
		free(data);
	}
}


// read from instream ----------------------------------------------------

uint8 *instream::read(int n)
{
	if (n < 0) {
		build_error_message(
			"in instream::read(int):"
			" cannot read negative bytes (%d) at offset 0x%06x",
			n, curr_offset());
		throw DICOM_INSTREAM_ERROR;
	}

	if (p + n <= q) {
		uint8 *value = p;
		p += n;
		while (basestream->_q < p) {
			long nread = basestream->in->read(basestream->_q,
						basestream->q - basestream->_q >= NBYTES_TO_READ?
						NBYTES_TO_READ:
						basestream->q - basestream->_q
					);
			basestream->_q += nread;

			LOG_DEBUG_MESSAGE(
					"instream{%p}::read() +%d bytes from disk "
					"(%d bytes remaining)\n",
					this, nread, basestream->q - basestream->_q);

			if (nread == 0) {
				build_error_message("cannot read file; "
						"read 0 bytes from file");
				throw DICOM_FILE_ERROR;
			}
			if (basestream->_q == basestream->q) {

				LOG_DEBUG_MESSAGE(
						"instream{%p}::read() - fclose('%s')\n",
						this, basestream->in->get_filename());

				basestream->in->close();
				delete basestream->in;
				basestream->in = NULL;
			}
		}

		return value;
	}
	else {
		build_error_message(
			"in instream::read(int):"
			" cannot read %d bytes from file (or memory) offset 0x%06x",
			n, curr_offset());
		throw DICOM_INSTREAM_ERROR;
	}

}


// inflate data in the instream ------------------------------------------

void instream::inflate(int offset)
{
	rewind();
	skip(q-data); // this loads entire file image into memory
	rewind();

	char *inf_data;
	int inf_size;
	inflate_dicomfile_a(
			(char*)data, q-data, offset, &inf_data, &inf_size);
	if (!inf_data) {
		build_error_message(
			"in instream::inflate(int):"
			" cannot decompress the deflated file image");
		throw DICOM_INFLATE_ERROR;
	}

	if (in) {
		in->close();
		delete in; in = NULL;
	}
	if (own_memory) free(data); data = NULL;

	p = data = (uint8 *)inf_data;
	q = _q = p + inf_size;
	own_memory = true;

	LOG_DEBUG_MESSAGE("instream{%p}::inflate(%d)\n",
			this, offset);
}

// trim unfilled data buffer ---------------------------------------------

long instream::free_unread_data() {
	if (!own_memory)
		return 0;

	if (this != basestream)
		return 0;

	if (p == q)
		return 0; // no need to trim memory

	long re_datasize = p - data;
	uint8* re_data = (uint8 *)realloc(data, re_datasize);

	long ptr_diff = re_data - data;

	data += ptr_diff;
	p += ptr_diff;
	_q = q = data + re_datasize;

	if (in) {
		in->close();
		delete in; in = NULL;
	}

	return ptr_diff;
}

} // namespace dicom ------------------------------------------------------
