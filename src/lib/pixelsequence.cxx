/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include <list>

#include "pixelsequence.h"
#include "instream.h"
#include "errormsg.h"
#include <iostream>

namespace dicom { //------------------------------------------------------

pixelfragment::pixelfragment(instream *in, uint32 len, bool copydata)
{
	if (copydata) {
		ptr = (uint8 *)malloc(len);
		if (!ptr) {
			build_error_message(
				"in pixelfragment::pixelfragment(...):"
				" cannot allocate %d bytes", len);
			throw DICOM_MEMORY_ERROR;
		}

		this->len = len;
		memcpy(ptr, in->read(len), len);
		own_memory = true;
	} else {
		ptr = in->read(len);
		this->len = len;
		own_memory = false;
	}

	check_have_ffd9();

	LOG_DEBUG_MESSAGE("pixelfragment{%p}::pixelfragment("
			"instream{%p}, %d)\n", this, in, len);
}

pixelfragment::pixelfragment(uint8 *data, uint32 datasize, bool copydata)
{
	if (copydata) {
		ptr = (uint8 *)malloc(datasize);
		if (!ptr) {
			build_error_message(
				"in pixelfragment::pixelfragment(...):"
				" cannot allocate %d bytes", len);
			throw DICOM_MEMORY_ERROR;
		}

		this->len = datasize;
		memcpy(ptr, data, datasize);
		own_memory = true;
	} else {
		ptr = data;
		this->len = datasize;
		own_memory = false;
	}

	check_have_ffd9();

	LOG_DEBUG_MESSAGE("pixelfragment{%p}::pixelfragment("
			"{%p}, %d)\n", this, data, len);
}

pixelfragment::~pixelfragment()
{
	if (own_memory) free(ptr);

	LOG_DEBUG_MESSAGE("pixelfragment{%p}::~pixelfragment()", this);
	if (own_memory)
		LOG_DEBUG_MESSAGE(" free memory\n", this);
	else
		LOG_DEBUG_MESSAGE(" \n", this);
}

void pixelfragment::check_have_ffd9()
{
	// check 0xFFD9 ( End of Image - EOI - (jpeg) or
	// or End of Codestream - EOC - (jpeg2000) marker
	// at the end of fragment

	have_ffd9 = false;
	int c = (16 < len ? 16 : len);
	uint8 *p = ptr + len - 2;
	while (c--)
		if (p[0] == 0xff && p[1] == 0xd9) {
			have_ffd9 = true;
			break;
		} else
			p--;
}

pixelsequence::~pixelsequence()
{
	std::list<pixelfragment *>::iterator it;
	for (it=fragments.begin(); it!=fragments.end(); it++)
		delete (*it);

	LOG_DEBUG_MESSAGE("pixelsequence{%p}::~pixelsequence()\n", this);
}

int pixelsequence::load(instream *in, bool copydata)
{
	// initial scan

	uint16 gggg, eeee;
	uint32 len;

	try {
		gggg = in->read16u_le();	// should be 0xfffe
		eeee = in->read16u_le();	// should be 0xe000
		len = in->read32u_le();
		in->read(len); // skip first item (basic offset table)

		while (true) {
			gggg = in->read16u_le();
			eeee = in->read16u_le();
			len = in->read32u_le();

			if (gggg == 0xfffe && eeee == 0xe0dd) // Sequence Delimiter Item
				break;

			pixelfragment *frag = new pixelfragment (in, len, copydata);
			fragments.push_back(frag);
		}

		LOG_DEBUG_MESSAGE("pixelsequence{%p}::load()\n", this);
	} catch (errtype err) {
		append_error_message("in pixelsequence::load():");
		return err;
	}

	return DICOM_OK;
}

void pixelsequence::dump(std::iostream *os, const char *prefix)
{
	char buf[1024];
	std::list<pixelfragment *>::iterator it;
	int frameidx = 0, fragidx = 0;
	for (it=fragments.begin(); it!=fragments.end(); it++) {
		sprintf(buf, "Frame #%d, Fragment #%d, ( %d bytes%s%s )\n",
				frameidx, fragidx++, (*it)->len,
				((*it)->own_memory ? " *": ""),
				((*it)->have_ffd9 ? ", ends with 0xFFD9": "")
		);
		(*os) << prefix << buf;
		if ((*it)->have_ffd9) frameidx ++;
	}
}

void pixelsequence::get_frame_data_a
	(int frameidx, char **buf, int *bufsiz, int *isalloc)
{
	*buf = NULL;
	*bufsiz = 0;
	*isalloc = false;

	int idx = 0;
	std::list<pixelfragment *>::iterator it, it_start, it_end;
	for (it=fragments.begin(); it!=fragments.end(); it++) {
		if (frameidx == idx) {
			if (*bufsiz == 0)
				it_start = it;
			*bufsiz += (*it)->len;
			it_end = it;
		}
		if ((*it)->have_ffd9) idx ++;
		if (idx > frameidx) break;
	}

	if (*bufsiz) {
		if (it_start == it_end) { // one fragment in a frame
			*buf = (char *)(*it_start)->ptr;
			*isalloc = false;
		} else { // multiple fragment in a frame
			*buf = (char *)malloc(*bufsiz);
			char *p = *buf;
			it = it_start;
			while (true) {
				memcpy(p, (*it)->ptr, (*it)->len);
				p += (*it)->len;
				if (it == it_end) break;
				it++;
			}
			*isalloc = true;
		}
	}
}

int pixelsequence::add_frame_data
	(char *data, int datasize, int copydata, int fragmentsize)
{
	pixelfragment *frag;
	int bytestocopy;

	fragmentsize *= 1024; // fragment size in kilo bytes.
	if (fragmentsize == 0)
		fragmentsize = datasize;

	while (datasize) {
		bytestocopy = fragmentsize < datasize? fragmentsize: datasize;

		try {
			frag = new pixelfragment ((uint8 *)data, bytestocopy, copydata);
		} catch (errtype err) {
			return err; // DICOM_MEMORY_ERROR
		}

		data += bytestocopy;
		datasize -= bytestocopy;

		if (datasize == 0 && frag->have_ffd9 != true) {
			LOG_WARNING_MESSAGE(" in  pixelsequence::add_frame_data(...):"
					" the frame is not ended with FFD9!\n");
			frag->have_ffd9 = true;
		}

		fragments.push_back(frag);
	}

	if (number_of_frames() == 0) {
		// TODO: if there is no fragment ends with FFD9,
		// then use basic off set information to sort fragments into frame
		std::list<pixelfragment *>::iterator it;
		it = fragments.end(); it--;
		(*it)->have_ffd9 = true;
	}

	return DICOM_OK;
}

void pixelsequence::_save(outstream* os, opttype opt) {
	std::list<pixelfragment *>::iterator it;

	if (opt & OPT_SAVE_BASIC_OFFSET_TABLE) {
		// Table A.4-2 ELEMENTS WITH BASIC OFFSET TABLE
		// item tag(4) + item len(4) + item val(4) * number of fragments
		os->write16u_le(0xfffe);
		os->write16u_le(0xe000);
		os->write32u_le(4 * fragments.size());

		uint32 offset = 0;
		int is_new_frame = 1;
		for (it=fragments.begin(); it!=fragments.end(); it++) {
			if (is_new_frame) {
				os->write32u_le(offset);
				is_new_frame = 0;
			}
			offset += ((*it)->len + 4 + 4);
			if ((*it)->have_ffd9)
				is_new_frame = 1;
		}
	}
	else {
		// Table A.4-1 ELEMENTS WITHOUT BASIC OFFSET TABLE
		// Item Tag(4) + Item len(4)
		os->write16u_le(0xfffe);
		os->write16u_le(0xe000);
		os->write32u_le(0x00000000);
	}

	for (it=fragments.begin(); it!=fragments.end(); it++) {
		os->write16u_le(0xfffe);
		os->write16u_le(0xe000);
		os->write32u_le((*it)->len);
		os->write((*it)->ptr, (*it)->len);
	}

	os->write16u_le(0xfffe);
	os->write16u_le(0xe0dd);
	os->write32u_le(0x00000000);
}

} // namespace dicom -----------------------------------------------------
