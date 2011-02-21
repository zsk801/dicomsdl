/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include "dicom.h"
#include "instream.h"
#include "outstream.h"
#include "errormsg.h"
#include "sequence.h"

namespace dicom { //------------------------------------------------------

int sequence::load(void *_in, dicomfile *dfobj, uidtype _tsuid)
{
	instream* s = (instream *)_in;

	LOG_DEBUG_MESSAGE("sequence{%p}::load( instream{%p} )\n", this, s);
	LOG_DEBUG_MESSAGE("---- dataset sequence start ----\n");

	int endian = (_tsuid == UID_EXPLICIT_VR_BIG_ENDIAN?
					BIG_ENDIAN : LITTLE_ENDIAN);
	tagtype tag;
	uint16 gggg, eeee;
	uint32 len;
	int result = DICOM_OK;
	try {
		while (1) {
			if (s->iseof()) break;

			gggg = s->read16u(endian);
			eeee = s->read16u(endian);
			tag = make_tag(gggg,eeee);

			len = s->read32u(endian);

			if (tag == 0xfffee0dd) break;
			if (tag != 0xfffee000) {
				build_error_message(
					"in sequence::load(): "
					"(fffe,e000) is expected for item data element "
					"but (%04x,%04x) appeared.", gggg, eeee);
				throw DICOM_INSTREAM_ERROR;
			}

			dataset* dsinseq = add_dataset();
			instream *subs;

			if (len != 0xffffffff)
				subs = new instream(s, len);
			else
				subs = new instream(s, -1);

			result = dsinseq->load((void *)subs, dfobj, _tsuid);

			if (len == 0xffffffff)
				len = (int)(subs->currptr() - s->currptr());
			delete subs;

			s->skip(len);

			if (result != DICOM_OK)
				break;
		}
	} catch (errtype err) {
		result = err;
	}

	LOG_DEBUG_MESSAGE("----- dataset sequence end -----\n");

	if (result != DICOM_OK)
		append_error_message("in sequence::load(...):");
	return result;
}

dataset* sequence::add_dataset()
{
	dataset *dsinseq = new dataset();
	seq.push_back(dsinseq);
	return dsinseq;
}

void sequence::add_dataset(dataset* ds)
{
	seq.push_back(ds);
}

void sequence::remove_dataset(int idx, bool delete_dataset) {
	if (idx < 0) return;
	dataset *dsinseq = seq[idx];
	if (delete_dataset)
		delete dsinseq;
	seq.erase(seq.begin()+idx);
}

void sequence::remove_dataset(dataset *ds, bool delete_dataset) {
	std::vector <dataset *>::iterator it;
	for (it = seq.begin(); it != seq.end(); it++)
		if ((*it) == ds) {
			if (delete_dataset)
				delete (*it);
			seq.erase(it);
		}
}

void sequence::remove_all_datasets(bool delete_dataset) {
	if (delete_dataset) {
		int idx = size();
		while (idx--)
			remove_dataset(idx);
	} else {
		seq.clear();
	}
}

void sequence::_save(void *ostream, uidtype tsuid, opttype opt)
{
	// See Part 5: Data Structures and Encoding, Table 7.5-1,2,3

	outstream* os = (outstream *)ostream;

	int endian =
		(tsuid==UID_EXPLICIT_VR_BIG_ENDIAN ? BIG_ENDIAN:LITTLE_ENDIAN);

	uint32 len;

	for (int i = 0; i < size(); i++) {
		// mark dataset's position within dicomfile
		os->mark_offset((long)dataset_at(i));

		// Item tag(4)
		os->write16u(0xfffe, endian);
		os->write16u(0xe000, endian);

		// Item len(4)
		len = -1;
		os->write32u(len, endian);
		if (!(opt & OPT_SAVE_IMPLICIT_DATASET_LENGTH)) {
			os->reserve_bytes_for_length_value(
					(long)dataset_at(i), 4, endian, -4);
			os->mark_start_pos((long)dataset_at(i));
		}

		// Item value
		dataset_at(i)->_save(
				ostream, tsuid, opt|__OPT_SAVE_DATASET_IN_SEQUENCE);

		if (!(opt & OPT_SAVE_IMPLICIT_DATASET_LENGTH)) {
			os->mark_end_pos((long)dataset_at(i));
		}
	}

	// Seq delim tag when SQ has undefined length
	if (opt & OPT_SAVE_IMPLICIT_SQ_LENGTH) {
		os->write16u(0xfffe, endian);
		os->write16u(0xe0dd, endian);
		os->write32u_le(0);
	}
}


} // namespace dicom -----------------------------------------------------
