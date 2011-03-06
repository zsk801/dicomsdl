/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include "dicom.h"
#include "memutil.h"
#include "errormsg.h"
#include "instream.h"
#include "outstream.h"

namespace dicom { //------------------------------------------------------

int dicomfile::load_from_file(const char *filename, opttype opt, optarg arg)
{

	LOG_DEBUG_MESSAGE(
			"dicomfile{%p}::load_from_file(%s)\n",
			this, filename);

	try {
		instream *s = new instream(filename);
		stream = (void *)s;
		_read_from_stream(opt, arg);

		long ptrdiff = s->free_unread_data();
		if (ptrdiff) realloc_ptr(ptrdiff);

		clear_error_message();
		return DICOM_OK;
	}
	catch (errtype errcode) {
		append_error_message(
			"in dicomfile::load_from_file(...):"
			" error while loading from file '%s'", filename);
		return errcode;
	}
}

int dicomfile::load_from_data(const char *data, int datasize, opttype opt, optarg arg)
{
	LOG_DEBUG_MESSAGE(
			"dicomfile{%p}::load_from_data(%p, %d, %016llx)\n",
			this, data, datasize, opt);

	try {
		instream *s = new instream((uint8 *)data, datasize,
				(opt & OPT_LOAD_DONOT_COPY_DATA) != OPT_LOAD_DONOT_COPY_DATA // copydata
		);
		stream = (void *)s;
		_read_from_stream(opt, arg);

		if (!(opt & OPT_LOAD_DONOT_COPY_DATA)) { // copydata
			long ptrdiff = s->free_unread_data();
			if (ptrdiff) realloc_ptr(ptrdiff);
		}

		clear_error_message();
		return DICOM_OK;
	}
	catch (errtype errcode) {
		append_error_message(
			"in dicomfile::load_from_data(...):"
			" error while loading from memory <%p>, len=%d'",
			data, datasize);
		return errcode;
	}
}

void dicomfile::_read_from_stream(opttype opt, optarg arg)
{
	instream *s = (instream *)(this->stream);

	s->skip(128);
	if (memcmp(s->read(4), "DICM", 4))
		s->unread(128+4); // preamble in DICOM PART 10 FILE is omitted

	int ret;
	ret = load(s, this, UID_EXPLICIT_VR_LITTLE_ENDIAN, opt, arg);
	if (ret == DICOM_DEFLATED_FILEIMAGE) {
		int offset = get_dataelement(0x00020000)->to_int(0)
				+ 132 // Bramble size
				+ 12; // size of (0002,0000) element
		s->inflate(offset);
		remove_all_dataelements();
		s->skip(128+4);
		ret = load(s, this, UID_DEFLATED_EXPLICIT_VR_LITTLE_ENDIAN, opt);
	}

	if (ret != DICOM_OK)
		throw (errtype)ret;

	if (number_of_elements() == 0) {
		build_error_message("in dicomfile::_read_from_stream(): "
				"dicomfile object is empty; may be not a dicom file");

		throw (errtype)DICOM_ERROR;
	}
}

DLLEXPORT dicomfile* open_dicomfile(const char *filename, opttype opt, optarg arg)
{
	dicomfile *df = new dicomfile();
	int ret = df->load_from_file(filename, opt, arg);
	if ((ret == DICOM_OK) ||
		((opt & OPT_LOAD_CONTINUE_ON_ERROR) && df->number_of_elements()))
		return df;
	else {
		delete df;
		return NULL;
	}
}

DLLEXPORT dicomfile* open_dicomfile_from_memory
				(char *data, int datasize, opttype opt, optarg arg)
{
	dicomfile *df = new dicomfile();
	int ret = df->load_from_data(data, datasize, opt, arg);
	if ((ret == DICOM_OK) ||
		((opt & OPT_LOAD_CONTINUE_ON_ERROR) && df->number_of_elements()))
		return df;
	else {
		delete df;
		return NULL;
	}
}

DLLEXPORT void close_dicomfile(dicomfile *df)
{
	if (df) delete df;
}

int dicomfile::save_to_file(const char *filename, opttype opt)
{
	char *val=NULL;
	FILE *fout=NULL;
	int len;
	int ret;
	ret = save_a(&val, &len, opt);
	if (ret != DICOM_OK) {
		append_error_message("in dicomfile::write_to_file(): "
				"cannot build dicomfile object");
		return ret;
	}

	fout = fopen(filename, "wb");
	if (!fout) {
		free(val);
		build_error_message("in dicomfile::write_to_file(): "
				" cannot write to file '%s'", filename);
		return DICOM_WRITE_ERROR;
	}

	if (fwrite(val, len, 1, fout) != 1) {
		free(val);
		fclose(fout);
		build_error_message("in dicomfile::write_to_file(): "
				" cannot write to file '%s'", filename);
		return DICOM_WRITE_ERROR;
	}

	fclose(fout);
	free(val);
	return DICOM_OK;
}

int dicomfile::save_to_memory_a(char **val_a, int *len_a, opttype opt)
{
	return save_a(val_a, len_a, opt);
}

dicomfile::~dicomfile()
{
	if (stream)
		delete (instream *)stream;
}

void dicomfile::mark_dataset_offset(uint32 offset, dataset *ds)
{
	dataset_pos_list[offset] = ds;
}

dataset* dicomfile::dataset_at(uint32 offset)
{
	std::map<uint32, dataset*>::iterator it;
	it = dataset_pos_list.find(offset);
	if (it != dataset_pos_list.end())
		return it->second;
	else
		return NULL;
}

char* dicomfile::get_filename()
{
	if (stream)
		return ((instream *)stream)->get_filename();
	else
		return NULL;
}

} // namespace dicom -----------------------------------------------------
