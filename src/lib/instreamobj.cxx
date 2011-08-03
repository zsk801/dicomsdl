/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include "dicom.h"
#include "minizip/unzip.h"
#include "instreamobj.h"
#include <string.h>

namespace dicom { //------------------------------------------------------

// reading data from file ------------------------------------------------

fileinstreamobj::fileinstreamobj(const char *_fn)
{
	filesize = 0;
	fn = NULL;

	fp = fopen(_fn, "rb");
	if (fp) {
		fseek(fp, 0L, SEEK_END);
		filesize = ftell(fp);
		fseek(fp, 0L, SEEK_SET);

		fn = (char *)malloc(strlen(_fn)+1);
		strcpy(fn, _fn);
	}
};

bool fileinstreamobj::is_valid()
{
	return (fp != NULL);
};

void fileinstreamobj::close() {
	if (fp) fclose(fp);
	fp = NULL;

	if (fn) free(fn);
	fn = NULL;
};

long fileinstreamobj::read(uint8 *dst, long bytes_to_read)
{
	if (fp)
		return fread(dst, 1, bytes_to_read, fp);
	else
		return 0;
}

// reading data from zipfile ----------------------------------------------

zipfileinstreamobj::zipfileinstreamobj(std::string _zipfn, std::string _fn)
{
	longfn = "zip:///"+_zipfn +":"+ _fn;
	zipfn = _zipfn;
	fn = _fn;

	unz_file_info64 finfo;
	char fn_inzip[256];

	uf = unzOpen64(_zipfn.c_str());
	if (!uf) goto L_EXTRACT_ERROR;

    if (unzLocateFile(uf, _fn.c_str(), 0) != UNZ_OK)
    	goto L_EXTRACT_ERROR;

    if (unzGetCurrentFileInfo64(
    		uf, &finfo, fn_inzip,sizeof(fn_inzip),NULL,0,NULL,0)
    	!= UNZ_OK)
    	goto L_EXTRACT_ERROR;

    filesize = (long)(finfo.uncompressed_size);

    if (unzOpenCurrentFilePassword(uf, NULL) != UNZ_OK)
    	goto L_EXTRACT_ERROR;

    return;

L_EXTRACT_ERROR:
	if (uf) unzClose(uf);
	uf = NULL;
}

bool zipfileinstreamobj::is_valid()
{
	return (uf != NULL);
}

void zipfileinstreamobj::close()
{
	if (uf) {
	    unzCloseCurrentFile(uf);
		unzClose(uf);
	}
	uf = NULL;
}

long zipfileinstreamobj::read(uint8 *dst, long bytes_to_read)
{
	int bytes_read = unzReadCurrentFile(uf, dst, bytes_to_read);
	if (bytes_read != bytes_to_read) {
		close();
		return bytes_read;
	}

	return bytes_read;
}

} // namespace dicom ------------------------------------------------------
