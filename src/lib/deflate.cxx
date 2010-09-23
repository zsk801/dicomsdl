/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include "dicom.h"
#include "errormsg.h"
#include <sstream>
#include "zlib/zlib.h"
#include "deflate.h"

namespace dicom { //------------------------------------------------------

#define CHUNK 0x10000

void inflate_dicomfile_a
	(char *data, int datasize, int offset, char **val_a, int *len_a)
{
	std::stringstream ss;

	char *val = NULL;	*val_a = val;
	int len = 0;		*len_a = len;

	ss.write(data, offset);
	data += offset;
    datasize -= offset;

	// codes from zpipe.c ------------------------------------------------

    int ret;
    unsigned have;
    z_stream strm;
    unsigned char out[CHUNK];

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm, -15);
    if (ret != Z_OK)
        return; /// ret

	strm.avail_in = datasize;
	strm.next_in = (unsigned char *)data;

	do {
		strm.avail_out = CHUNK;
		strm.next_out = out;
		ret = inflate(&strm, Z_NO_FLUSH);
		//assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
		switch (ret) {
		case Z_NEED_DICT:
			ret = Z_DATA_ERROR;     /* and fall through */
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			(void)inflateEnd(&strm);
			return; /// ret;
		}
		have = CHUNK - strm.avail_out;
		ss.write((const char *)out, have);
	} while (strm.avail_out == 0);

    (void)inflateEnd(&strm);
    //return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;

	// build return value -----------------------

	len = (int)ss.tellp();
	val = (char *)malloc(len);
	ss.read(val, len);
	*val_a = val;
	*len_a = len;
}

void deflate_dicomfile_a
	(char *data, int datasize, int offset, int level, char **val_a, int *len_a)
{
	std::stringstream ss;

	char *val = NULL;	*val_a = val;
	int len = 0;		*len_a = len;

	ss.write(data, offset);
	data += offset;
	datasize -= offset;

	// codes from zpipe.c ------------------------------------------------

    int ret, flush;
    unsigned have;
    z_stream strm;
    //unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit2
		(&strm, level, Z_DEFLATED, -15, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK)
        return; // ret;

    strm.avail_in = datasize;
	flush = Z_FINISH;
	strm.next_in = (unsigned char *)data;

	/* run deflate() on input until output buffer not full, finish
	   compression if all of source has been read in */
	do {
		strm.avail_out = CHUNK;
		strm.next_out = out;
		ret = deflate(&strm, flush);    /* no bad return value */
		//assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
		have = CHUNK - strm.avail_out;
		ss.write((const char *)out, have);
	} while (strm.avail_out == 0);
	//assert(strm.avail_in == 0);     /* all input will be used */

	/* done when last data in file processed */
    //assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    //return Z_OK;

	// build return value -----------------------

	len = (int)ss.tellp();
	val = (char *)malloc(len);
	ss.read(val, len);
	*val_a = val;
	*len_a = len;
}

} // namespace dicom -----------------------------------------------------
