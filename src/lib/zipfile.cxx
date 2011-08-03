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
#include "errormsg.h"
#include <string>
#include <sstream>

namespace dicom { //------------------------------------------------------

// extract a file from zipfile -------------------------------------------

DLLEXPORT void zipfile_extract_file_a(const char *zipfn, const char *fn,
		char **val_a, int *len_a)
{
	char *val=NULL; *val_a = val;
	int len=0;		*len_a = len;

	unzFile uf=NULL;
	int err=UNZ_OK;
    char fn_inzip[256];
    unz_file_info64 finfo;

	uf = unzOpen64(zipfn);
	if (!uf) {
		build_error_message("in zipfile_extract_file_a():"
			" error in opening '%s'", zipfn);
		goto L_EXTRACT_ERROR;
	}


    if (unzLocateFile(uf,fn,0)!=UNZ_OK)
    {
        build_error_message("in zipfile_extract_file_a():"
			" file '%s' not found in the zipfile '%s'", fn, zipfn);
        goto L_EXTRACT_ERROR;
    }

    err = unzGetCurrentFileInfo64(
    		uf,&finfo,fn_inzip,sizeof(fn_inzip),NULL,0,NULL,0);
    if (err!=UNZ_OK)
    {
    	build_error_message("in zipfile_extract_file_a(): "
			"error %d with zipfile in unzGetCurrentFileInfo",err);
    	goto L_EXTRACT_ERROR;
    }

    len = (int)(finfo.uncompressed_size);
    val = (char *)malloc(len);

    err = unzOpenCurrentFilePassword(uf,NULL);
    if (err!=UNZ_OK)
    {
    	build_error_message("in zipfile_extract_file_a(): "
        	"error %d with zipfile in unzOpenCurrentFilePassword\n",err);
    	goto L_EXTRACT_ERROR;
    }

    err = unzReadCurrentFile(uf,val,len);
    if (err != len)
    {
    	build_error_message("in zipfile_extract_file_a(): "
			"error %d with zipfile in unzReadCurrentFile",err);
    	unzCloseCurrentFile(uf);
    	goto L_EXTRACT_ERROR;
    }

    unzCloseCurrentFile(uf);
	unzClose(uf);

	*val_a = val;
	*len_a = len;

	build_error_message("");
	return;

L_EXTRACT_ERROR:
	if (uf)
		unzClose(uf);
	*val_a = NULL;
	*len_a = 0;
	return;
}

// get list from zipfile -------------------------------------------------

int __zipfile_get_list(unzFile uf, std::stringstream &ss)
{
    uLong i;
    unz_global_info64 gi;
    int err=UNZ_OK;

    err = unzGetGlobalInfo64(uf,&gi);
    for (i=0;i<gi.number_entry;i++) {
    	if (i) ss << "\n";

        char filename_inzip[256];
        unz_file_info64 file_info;

        err = unzGetCurrentFileInfo64(
        		uf,&file_info,filename_inzip,
        		sizeof(filename_inzip),NULL,0,NULL,0);
        if (err!=UNZ_OK) {
        	build_error_message("in __zipfile_get_list(): "
				"error %d with zipfile in unzGetCurrentFileInfo\n", err);
            break;
        }

        char lastc = filename_inzip[strlen(filename_inzip)-1];
        if (lastc != '/' && lastc != '\\')
        	ss << filename_inzip;

        if ((i+1)<gi.number_entry) {
            err = unzGoToNextFile(uf);
            if (err!=UNZ_OK) {
            	build_error_message("in __zipfile_get_list(): "
					"error %d with zipfile in unzGoToNextFile\n", err);
                break;
            }
        }
    }

    return err; // UNZ_OK or not
}

DLLEXPORT std::string zipfile_get_list(const char *zipfn)
{
	unzFile uf=NULL;
	std::stringstream ss;
	int err;

	uf = unzOpen64(zipfn);
	if (!uf) {
		build_error_message("in zipfile_get_list(): "
				"error in opening '%s'", zipfn);
		goto L_GETLIST_ERROR;
	}

	err = __zipfile_get_list(uf, ss);
	unzClose(uf);
	if (err != UNZ_OK) {
		build_error_message("in zipfile_get_list(): "
				"error while get filelist in '%s'\n..%s",
				zipfn, err);
		goto L_GETLIST_ERROR;
	}

	build_error_message("");
	return ss.str();

L_GETLIST_ERROR:
	return std::string("");
}

} // namespace dicom -----------------------------------------------------
