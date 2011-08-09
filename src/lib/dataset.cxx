/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include <string.h>
#include <sstream>
#include <set>

#include "instream.h"
#include "outstream.h"
#include "errormsg.h"
#include "memutil.h"
#include "dicom.h"
#include "pixelsequence.h"
#include "recordoffset.h"
#include "imagecodec.h"
#include "deflate.h"
#include "sequence.h"

namespace dicom { //------------------------------------------------------

int dataset::load(void *stream, dicomfile* dfobj,
				  uidtype _tsuid, opttype opt, optarg arg)
{
	instream* s;
	dataelement *e;
	uint16 gggg, eeee;
	tagtype tag = 0;
	vrtype vr = VR_NULL;
	uint32 len;
	int endian;
	int vr_implicity;
	int curr_offset;
	tagtype loaduntil;

	s = (instream *)stream;
	tsuid = _tsuid;

	loaduntil = arg;
	if (loaduntil == 0) loaduntil = 0xffffffff;

	dfobj->mark_dataset_offset(s->curr_offset()-8, this);

	try {
		while (1) {

			if (s->iseof()) break;

			endian = (tsuid == UID_EXPLICIT_VR_BIG_ENDIAN?
						BIG_ENDIAN : LITTLE_ENDIAN);
			vr_implicity = (tsuid == UID_IMPLICIT_VR_LITTLE_ENDIAN?
						true : false);
			curr_offset = s->curr_offset();

			gggg = s->read16u(endian);
			eeee = s->read16u(endian);

			// All elements in File Meta Elements shall be encoded as
			// Explicit VR Little Endian Transfer Syntax.
			if (gggg == 0x0200) // tsuid is UID_EXPLICIT_VR_BIG_ENDIAN
				gggg = 0x0002;
			if (gggg == 0x0002) {
				endian = LITTLE_ENDIAN;
				vr_implicity = false;
			}

			tag = make_tag(gggg,eeee);

			if (tag == 0xfffee00d) {
				// Item Delim. Tag ref. # Part 5., Table 7.5-3
				s->read32(endian); break;
			}

			if (tag > loaduntil) break;

			vr = s->read16u_le();
			if (vr_implicity) vr = VR_NULL;

			switch (vr) {
				case VR_OB:			case VR_OW:			case VR_OF:
				case VR_SQ:			case VR_UT:			case VR_UN:
					// PS 3.5-2009, Table 7.1-1
					// Data element with explicit VR of
					// OB, OW, OF, SQ, UT OR UN
					s->read(2);
					len = s->read32u(endian);
					break;

				case VR_FL:			case VR_FD:			case VR_SL:
				case VR_UL:			case VR_SS:			case VR_US:

				case VR_AE:	case VR_AS:	case VR_AT:	case VR_CS:	case VR_DA:
				case VR_DS:	case VR_DT:	case VR_IS:	case VR_LO:	case VR_LT:
				case VR_PN:	case VR_SH:	case VR_ST:	case VR_TM:	case VR_UI:
					// PS 3.5-2009, Table 7.1-2
					// Other than as shown in Table 7.1-1
					len = s->read16u(endian);
					break;

				case VR_UK:
					// VR_UK; strange VR in some DICOM file,
					// probably contains a short string
					len = s->read16u(endian);
					vr = VR_UN;
					break;

				case VR_NULL:
				default:
					// PS 3.5-2009, Table 7.1-3
					// DATA ELEMENT STRUCTURE WITH IMPLICIT VR
					s->unread(2);
					len = s->read32u(endian);
					vr = get_tag_vr(tag);
					if (!vr)
						// just assume as UN
						vr = VR_UN;

					break;
			}

			if (gggg == 0x0004) {
				//(0004,1200), (0004,1202), (0004,1400), (0004,1420)
				if (eeee == 0x1200 || eeee == 0x1202 ||
					eeee == 0x1400 || eeee == 0x1420)
					vr = VR_OFFSET;
			}

			if (vr == VR_SQ)
			{
				// PS 3.5-2009, Table 7.5-1, 7.5-2, 7.5-3
				// Processing VR_SQ

				e = add_dataelement(tag, vr);
				instream *subs;
				sequence *seq = (sequence *)(e->ptr);

				if ((unsigned int)len != 0xffffffff)
					subs = new instream(s, len);
				else
					subs = new instream(s, -1);

				int ret = seq->load(subs, dfobj, tsuid);
				if (ret != DICOM_OK) {
					delete subs;
					throw (errtype)ret;
				}

				if (len == 0xffffffff)
                    len = (int)(subs->currptr() - s->currptr());
				delete subs;

				e->len = len;
				e->endian = endian;

				s->skip(len);
			}
			else if ((vr == VR_OB || vr == VR_OW) && tag == 0x7fe00010)
			{
				if (len != 0xffffffff) {
					e = get_dataelement(0x00280100);
					if (e->to_int(0) > 8  && vr == VR_OB)
						vr = VR_OW; // should be...

					e = add_dataelement
							(tag, vr, len, s->read(len), endian, false);
				}
				else
				{
					// PS 3.5-2009, Table A.4-1, A.4-2
					// A.4 ENCAPSULATION OF ENCODED PIXEL DATA
					instream *subs;
					pixelsequence *pixseq;

					subs = new instream(s, -1);
					pixseq = new pixelsequence();

					int ret = pixseq->load(subs);
					if (ret != DICOM_OK) {
						delete pixseq;
						delete subs;
						throw (errtype)ret;
					}

					len = subs->currptr() - s->currptr();
					delete subs;

					e = add_dataelement(tag, VR_PIXSEQ,
							len, (void *)pixseq, endian, true);

					s->skip(len);
				}
			}
			else if (vr == VR_OFFSET) {
				//(0004,1200), (0004,1202), (0004,1400), (0004,1420)
				uint8* p = s->read(len);
				uint32 offset;
				if (endian == LITTLE_ENDIAN)
					offset = le_get_int32(p);
				else
					offset = be_get_int32(p);

				recordoffset *recoff = new recordoffset(offset, dfobj);

				e = add_dataelement
						(tag, vr, len, (void *)recoff, endian, true);
			}
			else {
				if (len != 0xffffffff) {
					e = add_dataelement
						(tag, vr, len, s->read(len), endian, false);
				}
				else
				{
					// PROBABLY SEQUENCE ELEMENT WITH IMPLICIT VR WITH
					e = add_dataelement(tag, vr);

					instream *subs = new instream(s, -1);
					sequence *seq = (sequence *)(e->ptr);

					int ret = seq->load(subs, dfobj, tsuid);
					if (ret != DICOM_OK) {
						delete subs;
						throw (errtype)ret;
					}

					len = (int)(subs->currptr() - s->currptr());
					delete subs;

					e->len = len;
					e->endian = endian;

					s->skip(len);
				}
			}

			if (tag == 0x00020010) {
				uidtype u = e->to_uid();
				if (u == UID_DEFLATED_EXPLICIT_VR_LITTLE_ENDIAN &&
						tsuid != UID_DEFLATED_EXPLICIT_VR_LITTLE_ENDIAN) {
					tsuid = u;
					return DICOM_DEFLATED_FILEIMAGE;
				}

				tsuid = u;
				LOG_DEBUG_MESSAGE(
					"dataset{%p}::load() - set transfer syntax = %s (%s)\n",
					this, uid_to_uidvalue(tsuid), uid_to_uidname(tsuid));
			}

			LOG_DEBUG_MESSAGE(
				 "{offset:%06x} 0x%08x  %s  (%2d)  %6d  %s\t = %s\n",
				  curr_offset,
				  e->tag, get_vr_repr(e->vr),
				  e->get_vm(), e->len,
				  e->repr_string().c_str(),  get_tag_keyword(e->tag));
		}

	} catch (errtype err) {
		append_error_message(
			"in dataset::load():"
			" error while loading tag=%08x, vr=%s, len=%d at offset:%d",
			tag, get_vr_repr(vr), len, curr_offset);
		return err;
	}

	return DICOM_OK;
}

int dataset::save_a(char **val_a, int *len_a, opttype opt)
{
	int ret;
	outstream *oss = NULL;
	try {
		oss = new outstream();
		_save((void *)oss, UID_UNKNOWN, opt);
		*val_a = NULL; *len_a = 0;

		oss->to_string_a(val_a, len_a);

		// ---------deflate -------------------
		if (val_a && tsuid == UID_DEFLATED_EXPLICIT_VR_LITTLE_ENDIAN) {
			int offset = -1;
			char *val = *val_a;

			if (!memcmp(val+128, "DICM\x02\x00\x00\x00UL\x04\x00", 12))
				offset = le_get_uint32(val+140)+132+12;

			if (opt & OPT_SAVE_WITHOUT_METAINFO)
				offset = 0;

			if (offset >= 0) {
				char *def_val;
				int def_len;

				deflate_dicomfile_a(*val_a, *len_a,
						offset, -1, &def_val, &def_len);
				if(!def_val) {
					build_error_message("in dataset::save_a(): "
						"cannot deflate dicomfile object");
					throw DICOM_DEFLATE_ERROR;
				}
				*val_a = def_val;
				*len_a = def_len;
			}
		}

		ret = DICOM_OK;
	} catch (errtype err) {
		append_error_message("in dataset::save_a(): ");
		if (*val_a) free(*val_a);
		*val_a = NULL;
		*len_a = 0;
		ret = err;
	}
	if (oss) delete oss;
	return ret;
}

int dataset::_save(void *ostream, uidtype tsuid, opttype opt)
{
	uint8 buf[512];
	outstream *os;
	dataelement *e;
	element_dict_type::iterator it;
	tagtype tag;
	int endian;

	os = (outstream *)ostream;

	// write preamble ----------------------------------------------------
	if (!(opt & OPT_SAVE_WITHOUT_PREAMBLE ||
		  opt & __OPT_SAVE_DATASET_IN_SEQUENCE)) {
		memset(buf, 0, 128);
		os->write(buf, 128);
		os->write((uint8 *)"DICM", 4);
	}

	// add group length elements for every groups ------------------------
	{
		std::set<uint16> group_list;
		std::set<uint16>::iterator g_it;
		for (it = edict.begin(); it != edict.end(); it++) {
			group_list.insert(group_in_tag((*it).first));
		}

		for (g_it = group_list.begin(); g_it != group_list.end(); g_it++) {
			if (opt&OPT_SAVE_CALC_GROUPLENGTH || *g_it==0x0002) {
				tag = make_tag(*g_it, 0x0000);
				add_dataelement(tag)->from_int(0);
			}
		}
	}

	// write elements ----------------------------------------------------

	uint16 cur_group = 0xffff;
	dataelement *cur_group_e = NULL;

	if (tsuid == UID_UNKNOWN)
		tsuid = get_dataelement(0x00020010)->to_uid();

	endian =
		(tsuid==UID_EXPLICIT_VR_BIG_ENDIAN ? BIG_ENDIAN:LITTLE_ENDIAN);

	for (it = edict.begin(); it != edict.end(); it++) {
		tag = (*it).first;
		e = (*it).second;

		if (opt & OPT_SAVE_WITHOUT_METAINFO &&
			group_in_tag(tag) == 0x0002)
			continue;

		endian =
			(tsuid==UID_EXPLICIT_VR_BIG_ENDIAN ? BIG_ENDIAN:LITTLE_ENDIAN);

		if (group_in_tag(tag) == 0x0002)
			endian = LITTLE_ENDIAN;

		// finish previous element group
		if (cur_group != group_in_tag(tag)) {
			if (cur_group != 0xffff && cur_group_e != NULL) {
				os->mark_end_pos((long)cur_group_e);
				cur_group = -1;
				cur_group_e = NULL;
			}
		}

		// write an element ------
		e->_save(ostream, tsuid, opt);

		// start new element group
		if (element_in_tag(tag) == 0x0000) {
			cur_group = group_in_tag(tag);
			cur_group_e = e;
			os->reserve_bytes_for_length_value(
					(long)cur_group_e, 4, endian, -4);
			os->mark_start_pos((long)cur_group_e);
		}
	}

	// finish last element group
	if (cur_group != 0xffff && cur_group_e != NULL) {
		os->mark_end_pos((long)cur_group_e);
		cur_group = -1;
		cur_group_e = NULL;
	}

	// Item Delim Tag
	if ((opt & OPT_SAVE_IMPLICIT_DATASET_LENGTH) &&
	    (opt & __OPT_SAVE_DATASET_IN_SEQUENCE)) {
		os->write16u(0xfffe, endian);
		os->write16u(0xe00d, endian);
		os->write32u(0x0000, endian);
	}

	return DICOM_OK;
}

dataelement* dataset::add_dataelement(tagtype tag, vrtype vr,
	uint32 len, void *ptr, int endian, int own_memory)
{
	if (vr == VR_NULL) {
		vr = get_tag_vr(tag);
		if (vr == VR_NULL) {
			LOG_WARNING_MESSAGE("in dataset::add_dataelement(...): "
					"cannot find VR for tag (%04x,%04x); "
					"specify VR if a data element has private tag\n",
					group_in_tag(tag), element_in_tag(tag));
			return nullelement();
		}
	}

	{
		uint16 gggg, eeee;
		gggg = group_in_tag(tag);
		eeee = element_in_tag(tag);
		if (gggg == 0x0004) {
			//(0004,1200), (0004,1202), (0004,1400), (0004,1420)
			if (eeee == 0x1200 || eeee == 0x1202 ||
				eeee == 0x1400 || eeee == 0x1420)
				vr = VR_OFFSET;
		}
	}

	// check data validity
	switch (vr) {
	case VR_FD:
		if (len % 8) {
			LOG_WARNING_MESSAGE("in dataset::add_dataelement(...): "
						"length of dataelement with VR='%s' "
						"should be multiply of 8, but is %d. "
						"value is truncated.\n",
						get_vr_repr(vr), len);
			len = (len/8)*8;
		}
	case VR_AT:		case VR_FL:		case VR_OF:
	case VR_UL:		case VR_SL:
		if (len % 4) {
			LOG_WARNING_MESSAGE("in dataset::add_dataelement(...): "
						"length of dataelement with VR='%s' "
						"should be multiply of 4, but is %d. "
						"value is truncated.\n",
						get_vr_repr(vr), len);
			len = (len/4)*4;
		}
	case VR_US:		case VR_SS:		case VR_OW:
		if (len % 2) {
			LOG_WARNING_MESSAGE("in dataset::add_dataelement(...): "
						"length of dataelement with VR='%s' "
						"should be multiply of 2, but is %d. "
						"value is truncated.\n",
						get_vr_repr(vr), len);
			len = (len/2)*2;
		}
	default:
		break;
	}

	remove_dataelement(tag);

	dataelement *e;
	e = new dataelement();

	e->tag = tag;
	e->vr = vr;
	e->len = len;
	e->ptr = ptr;
	e->endian = endian;
	e->own_memory = own_memory;
	edict[tag] = e;

	if (ptr == NULL) {
		if (vr == VR_SQ) {
			e->ptr = new sequence();
			e->own_memory = true;
		} else if (vr == VR_PIXSEQ) {
			e->ptr = new pixelsequence();
			e->own_memory = true;
		} else if (vr == VR_OFFSET) {
			e->ptr = new recordoffset();
			e->own_memory = true;
		}
	}

	return e;
};

dataelement* dataset::add_dataelement(const char *tagstr, vrtype vr)
{
	char *nextptr, *endptr = (char *)(tagstr)+strlen(tagstr);

	tagtype _tag;
	int _seqidx;
	dataelement *_el;
	dataset *_set = this;
	sequence *seq;

	if ((tagstr[0] >= '0' && tagstr[0] <= '9') ||
		((tagstr[0] == 'F' || tagstr[0] == 'f') &&
		 (tagstr[1] == 'F' || tagstr[1] == 'f'))) {
		while (1) {
			_tag = strtol(tagstr, &nextptr, 16);
			_el = _set->get_dataelement(_tag);

			tagstr = nextptr + 1;
			if (tagstr >= endptr) {
				_el = _set->add_dataelement(_tag, vr);
				break;
			}
			if (_el->vr != VR_SQ) // or _el == NULLELEMENT
				_el = _set->add_dataelement(_tag, VR_SQ);

			_seqidx = strtol(tagstr, &nextptr, 16);
			seq = (sequence *)(_el->ptr);
			if (_seqidx >= seq->size()) {
				seq->add_dataset();
				_seqidx = seq->size()-1;
			}
			_set = seq->dataset_at(_seqidx);
			tagstr = nextptr + 1; if (tagstr >= endptr) break;
		}
	} else {
		tagtype tag = find_tag(tagstr);
		if (tag != 0xffffffff)
			_el = add_dataelement(tag, vr);
		else {
			_el = nullelement();
		}
	}
	return _el;
}

dataelement* dataset::get_dataelement(tagtype tag)
{
	element_dict_type::iterator it;
	it = edict.find(tag);
	if (it != edict.end()) {
		return it->second;
	}
	else {
		return nullelement();
	}
}

dataelement* dataset::get_dataelement(const char *tagstr)
{
	char *nextptr, *endptr = (char *)tagstr+strlen(tagstr);

	tagtype _tag;
	int _seqidx;
	dataelement *_el;
	dataset* _set = this;

	if ((tagstr[0] >= '0' && tagstr[0] <= '9') ||
		((tagstr[0] == 'F' || tagstr[0] == 'f') &&
		 (tagstr[1] == 'F' || tagstr[1] == 'f'))) {
		while (1) {
			_tag = strtol(tagstr, &nextptr, 16);
			_el = _set->get_dataelement(_tag);
			if (!_el->is_valid())
				break;
			tagstr = nextptr + 1; if (tagstr >= endptr) break;
			if (_el->vr != VR_SQ)
				{ _el = nullelement(); break;	}
			_seqidx = strtol(tagstr, &nextptr, 10);
			_set = _el->dataset_at(_seqidx);
			tagstr = nextptr + 1;
			if (_set == NULL || tagstr >= endptr)
				{ _el = nullelement(); break;	}
		}
	} else {
		tagtype tag = find_tag(tagstr);
		if (tag != 0xffffffff)
			_el = get_dataelement(tag);
		else {
			LOG_WARNING_MESSAGE("in dataset::get_dataelement(...): "
					"cannot find keyword \"%s\"\n", tagstr);
			_el = nullelement();
		}
	}
	return _el;
}

void dataset::remove_dataelement(tagtype tag)
{
	element_dict_type::iterator it = edict.find(tag);
	if (it != edict.end()) {
		delete it->second; // delete will do free_value()
		edict.erase(tag);
	}
}

void dataset::remove_all_dataelements()
{
	element_dict_type::iterator it = edict.begin();
	while (it != edict.end()) {
		delete it->second; // delete will do free_value()
		it++;
	}
	edict.clear();

	LOG_DEBUG_MESSAGE("dataset{%p}::remove_all_dataelements()\n", this);
}

void dataset::dump_string_a(char **val_a, int *len_a, std::string prefix) {
	std::stringstream ss;
	if (prefix.size() == 0)
		ss << "TAG\tVR\tVM\tLEN\tVAL\t# KEYWORD\n";

	_dump(&(ss), prefix+std::string("'"));

	*len_a = (int)ss.tellp();
	*val_a = (char *)malloc(*len_a+1);
	ss.read(*val_a, *len_a);
	(*val_a)[*len_a] = '\0';
};

std::string dataset::dump_string(std::string prefix) {
	char *val;
	int len;
	dump_string_a(&val, &len, prefix);
	std::string v(val, len);
	if (val) free(val);
	return v;
};

void dataset::_dump(std::iostream *os, std::string prefix)
{
	dataelement_iterator iter(this);
	dataelement *e;
	char buf[256];

	while (e = iter.next()) {
		(*os) << prefix;
		sprintf(buf, "%08x'\t%s\t", e->tag, get_vr_repr(e->vr));
		(*os) << buf;
		sprintf(buf, "(%2d)\t%6d\t", e->get_vm(), e->len );
		(*os) << buf;
		(*os) << e->repr_string().c_str();

		switch (e->tag) {
		case 0x00020002:
		case 0x00020010:
		case 0x00080016:
			{
				(*os) << " = ";
				const char *s =
						uidvalue_to_uidname(e->to_string().c_str());
				(*os) << (s ? s : "??? unknown UID");
			}
		}

		(*os) << "\t# " << get_tag_keyword(e->tag) << "\n";

		// -- process sub datasets in sequence

		if (e->vr == VR_SQ) {
			for (int i = 0;
				 i < e->number_of_datasets();
				 i++) {
				sprintf(buf, "%s%08x.%d.", prefix.c_str(), e->tag, i);
				e->dataset_at(i)->_dump(os, std::string(buf));
			}
		}

		// -- process pixel sequence

		if (e->vr == VR_PIXSEQ)
			((pixelsequence *)e->ptr)->dump(os, "\t\t\t\t\t");
	}
}

void dataset::get_image_info(
		int *width, int *height,
		int *prec, int *sgnd,
		int *ncomps, int *bytes_per_pixel,
		int *nframes)
{
	// (0028,0011) # Rows
	*width = get_dataelement(0x00280011)->to_int(0);

	// (0028,0010) # Columns
	*height = get_dataelement(0x00280010)->to_int(0);

	// (0028,0101) # Bits Stored
	*prec = get_dataelement(0x00280101)->to_int(0);

	// (0028,0103) # Pixel Representation
	// 0 - unsigned
	// 1 - signed
	*sgnd = get_dataelement(0x00280103)->to_int(0);

	// (0028,0002) # Samples per Pixel
	*ncomps = get_dataelement(0x00280002)->to_int(0);


	// (0028,0100) # Bits Allocated
	int bitsalloc = get_dataelement(0x00280100)->to_int(0);

	if (bitsalloc > 8)
		*bytes_per_pixel = *ncomps * 2;
	else if (bitsalloc > 0)
		*bytes_per_pixel = *ncomps;
	else
		*bytes_per_pixel = 0;

	// (0028,0008) # Number of Frames
	*nframes = get_dataelement(0x00280008)->to_int(1);
}

void dataset::set_image_dimension(
		int width, int height,
		int prec, int sgnd, int ncomps)
{
	// (0028,0011) # Rows
	add_dataelement(0x00280011, VR_US)->from_int(width);

	// (0028,0010) # Columns
	add_dataelement(0x00280010, VR_US)->from_int(height);

	// (0028,0100) # Bits Allocated
	add_dataelement(0x00280100, VR_US)->from_int(prec>8?16:8);

	// (0028,0101) # Bits Stored
	add_dataelement(0x00280101, VR_US)->from_int(prec);

	// (0028,0102) # High Bit
	add_dataelement(0x00280102, VR_US)->from_int(prec-1);

	// (0028,0103) # Pixel Representation
	// 0 - unsigned
	// 1 - signed
	add_dataelement(0x00280103, VR_US)->from_int(sgnd);

	// (0028,0002) # Samples per Pixel
	add_dataelement(0x00280002, VR_US)->from_int(ncomps);
}

void dataset::get_pixeldata_a(char **val_a, int *len_a)
{
	char *val = NULL;
	int len = 0;

	int width, height, prec, sgnd, ncomps, bpp, nframes;
	int ret;
	dataelement *e;

	get_image_info(&width, &height, &prec, &sgnd, &ncomps, &bpp, &nframes);

	e = get_dataelement(0x7fe00010);
	if (!(e->is_valid()) || width*height*bpp == 0) {
		val = NULL;
		len = 0;
		return; // DICOM_ERROR
	}

	len = width * height * nframes * bpp;
	val = (char*)malloc(len);

	if (!val) {
		build_error_message("in dataset::get_pixeldata_a(): "
				"cannot alloc %d bytes for pixeldata", len);
		*val_a = NULL;
		*len_a = 0;
		return;
	}

	ret = copy_pixeldata_to(val, width*bpp, width*bpp*height, -1);

	if (ret < 0) { // Error
		free(val);
		val = NULL;
		len = 0;
	}

	*val_a = val;
	*len_a = len;
}

int dataset::copy_pixeldata_to
	(char *buf, int rowstep, int framestep, int idx)
{
	int width, height, prec, sgnd, ncomps, bpp, nframes;
	int ret;
	dataelement *e;

	get_image_info(&width, &height, &prec, &sgnd, &ncomps, &bpp, &nframes);

	e = get_dataelement(0x7fe00010);
	if (!(e->is_valid()) || width*height*bpp == 0) {
		build_error_message(
			"in dataset::copy_pixeldata_to(...):"
			" invalid image component(s) -"
			" PixelData, Rows, Columns, BitsStored, SamplesPerPixel");
		return DICOM_IMAGE_ERROR;
	}

	switch (tsuid) {
	case UID_IMPLICIT_VR_LITTLE_ENDIAN:
	case UID_EXPLICIT_VR_LITTLE_ENDIAN:
	case UID_DEFLATED_EXPLICIT_VR_LITTLE_ENDIAN:
	case UID_EXPLICIT_VR_BIG_ENDIAN:
	{
		e->set_endian();
		nframes = e->len / (width*height*bpp);

		int srcrowstep, srcframestep;
		srcrowstep = width * bpp;
		srcframestep = srcrowstep * height;
		if (idx < 0) { // copy all pixel data
			char *p, *q;
			p = (char *)(e->ptr);
			if (framestep < 0) buf += -framestep*(nframes-1);
			for (int j = 0; j < nframes; j++) {
				q = buf;
				if (rowstep < 0) q += -rowstep*(height-1);
				for (int i = 0; i < height; i++) {
					memcpy(q, p, srcrowstep);
					p += srcrowstep;
					q += rowstep;
				}
				buf += framestep;
			}
			ret = DICOM_OK;
		}
		else { // copy one frame pixel data
			if (idx < nframes) {
				char *p, *q;
				p = (char *)(e->ptr) + srcframestep * idx;
				q = buf;
				if (rowstep < 0) q += -rowstep*(height-1);
				for (int i = 0; i < height; i++) {
					memcpy(q, p, srcrowstep);
					p += srcrowstep;
					q += rowstep;
				}
				ret = DICOM_OK;
			} else {
				build_error_message(
					"in dataset::copy_pixeldata_to(...):"
					" illegal frame index %d", idx);
				ret = DICOM_ILLEGAL_INDEX_ERROR;
			}
		}
	}
	break;

	case UID_JPEG_BASELINE_PROCESS_1:
	case UID_JPEG_EXTENDED_PROCESS_2AND4:
	case UID_JPEG_LOSSLESS_NON_HIERARCHICAL_PROCESS_14:
	case UID_JPEG_LOSSLESS_NON_HIERARCHICAL_FIRST_ORDER_PREDICTION_PROCESS_14:
	case UID_JPEG_2000_IMAGE_COMPRESSION_LOSSLESS_ONLY:
	case UID_JPEG_2000_IMAGE_COMPRESSION:
	case UID_JPEG_LS_LOSSLESS_IMAGE_COMPRESSION:
	case UID_JPEG_LS_LOSSY_NEAR_LOSSLESS_IMAGE_COMPRESSION:
	case UID_JPEG_2000_PART_2_MULTI_COMPONENT_IMAGE_COMPRESSION_LOSSLESS_ONLY:
	case UID_JPEG_2000_PART_2_MULTI_COMPONENT_IMAGE_COMPRESSION:
	case UID_JPIP_REFERENCED:
	case UID_JPIP_REFERENCED_DEFLATE:
	case UID_MPEG2_MAIN_PROFILE_MAIN_LEVEL:
	case UID_RLE_LOSSLESS:
		{
			char *src;
			int srclen;
			int isalloc;

			PIXELDATA_INFO pi;
			memset(&pi, 0, sizeof(PIXELDATA_INFO));
			pixelsequence *pixseq = (pixelsequence *)(e->ptr);
			nframes = pixseq->number_of_frames();

			if (idx < 0) { // copy all pixel data
				if (framestep < 0) buf += -framestep*(nframes-1);
				for (int i = 0; i <nframes; i++) {
					pixseq->get_frame_data_a(i, &src, &srclen, &isalloc);
					ret = decode_pixeldata(
							tsuid,
							src, srclen,
							buf, rowstep, &pi);
					if (isalloc) free(src);
					buf += framestep;
					if (ret < 0) break;
				}
			}
			else { // copy one frame pixel data
				if (idx < nframes) {
					pixseq->get_frame_data_a(idx, &src, &srclen, &isalloc);
					ret = decode_pixeldata(
							tsuid,
							src, srclen,
							buf, rowstep, &pi);
					if (isalloc) free(src);
				} else {
					build_error_message(
						"in dataset::copy_pixeldata_to(...):"
						" illegal frame index %d", idx);
					ret = DICOM_ILLEGAL_INDEX_ERROR;
				}
			}
		}
		break;

	default:
		build_error_message(
			"in dataset::copy_pixeldata_to(...):"
			" unsupported format \"%s\"", uid_to_uidname(tsuid));
		ret = DICOM_UNSUPPORTED_TRANSFERSYNTAX_ERROR;
		break;
	}
	return ret;
}

int dataset::set_pixeldata(uidtype tsuid, char *buf,
	int width, int height, int prec, int sgnd, int ncomps, int nframes,
	int rowstep, int framestep, int quality)
{
	int ret;
	if (nframes < 1) nframes = 1;

	//-----

	if (tsuid == UID_JPEG_BASELINE_PROCESS_1 && prec > 8) {
		build_error_message("in dataset::set_pixeldata(...):"
			" JPEG BASE PROCESS 1 encoding"
			" doesn't allow %d bits precision", prec);
		return DICOM_ILLEGAL_ARGUMENT_ERROR;
	}
	if (tsuid == UID_JPEG_EXTENDED_PROCESS_2AND4 && prec > 12) {
		build_error_message("in dataset::set_pixeldata(...):"
			" JPEG EXTENDED PROCESS 2 & 4 encoding"
			" doesn't allow %d bits precision", prec);
		return DICOM_ILLEGAL_ARGUMENT_ERROR;
	}

	//------

	set_image_dimension(width, height, prec, sgnd, ncomps);
	int bpp = (prec>8?2:1)*ncomps;

	switch (tsuid) {
	case UID_IMPLICIT_VR_LITTLE_ENDIAN:
	case UID_EXPLICIT_VR_LITTLE_ENDIAN:
	case UID_DEFLATED_EXPLICIT_VR_LITTLE_ENDIAN:
	case UID_EXPLICIT_VR_BIG_ENDIAN:
	{
		vrtype vr = (prec > 8? VR_OW : VR_OB);
		dataelement *e = add_dataelement(0x7fe00010, vr);
		e->set_endian();

		int dstrowstep, dstframestep;
		dstrowstep = width * bpp;
		dstframestep = dstrowstep * height;
		ret = e->alloc_memory(dstframestep*nframes);
		if (ret == DICOM_OK) { // copy all pixel data
			char *p, *q;
			q = (char *)(e->ptr);
			if (framestep < 0) buf += -framestep*(nframes-1);
			for (int j = 0; j < nframes; j++) {
				p = buf;
				if (rowstep < 0) p += -rowstep*(height-1);
				for (int i = 0; i < height; i++) {
					memcpy(q, p, dstrowstep);
					p += rowstep;
					q += dstrowstep;
				}
				buf += framestep;
			}
			ret = DICOM_OK;
		}
	}
	break;
	default:
	{
		dataelement *e = add_dataelement(0x7fe00010, VR_PIXSEQ);
		pixelsequence *pixseq = (pixelsequence *)(e->ptr);
		PIXELDATA_INFO pi;
		memset(&pi, 0, sizeof(PIXELDATA_INFO));
		pi.prec = prec;
		pi.rows = height;
		pi.cols = width;
		pi.ncomps = ncomps;
		pi.opt[OPT_JPEG_QUALITY] = quality;
		pi.opt[OPT_JPEG2K_QUALITY] = quality;

		if (framestep < 0) buf += -framestep*(nframes-1);
		for (int i = 0; i < nframes; i++) {
			char *src = buf;
			int srcstep = rowstep;
			if (rowstep < 0) src += -rowstep*(height-1);
			char *dst;
			int dstlen;

			ret = encode_pixeldata(
					tsuid, src, srcstep, &dst, &dstlen, &pi);
			if (ret != DICOM_OK) break;

			ret = pixseq->add_frame_data(dst, dstlen, true, 0);
			free(dst);
			if (ret != DICOM_OK) break;

			buf += framestep;
		}
		break;
	}

	} // switch (tsuid)

	if (ret == DICOM_OK) {
		this->tsuid = tsuid;

		// Transfer Syntax UID

		add_dataelement(0x00020010, VR_UI)
			->from_string(uid_to_uidvalue(tsuid));

		// set 'attribute lossy image compression'
		if ((tsuid == UID_JPEG_2000_IMAGE_COMPRESSION && quality != 0)||
			tsuid == UID_JPEG_BASELINE_PROCESS_1 ||
			tsuid == UID_JPEG_EXTENDED_PROCESS_2AND4 ||
			tsuid == UID_JPEG_LS_LOSSY_NEAR_LOSSLESS_IMAGE_COMPRESSION )
			add_dataelement(0x00282110, VR_CS)->from_string("01");
	}
	else { // ERROR
		append_error_message("in dataset::set_pixeldata(...):");
	}
	return ret;
}

int dataset::set_filemetainfo(
	uidtype sop_class_uid,
	char* sop_instance_uid,
	uidtype transfer_syntax_uid)
{
	// PS3-10, Table 7.1-1 DICOM File Meta Information
	dataelement *e;
	char *val;
	int len;

	// File Meta Information Version -------------------------------------
	e = get_dataelement(0x00020001);
	if (!e->is_valid())
		add_dataelement(0x00020001, VR_OB)->from_data("\x00\x01", 2);

	// Media Storage SOP Class UID ---------------------------------------
	if (sop_class_uid) {
		add_dataelement(0x00020002, VR_UI)
				->from_string(uid_to_uidvalue(sop_class_uid));
		add_dataelement(0x00080016, VR_UI)
				->from_string(uid_to_uidvalue(sop_class_uid));
	} else {
		get_dataelement(0x00080016)->to_string_a(&val, &len);
		if (val) {
			add_dataelement(0x00020002, VR_UI)->from_string(val);
			free(val);
		} else {
			build_error_message("in dataset::set_filemetainfo():"
					" no valid sop_class_uid is provided");
			return DICOM_ILLEGAL_ARGUMENT_ERROR;
		}
	}

	// SOP Instance UID --------------------------------------------------
	if (sop_instance_uid) {
		add_dataelement(0x00020003, VR_UI)->from_string(sop_instance_uid);
		add_dataelement(0x00080018, VR_UI)->from_string(sop_instance_uid);
	} else {
		// use (0008,0018) value for (0002,0003)
		get_dataelement(0x00080018)->to_string_a(&val, &len);
		if (val) {
			add_dataelement(0x00020003, VR_UI)->from_string(val);
			free(val);
		} else {
			build_error_message("in dataset::set_filemetainfo():"
					" no valid sop_instance_uid is provided");
			return DICOM_ILLEGAL_ARGUMENT_ERROR;
		}
	}

	// Transfer Syntax UID
	if (!transfer_syntax_uid) {
		if (get_dataelement(0x00020010)->is_valid())
			transfer_syntax_uid = get_dataelement(0x00020010)->to_uid();
		else
			transfer_syntax_uid = UID_EXPLICIT_VR_LITTLE_ENDIAN;
	}
	if (get_dataelement(0x7fe00010)->is_valid())
		change_pixelencoding(transfer_syntax_uid);
	else {
		tsuid = transfer_syntax_uid;
		add_dataelement(0x00020010, VR_UI)
			->from_string(uid_to_uidvalue(tsuid));
	}

	// Implementation Class UID
	add_dataelement(0x00020012, VR_UI)
		->from_string(DICOMSDL_IMPLCLASSUID);

	// Implementation Class UID
	add_dataelement(0x00020013, VR_SH)
		->from_string(DICOMSDL_IMPLVERNAME);

	return DICOM_OK;
}

#define is_rawtype(u) \
	((u) == UID_IMPLICIT_VR_LITTLE_ENDIAN || \
	 (u) == UID_EXPLICIT_VR_LITTLE_ENDIAN || \
	 (u) == UID_DEFLATED_EXPLICIT_VR_LITTLE_ENDIAN || \
	 (u) == UID_EXPLICIT_VR_BIG_ENDIAN)

int dataset::change_pixelencoding(uidtype new_tsuid, int quality)
{
	int ret;

	if (is_rawtype(tsuid) && is_rawtype(new_tsuid)) {
		tsuid = new_tsuid;
	}
	else {
		// decode and encode it!
		char *val = NULL;
		int len = 0;

		int width, height, prec, sgnd, ncomps, bpp, nframes;
		dataelement *e;

		get_image_info(&width, &height, &prec, &sgnd, &ncomps, &bpp, &nframes);

		e = get_dataelement(0x7fe00010);
		if (!(e->is_valid()) || width*height*bpp == 0) {
			build_error_message(
				"in dataset::change_pixelencoding(...):"
				" invalid image component(s) -"
				" PixelData, Rows, Columns, BitsStored, SamplesPerPixel");
			return DICOM_IMAGE_ERROR;
		}

		len = width * height * nframes * bpp;

		val = (char*)malloc(len);
		if (!val) {
			build_error_message(
				"in dataset::change_pixelencoding(...):"
				" cannot allocate %d bytes", len);
			return DICOM_MEMORY_ERROR;
		}

		ret = copy_pixeldata_to(val, width*bpp, width*bpp*height, -1);
		if (ret < 0) {
			append_error_message("in dataset::change_pixelencoding(...):");
			free(val);
			return ret;
		}

		set_pixeldata(new_tsuid, val,
			width, height, prec, sgnd, ncomps, nframes,
			width*bpp, width*bpp*height, quality);
		tsuid = new_tsuid;
	}

	if (!(*get_dataelement(0x00020001))) {
		// figure out using information in dataset.
		if (set_filemetainfo(UID_UNKNOWN, NULL, new_tsuid)) {
			append_error_message("in dataset::change_pixelencoding(...):"
					" error in setting metainfo for this image");
			return DICOM_ERROR;
		}
	}
	else
		add_dataelement(0x00020010, VR_UI)
			->from_string(uid_to_uidvalue(new_tsuid));

	return DICOM_OK;
}

void dataset::realloc_ptr(long ptrdiff)
{
	if (ptrdiff == 0) return;

	element_dict_type::iterator it;
	dataelement *e;

	for (it = edict.begin(); it != edict.end(); it++) {
		e = (*it).second;
		if (e->own_memory == false && e->ptr) {
			char *p = (char *)(e->ptr); p += ptrdiff; e->ptr = p;
		} else if (e->vr == VR_SQ) {
			for (int i = 0;
				 i < e->number_of_datasets();
				 i++)
				e->dataset_at(i)->realloc_ptr(ptrdiff);
		}
	}
}

} // namespace dicom -----------------------------------------------------

