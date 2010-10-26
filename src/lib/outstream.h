/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#ifndef __OUTSTREAM_H__
#define __OUTSTREAM_H__

#include <ostream>
#include <map>
#include <list>
#include <stdio.h>
#include "dicom.h"
#include "memutil.h"

namespace dicom { //------------------------------------------------------

struct objofs_maker_struct {
	long objid;		// (long)(pointer to object)
	long objofs;	// offset of the object within the file

	long val_pos;	// position where 'offset value' will be stored
	int val_endian;
	int val_siz;	// 1 or 2 or 4
};

struct length_value_struct {
	int endian;
	int val_siz;	// 1 or 2 or 4
	long val_pos;	// position where 'length value' will be stored
	long start_pos, end_pos;

	length_value_struct() { val_pos = start_pos = end_pos = -1; };
};

class outstream {
	std::ostream* os;

	std::list <objofs_maker_struct> objofs_marker_dict;
	std::map <long, length_value_struct> length_values;

public:
	outstream(); // attach to memory
	outstream(char *filename);	// attach to file
	~outstream();

	void write(uint8 *data, uint32 datasize);

	void to_string_a(char **val_a, int *len_a);

	void write32u_le(uint32 c)
		{	uint8 p[4]; ptr_as_uint32(p) = c;
			if (MACHINE_ENDIANNESS == BIG_ENDIAN) swap32(p);
			write(p, 4); };
	void write16u_le(uint16 c)
		{	uint8 p[2]; ptr_as_uint16(p) = c;
			if (MACHINE_ENDIANNESS == BIG_ENDIAN) swap16(p);
			write(p, 2); };

	void write32u_be(uint32 c)
		{	uint8 p[4]; ptr_as_uint32(p) = c;
			if (MACHINE_ENDIANNESS == LITTLE_ENDIAN) swap32(p);
			write(p, 4); };
	void write16u_be(uint16 c)
		{	uint8 p[2]; ptr_as_uint16(p) = c;
			if (MACHINE_ENDIANNESS == LITTLE_ENDIAN) swap16(p);
			write(p, 2); };

	void write32u(uint32 c, int endian)
		{ endian == LITTLE_ENDIAN? write32u_le(c) : write32u_be(c);};
	void write16u(uint16 c, int endian)
		{ endian == LITTLE_ENDIAN? write16u_le(c) : write16u_be(c);};

	void write8u(uint8 c)
		{ write(&c, 1); };

	void write(uint32 c, int size_of_value, int endian)
	{
		if (size_of_value == 4)
			write32u(c, endian);
		else if (size_of_value == 2)
			write16u(c, endian);
		else
			write8u(c);
	}

	long tellp() { return os->tellp(); };

	//--------------------------------------------------------------------
	//   +-- reserve_bytes_for_offset_value(objid)
	//   |
	//   |                      +-- mark_offset(objid)
	//   |                      |
	//   [offset value] ::::::: [data segment .... ] ::::: ....

	void reserve_bytes_for_offset_value
		(long objid, int val_siz, int endian, int offsetdelta=0);
	void mark_offset(long objid);

	void reserve_32u_be_for_offset_value(long id)
		{ reserve_bytes_for_offset_value(id, 4, BIG_ENDIAN); };
	void reserve_32u_le_for_offset_value(long id)
		{ reserve_bytes_for_offset_value(id, 4, LITTLE_ENDIAN); };

	//   +-- reserve_bytes_for_length_value(id)
	//   |
	//   |                      +-- mark_start_pos(id)
	//   |                      |
	//   |                      |                +-- mark_end_pos(id)
	//   |                      |                |
	//   [length value] ::::::: [data segment .. ] ::::: ....

	void reserve_bytes_for_length_value
		(long id, int val_siz, int endian, int offsetdelta=0);
	void mark_start_pos(long id);
	void mark_end_pos(long id);

	void reserve_8u_for_length_value(long id)
		{ reserve_bytes_for_length_value(id, 1, LITTLE_ENDIAN); };
	void reserve_16u_be_for_length_value(long id)
		{ reserve_bytes_for_length_value(id, 2, BIG_ENDIAN); };
	void reserve_16u_le_for_length_value(long id)
		{ reserve_bytes_for_length_value(id, 2, LITTLE_ENDIAN); };
	void reserve_32u_be_for_length_value(long id)
		{ reserve_bytes_for_length_value(id, 4, BIG_ENDIAN); };
	void reserve_32u_le_for_length_value(long id)
		{ reserve_bytes_for_length_value(id, 4, LITTLE_ENDIAN); };

};

} // namespace nccsd ------------------------------------------------------

#endif // __OUTSTREAM_H__
