/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#ifndef __INSTREAM_H__
#define __INSTREAM_H__

#include "dicom.h"
#include "instreamobj.h"

namespace dicom { //------------------------------------------------------

#define NBYTES_TO_READ	(65536*2) // 128k block

class instream {
	uint8 *data;	// contains file image
	uint8 *p;		// current pointer
	uint8 *q;		// = data + filesize
	uint8 *_q;		// = p + ftell(fp)

	instreamobj* in;

	bool own_memory;	// true if data needs to be freed.
	instream *basestream;

public:
	// instream from a file
	instream(const char *filename);
	// instream from memory
	instream(uint8 *data, size_t datasize, bool copydata=false);
	// instream from 'base' instream
	instream(instream* basestream, int subsize);

	~instream();

	// read n bytes from stream
	// WARNING: do not free returned value from read(),
	//	        because read() don't 'new' memory.
	// read() functions may throw *char on error.
	uint8* read(int n);

	inline void rewind()
			{	p = data;  };
	inline void unread(int n)
			{	p = (p-n > data ? p-n : data);	};
	inline void skip(int n)
			{	read(n);	};
	bool iseof()
			{	return (p >= q);	};

	uint8 *dataptr() { return data; };
	uint8 *currptr() { return p; };
	uint32 curr_offset() { return (uint32)(p - basestream->data); };

	inline uint32 read32u_le()
		{ uint8*p = read(4); return le_get_uint32(p); };
	inline uint16 read16u_le()
		{ uint8*p = read(2); return le_get_uint16(p); };
	inline uint32 read32u_be()
		{ uint8*p = read(4); return be_get_uint32(p); };
	inline uint16 read16u_be()
		{ uint8*p = read(2); return be_get_uint16(p); };
	inline uint8  read8u()
		{ return ptr_as_uint8(read(1));	};

	inline uint32 read32u(int endian)
		{ return endian == LITTLE_ENDIAN? read32u_le() : read32u_be(); };
	inline uint16 read16u(int endian)
		{ return endian == LITTLE_ENDIAN? read16u_le() : read16u_be(); };

	inline uint32 read32_le()
		{ uint8*p = read(4); return le_get_int32(p); };
	inline uint16 read16_le()
		{ uint8*p = read(2); return le_get_int16(p); };
	inline uint32 read32_be()
		{ uint8*p = read(4); return be_get_int32(p); };
	inline uint16 read16_be()
		{ uint8*p = read(2); return be_get_int16(p); };
	inline int8	 read8()
		{ return ptr_as_int8(read(1));	};

	inline int32 read32(int endian)
		{ return endian == LITTLE_ENDIAN? read32_le() : read32_be(); };
	inline int16 read16(int endian)
		{ return endian == LITTLE_ENDIAN? read16_le() : read16_be(); };

	// inflate compressed data portion after offset bytes.
	// +---------------------+-----------------------------
	// | non-compressed data | compressed data ...
	// +---------------------+-----------------------------
	// <------ offset ------->
	void inflate(int offset);

	inline char* get_filename()
		{ return in->get_filename(); };

	// trim unfilled data buffer
	long free_unread_data();
};

} // namespace dicom ------------------------------------------------------

#endif // __INSTREAM_H__
