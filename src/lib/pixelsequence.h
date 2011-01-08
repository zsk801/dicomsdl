/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#ifndef __PIXELSEQUENCE_H__
#define __PIXELSEQUENCE_H__

#include <string.h>
#include <list>
#include <iomanip>

#include "instream.h"
#include "outstream.h"

namespace dicom { //------------------------------------------------------

struct pixelfragment {
	bool have_ffd9;	// true if this is the last fragment of a frame
					// e.g. FF D9 in the tail of ptr
	uint8 *ptr;
	uint32 len;
	bool own_memory;

	pixelfragment(instream *in, uint32 len, bool copydata = false);
	pixelfragment(uint8 *data, uint32 datasize, bool copydata = false);
	~pixelfragment();

	void check_have_ffd9();
};

struct pixelsequence {
	std::list <pixelfragment *> fragments;

	pixelsequence() {};
	~pixelsequence();

	int load(instream *in, bool copydata=false);
	void dump(std::iostream *os, const char *prefix);

	int number_of_frames() {
		int nframes = 0;
		std::list<pixelfragment *>::iterator it;
		for (it=fragments.begin(); it!=fragments.end(); it++)
			if ((*it)->have_ffd9) nframes ++;
		return nframes;
	};

	int number_of_fragments() {
		return fragments.size();
	};

	void get_frame_data_a
		(int frameidx, char **buf, int *bufsiz, int *isalloc);

	int add_frame_data
		(char *data, int datasize, int copydata=0, int fragmentsize=0);

	void _save(outstream* os, opttype opt);
};

} // namespace dicom -----------------------------------------------------

#endif //  __PIXELSEQUENCE_H__

