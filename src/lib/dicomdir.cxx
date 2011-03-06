/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include <string>
#include <map>
#include "dicom.h"
#include "errormsg.h"
#include "dicomdir.h"

namespace dicom { //------------------------------------------------------

const char *default_tag_list_for_dicomdir[] = {
		// Table F.5-1, PATIENT KEYS
		"PATIENT", "00100010", // Patient's Name (type2)
		"PATIENT", "00100020", //  Patient ID*

		// Table F.5-2 STUDY KEYS
		"STUDY", "00080020", // Study Date
		"STUDY", "00080030", // Study Time
		"STUDY", "00081030", // Study Description
		"STUDY", "0020000D", // Study Instance UID*
		"STUDY", "00200010", // Study ID
		"STUDY", "00080050", // Accession Number (type2)

		// Table F.5-3 SERIES KEYS
		"SERIES", "00080060", // Modality
		"SERIES", "0020000E", // Series Instance UID*
		"SERIES", "00200011", // Series Number

		// Table F.5-4 IMAGE KEYS
		"IMAGE", "00200013", // Instance Number

		NULL};

static dicomdir_taglist_t _tags;
static tagtype largest_tag;

DLLEXPORT void add_tags_for_dicomdir(char **drtype_taglist)
{
	_tags.add_tags(drtype_taglist);
}

DLLEXPORT void reset_tags_for_dicomdir()
{
	_tags.reset_tags();
}

// -----------------------------------------------------------------------

list_pchar_t* get_tags_for_dicomdir(const char *drtype)
{
	return _tags.get_tags(drtype);
}

tagtype _string_to_tag(char *tagstr)
{
	tagtype ret;
	ret = find_tag(tagstr);
	if (ret != 0xffffffff)
		return ret;
	char *dummy;
	return (tagtype)strtol(tagstr, &dummy, 16);
}

void dicomdir_taglist_t::add_tags(char **drtype_taglist)
{
	char **pp = drtype_taglist;
	char *pk, *pv;
	char *k, *v;
	tagtype tag;

	while (true) {
		pk = *pp++;
		if (!pk) break;
		pv = *pp++;
		if (!pv) break;

		tag = _string_to_tag(pv);
		if (tag > largest_tag) largest_tag = tag;

		// is directory record type string in the map?
		map_pchar_t::iterator it;
		it = taglist.find(pk);

		if (it != taglist.end()) {
			k = it->first;
		} else {
			// copy key (dir rec type)
			k = (char *)malloc(strlen(pk)+1);
			strcpy(k, pk);
		}

		// copy value (tag)
		v = (char *)malloc(strlen(pv)+1);
		strcpy(v, pv);

		list_pchar_t& tl = taglist[k];
		tl.push_back(v);
	}
};


void dicomdir_taglist_t::reset_tags()
{
	LOG_DEBUG_MESSAGE("taglist_t{%p}::reset_tags() - "
			"add mandatory tags for DICOMDIR\n", this);
	largest_tag = 0x00;

	// add mandatory tags for each directory record types
	free_tags();
	add_tags((char **)default_tag_list_for_dicomdir);
}

list_pchar_t* dicomdir_taglist_t::get_tags(const char *drtype)
{
	map_pchar_t::iterator it = taglist.find((char *)drtype);
	if (it != taglist.end())
		return &(it->second);
	else
		return NULL;
}

void dicomdir_taglist_t::free_tags()
{
	map_pchar_t::iterator i;
	list_pchar_t::iterator j;
#ifdef __DEBUG__
LOG_DEBUG_MESSAGE("taglist_t{%p}::free_tags() - "
		"free %d tag-string list pair(s)\n",
	this, taglist.size());
#endif
	for (i = taglist.begin(); i != taglist.end(); i++) {
#ifdef __DEBUG__
LOG_DEBUG_MESSAGE("taglist_t{%p}::free_tags() - "
		". free directory record type string '%s'\n",
	this, i->first);
#endif
		free(i->first);
		list_pchar_t &l = i->second;
		for (j = l.begin(); j != l.end(); j++) {
#ifdef __DEBUG__
LOG_DEBUG_MESSAGE("taglist_t{%p}::free_tags() - "
		"... free tag string '%s'\n", this, *j);
#endif
			free(*j);
		}
	}

	taglist.clear();
}

// dicomdir --------------------------------------------------------------

DLLEXPORT dicomdir* open_dicomdir(const char *filename, opttype opt)
{
	dicomdir *dr = new dicomdir();
	int ret = dr->load_from_file(filename, opt);
	if ((ret == DICOM_OK) || (opt & OPT_LOAD_CONTINUE_ON_ERROR))
		return dr;
	else {
		delete dr;
		return NULL;
	}
}

DLLEXPORT dicomdir* open_dicomdir_from_memory
				(char *data, int datasize, opttype opt)
{
	dicomdir *dr = new dicomdir();
	int ret = dr->load_from_data(data, datasize, opt);
	if ((ret == DICOM_OK) || (opt & OPT_LOAD_CONTINUE_ON_ERROR))
		return dr;
	else {
		delete dr;
		return NULL;
	}
}

DLLEXPORT void close_dicomdir(dicomdir *dr)
{
	if (dr) delete dr;
}


int dicomdir::load_from_file(const char *filename, opttype opt)
{
	if (df) delete df;
	if (root_dirrec) delete root_dirrec;
	root_dirrec = new dirrec_t;
	root_dirrec->set_dirrec_type("ROOT");

	df = new dicomfile();
	int ret = df->load_from_file(filename, opt);
	if ((ret != DICOM_OK) && !(opt & OPT_LOAD_CONTINUE_ON_ERROR)) {
		delete df; df = NULL;
		return ret;
	}

	uidtype scuid = (*df)[0x00020002];
	if (scuid == UID_MEDIA_STORAGE_DIRECTORY_STORAGE) {
		// (0004,1200)=OffsetOfTheFirstDirectoryRecordOfTheRootDirectoryEntity
		analyze_directory_records(root_dirrec, (*df)[0x00041200]);
	}
	else {
		ret = DICOM_ERROR;
		delete df; df = NULL;
	}
	return ret;
}

int dicomdir::load_from_data(const char *data, int datasize, opttype opt)
{
	if (df) delete df;
	if (root_dirrec) delete root_dirrec;
	root_dirrec = new dirrec_t;
	root_dirrec->set_dirrec_type("ROOT");

	df = new dicomfile();
	int ret = df->load_from_data(data, datasize, opt);
	if ((ret != DICOM_OK) && !(opt & OPT_LOAD_CONTINUE_ON_ERROR)) {
		delete df; df = NULL;
		return ret;
	}

	uidtype scuid = (*df)[0x00020002];
	if (scuid == UID_MEDIA_STORAGE_DIRECTORY_STORAGE) {
		// (0004,1200)=OffsetOfTheFirstDirectoryRecordOfTheRootDirectoryEntity
		analyze_directory_records(root_dirrec, (*df)[0x00041200]);
	}
	else {
		ret = DICOM_ERROR;
		delete df; df = NULL;
	}
	return ret;
}


dicomdir::dicomdir()
{
	df = NULL;
	root_dirrec = new dirrec_t;
	root_dirrec->set_dirrec_type("ROOT");
}

dicomdir::~dicomdir()
{
	if (df) delete df;
	if (root_dirrec) delete root_dirrec;
}

void dicomdir::analyze_directory_records(dirrec_t* base_dir, dataset *ds)
{
	while (ds) {
		// DirectoryRecordType
		char* drtype_str;
		ds->get_dataelement(0x00041430)->to_string_a(&drtype_str, NULL);
		if (drtype_str == NULL) {
			drtype_str = (char *)malloc(4);
			*drtype_str = '\0';

			LOG_WARNING_MESSAGE("in dicomdir::analyze_directory_records(..): "
					"no DirectoryRecordType in dataset (%s)\n",
					df->get_filename());
		}

		std::string key;

		if (!drtype_str) break;
		if (strlen(drtype_str) > 4) {
			switch (le_get_uint32(drtype_str)) {
			case 0x49544150: // 'PATI' ENT
				// key = PatientID
				key = ds->get_dataelement(0x00100020)->to_string();
				break;
			case 0x44555453: // 'STUD' Y
				// key = ReferencedSOPInstanceUIDInFile
				key = ds->get_dataelement(0x00041511)->to_string();
				if (key.size() == 0)
					// key = StudyInstanceUID if
					//   ReferencedSOPInstanceUIDInFile is abscent
					key = ds->get_dataelement(0x0020000d)->to_string();
				break;
			case 0x49524553: // 'SERI' ES
				key = ds->get_dataelement(0x00041511)->to_string();

				// key = SeriesInstanceUID
				// if ReferencedSOPInstanceUIDInFile is absent
				key = ds->get_dataelement(0x0020000e)->to_string();
				break;
			case 0x47414d49: // 'IMAG' E
			case 0x41544144: // 'DATA' SET
							 // strange type in GE LS PET scanner
			case 0x44205452: // 'RT D' OSE
			case 0x53205452: // 'RT S' TRUCTURE SET
			case 0x50205452: // 'RT P' LAN
			case 0x54205452: // 'RT T' REAT RECORD
			case 0x53455250: // 'PRES' ENTATION
			case 0x45564157: // 'WAVE' FORM
			case 0x44205253: // 'SR D' OCUMENT
			case 0x2059454b: // 'KEY ' OBJECT DOC /** TODO:
			case 0x43455053: // 'SPEC' TROSCOPY
			case 0x20574152: // 'RAW ' DATA
			case 0x49474552: // 'REGI' STRATION
			case 0x55444946: // 'FIDU' CIAL
			case 0x474e4148: // 'HANG' ING PROTOCOL
			case 0x41434e45: // 'ENCA' P DOC
			case 0x20374c48: // 'HL7 ' STRUC DOC
			case 0x554c4156: // 'VALU' E MAP
			case 0x52455453: // 'STER' EOMETRIC
			case 0x454c4150: // 'PALE' TTE
			case 0x56495250: // 'PRIV' ATE
			default:
				{
					// if there is Instance Number, use that as key
					int i; char s[64];
					i = ds->get_dataelement(0x00200013)->to_int(-1);
					if (i != -1) {
						sprintf(s, "%06d.", i);
						key = std::string(s);
					}
				}
				// key = ReferencedSOPInstanceUIDInFile
				key += ds->get_dataelement(0x00041511)->to_string();
				break;
			}
		}
		if (key.size() == 0) {
			LOG_WARNING_MESSAGE("in dicomdir::analyze_directory_records(..): "
					"no suitable key in dataset (%s)\n",
					df->get_filename());
			char s[32];
			sprintf(s, "{%p}", ds);
			key = std::string(s);
		}

		// take
		dirrec_t* curr_dir;
		std::map<std::string, dirrec_t *>::iterator it;
		it = base_dir->lowlevdir.find(key);
		if (it != base_dir->lowlevdir.end()) {
			curr_dir = it->second;
		} else {
			curr_dir = new dirrec_t;
			curr_dir->set_dirrec_type(drtype_str);
			curr_dir->ds = ds;
			base_dir->lowlevdir[key] = curr_dir;
		}

		free(drtype_str);

		// OffsetOfReferencedLowerLevelDirectoryEntity
		dataset *leafds = ds->get_dataelement(0x00041420)->to_dataset();
		if (leafds)
			analyze_directory_records(curr_dir, leafds);

		// OffsetOfTheNextDirectoryRecord
		ds = ds->get_dataelement(0x00041400)->to_dataset();
	}
};


// directory record structure --------------------------------------------

void dirrec_t::set_dirrec_type(const char *s)
{
	strncpy(dirrec_type, s, 20);
	dirrec_type[19] = '\0';
}

dirrec_t::dirrec_t() {
	ptr_as_uint32(dirrec_type) = 0;
	ds = NULL;
	own_dataset = false;
}

dirrec_t::~dirrec_t() {
	std::map<std::string, dirrec_t *>::iterator it;
	if (ds && own_dataset)
		delete ds;
	for (it = lowlevdir.begin(); it != lowlevdir.end(); it++)
		delete it->second;
}

// add a dicomfile -------------------------------------------------------

dirrec_t* add_lowlevdir
	(dicomfile *df, dirrec_t *base_dirrec,
	 const char *dirrec_type, std::string key)
{
	char *val; int len;
	std::map<std::string, dirrec_t *>::iterator r;
	dirrec_t *lowlev_dirrec;
	list_pchar_t* tags;
	list_pchar_t::iterator t;
	dataelement *e;
	dataset *ds;

	r = base_dirrec->lowlevdir.find(key);
	if (r != base_dirrec->lowlevdir.end()) {
		lowlev_dirrec = r->second;
	} else {
		lowlev_dirrec = new dirrec_t;
		lowlev_dirrec->set_dirrec_type(dirrec_type);
		base_dirrec->lowlevdir[key] = lowlev_dirrec;

		ds = new dataset;
		tags = get_tags_for_dicomdir(dirrec_type);
		for (t = tags->begin(); t != tags->end(); t++) {
			e = df->get_dataelement(*t);
			if (e->is_valid()) {
				e->raw_value(&val, &len);
				ds->add_dataelement(*t, e->vr)->from_data(val, len);
			}
		}
		ds->add_dataelement(0x00041410)->from_int(0xffff);
		ds->add_dataelement(0x00041430)
				->from_string(lowlev_dirrec->dirrec_type);

		lowlev_dirrec->ds = ds;
		lowlev_dirrec->own_dataset = true;
	}

	return lowlev_dirrec;
}

int dicomdir::add_dicomfile_image_type(dicomfile *df, char *ref_file_id)
{
	// mandatory fields --------------------------------------------------

	// (0010,0020) Patient ID
	std::string pat_key = (*df)[0x00100020].to_string();

	// (0020,000d) Study Instance UID
	std::string std_key = (*df)[0x0020000D].to_string();

	// (0020,000e) Series Instance UID
	std::string ser_key = (*df)[0x0020000E].to_string();

	// (0004,1500) >Referenced File ID
	std::string img_key = std::string(ref_file_id);

	// (0004,1510) >Referenced SOP Class UID in File = (0002,0002)
	std::string ref_sop_class_uid = (*df)[0x00020002].to_string();

	// (0004,1511) >Referenced SOP Instance UID in File = (0002,0003)
	std::string ref_sop_inst_uid = (*df)[0x00020003].to_string();

	// (0004,1512) >Referenced Transfer Syntax UID in File = (0002,0010)
	std::string ref_ts_uid = (*df)[0x00020010].to_string();

	if (pat_key.size()==0 || std_key.size()==0 || ser_key.size()==0 ||
		img_key.size()==0 || ref_sop_class_uid.size()==0 ||
		ref_sop_inst_uid.size()==0 || ref_ts_uid.size()==0) {
		return DICOM_ERROR;
	}

	// process patient directory entry -------------------------------
	dirrec_t *pat_dirrec
		= add_lowlevdir(df, root_dirrec, "PATIENT", pat_key);

	// process study directory entry ---------------------------------
	dirrec_t *std_dirrec
		= add_lowlevdir(df, pat_dirrec, "STUDY", std_key);

	// process series directory entry ---------------------------------
	dirrec_t *ser_dirrec
		= add_lowlevdir(df, std_dirrec, "SERIES", ser_key);

	// process image directory entry ----------------------------------
	dirrec_t *img_dirrec
		= add_lowlevdir(df, ser_dirrec, "IMAGE", ser_key);
	img_dirrec->ds->add_dataelement(0x00041500)
			->from_string(img_key);
	img_dirrec->ds->add_dataelement(0x00041510)
			->from_string(ref_sop_class_uid);
	img_dirrec->ds->add_dataelement(0x00041511)
			->from_string(ref_sop_inst_uid);
	img_dirrec->ds->add_dataelement(0x00041512)
			->from_string(ref_ts_uid);

	return DICOM_OK;
};

int dicomdir::add_dicomfile(dicomfile *df, char *ref_file_id)
{
	make_root_dirrec_own_dataset();

	int ret;

	uidtype sc; // sop class
	sc = (*df)[0x00020002];
	switch (sc) {
	case UID_COMPUTED_RADIOGRAPHY_IMAGE_STORAGE:
	case UID_CT_IMAGE_STORAGE:
	case UID_ENHANCED_CT_IMAGE_STORAGE:
	case UID_MR_IMAGE_STORAGE:
	case UID_ENHANCED_MR_IMAGE_STORAGE:
	case UID_ENHANCED_MR_COLOR_IMAGE_STORAGE:
	case UID_NUCLEAR_MEDICINE_IMAGE_STORAGE:
	case UID_ULTRASOUND_IMAGE_STORAGE:
	case UID_ULTRASOUND_MULTI_FRAME_IMAGE_STORAGE:
	case UID_ENHANCED_US_VOLUME_STORAGE:
	case UID_SECONDARY_CAPTURE_IMAGE_STORAGE:
	case UID_MULTI_FRAME_SINGLE_BIT_SECONDARY_CAPTURE_IMAGE_STORAGE:
	case UID_MULTI_FRAME_GRAYSCALE_BYTE_SECONDARY_CAPTURE_IMAGE_STORAGE:
	case UID_MULTI_FRAME_GRAYSCALE_WORD_SECONDARY_CAPTURE_IMAGE_STORAGE:
	case UID_MULTI_FRAME_TRUE_COLOR_SECONDARY_CAPTURE_IMAGE_STORAGE:
	case UID_X_RAY_ANGIOGRAPHIC_IMAGE_STORAGE:
	case UID_ENHANCED_XA_IMAGE_STORAGE:
	case UID_X_RAY_RADIOFLUOROSCOPIC_IMAGE_STORAGE:
	case UID_ENHANCED_XRF_IMAGE_STORAGE:
	case UID_X_RAY_3D_ANGIOGRAPHIC_IMAGE_STORAGE:
	case UID_X_RAY_3D_CRANIOFACIAL_IMAGE_STORAGE:
	case UID_BREAST_TOMOSYNTHESIS_IMAGE_STORAGE:
	case UID_RT_IMAGE_STORAGE:
	case UID_POSITRON_EMISSION_TOMOGRAPHY_IMAGE_STORAGE:
	case UID_ENHANCED_PET_IMAGE_STORAGE:
	case UID_DIGITAL_X_RAY_IMAGE_STORAGE_FOR_PRESENTATION:
	case UID_DIGITAL_X_RAY_IMAGE_STORAGE_FOR_PROCESSING:
	case UID_DIGITAL_MAMMOGRAPHY_X_RAY_IMAGE_STORAGE_FOR_PRESENTATION:
	case UID_DIGITAL_MAMMOGRAPHY_X_RAY_IMAGE_STORAGE_FOR_PROCESSING:
	case UID_DIGITAL_INTRA_ORAL_X_RAY_IMAGE_STORAGE_FOR_PRESENTATION:
	case UID_DIGITAL_INTRA_ORAL_X_RAY_IMAGE_STORAGE_FOR_PROCESSING:
	case UID_VL_ENDOSCOPIC_IMAGE_STORAGE:
	case UID_VL_MICROSCOPIC_IMAGE_STORAGE:
	case UID_VL_SLIDE_COORDINATES_MICROSCOPIC_IMAGE_STORAGE:
	case UID_VL_PHOTOGRAPHIC_IMAGE_STORAGE:
	case UID_VIDEO_ENDOSCOPIC_IMAGE_STORAGE:
	case UID_VIDEO_MICROSCOPIC_IMAGE_STORAGE:
	case UID_VIDEO_PHOTOGRAPHIC_IMAGE_STORAGE:
	case UID_OPHTHALMIC_PHOTOGRAPHY_8_BIT_IMAGE_STORAGE:
	case UID_OPHTHALMIC_PHOTOGRAPHY_16_BIT_IMAGE_STORAGE:
	case UID_OPHTHALMIC_TOMOGRAPHY_IMAGE_STORAGE:
	case UID_SEGMENTATION_STORAGE:
		ret = add_dicomfile_image_type(df, ref_file_id);
		break;
	default:
		LOG_WARNING_MESSAGE("in dicomdir::add_dicomfile(..): "
				"only image SOP instances can be added (%s)\n",
				df->get_filename());
		ret = DICOM_ERROR;
		break;
	}

	return ret; // or DICOM_ERROR
}

int dicomdir::add_dicomfile(const char *filename, char *ref_file_id)
{
	dicomfile *df = NULL;
	int ret;

	df = open_dicomfile(filename, OPT_LOAD_PARTIAL_FILE, largest_tag);
	if (df) {
		ret = add_dicomfile(df, ref_file_id);
		delete df;
	} else {
		LOG_WARNING_MESSAGE("in dicomdir::add_dicomfile(..): "
				"skip '%s'\n", filename);
		ret = DICOM_ERROR;
	}

	return ret;
}

void dicomdir::make_root_dirrec_own_dataset
	(dirrec_t* r, std::set<dataset *> *sds)
{
	if (!df)
		return;

	if (r == NULL) {
		r = root_dirrec;
		sds = new std::set<dataset *>;
	}

	if (r->ds) {
		sds->insert(r->ds);
		r->own_dataset = true;
	}

	std::map<std::string, dirrec_t *>::iterator it;
	for (it = r->lowlevdir.begin(); it != r->lowlevdir.end(); it++)
		make_root_dirrec_own_dataset(it->second, sds);

	if (r != root_dirrec)
		return;

	// now i'm in root entry

	// remove dataset entries in DirectoryRecordSequence
	sequence *seq = (*df)[0x00041220];
	if (seq) {
		for (int i = 0; i < seq->size(); i++) {
			if (sds->find(seq->dataset_at(i)) == sds->end())
				delete seq->dataset_at(i);
		}
		seq->remove_all_datasets(false); // false=do not delete dataset
	}

	delete sds;
}

// build and write dicomfile object -------------------------------------

void dicomdir_link_datasets(dirrec_t *dr, sequence *seq)
{
	if (dr->ds) {
		seq->add_dataset(dr->ds);
		dr->own_dataset = false;
	}

	if (dr->lowlevdir.size() == 0) {
		if (dr->ds)
			// no more OffsetOfReferencedLowerLevelDirectoryEntity
			dr->ds->add_dataelement(0x00041420);
		return;
	}

	std::map<std::string, dirrec_t *>::iterator it, it_next;
	it = dr->lowlevdir.begin();
	it_next = it; it_next++;

	if (dr->ds)
		// 0x00041420 OffsetOfReferencedLowerLevelDirectoryEntity
		dr->ds->add_dataelement(0x00041420)->from_dataset(it->second->ds);

	// 0x00041400 OffsetOfTheNextDirectoryRecord
	while (1) {
		if (it_next == dr->lowlevdir.end()) {
			it->second->ds->add_dataelement(0x00041400);
			break;
		} else {
			it->second->ds->add_dataelement(0x00041400)
				->from_dataset(it_next->second->ds);
		}
		it++; it_next++;
	}

	for (it = dr->lowlevdir.begin(); it != dr->lowlevdir.end(); it++)
		dicomdir_link_datasets(it->second, seq);
}

void dicomdir::build_dicomfile()
{
	if (df) delete df;
	df = new dicomfile();

	// (0004,1220) Directory Record Sequence
	sequence *seq = df->add_dataelement(0x00041220)->to_sequence();
	// link datasets
	dicomdir_link_datasets(root_dirrec, seq);

	// (0004,1200) OffsetOfTheFirstDirectoryRecordOfTheRootDirectoryEntity
	std::map<std::string, dirrec_t *>::iterator it;
	it = root_dirrec->lowlevdir.begin();
	if (it != root_dirrec->lowlevdir.end())
		df->add_dataelement(0x00041200)->from_dataset(it->second->ds);
	else
		df->add_dataelement(0x00041200);
	// (0004,1202) OffsetOfTheLastDirectoryRecordOfTheRootDirectoryEntity
	std::map<std::string, dirrec_t *>::reverse_iterator rit;
	rit = root_dirrec->lowlevdir.rbegin();
	if (rit != root_dirrec->lowlevdir.rend())
		df->add_dataelement(0x00041202)->from_dataset(rit->second->ds);
	else
		df->add_dataelement(0x00041202);

	// (0004,1130) File-set ID
	df->add_dataelement(0x00041130)->from_string(DICOMSDL_FILESETID);
	// (0004,1212) File-set Consistency Flag
	df->add_dataelement(0x00041212)->from_int(0);

	// take an instance and generate dicomdir's instance uid base on that
	std::string instuid;
	for (int i = seq->size()-1; i >= 0; i--) {
		dataelement *e = seq->dataset_at(i)->get_dataelement(0x00041511);
		if (e->is_valid()) {
			instuid = e->to_string();
			break;
		}
	}
	if (instuid.size() != 0)
		instuid = gen_uid((char *)(instuid.c_str()));
	else
		instuid = gen_uid();

	// set dicomfile's metainfo
	df->set_filemetainfo(
			dicom::UID_MEDIA_STORAGE_DIRECTORY_STORAGE,
			(char *)(instuid.c_str()),
			dicom::UID_EXPLICIT_VR_LITTLE_ENDIAN);
	df->remove_dataelement(0x00080016);
	df->remove_dataelement(0x00080018);
}

void dicomdir::save_to_file(const char *filename, opttype opt)
{
	build_dicomfile();
	df->save_to_file(filename, opt);
}

void dicomdir::save_to_memory_a
		(char **val_a, int *len_a, opttype opt)
{
	build_dicomfile();
	df->save_to_memory_a(val_a, len_a, opt);
}


// misc ------------------------------------------------------------------

void dicomdir::_dump(dirrec_t* r, std::string key, std::string prefix) {
	printf("%sTYPE [%s]\n", prefix.c_str(), r->dirrec_type);
	if (r->ds)
		printf("%s", r->ds->dump_string(prefix+"  ").c_str());

	std::map<std::string, dirrec_t *>::iterator it;
	for (it = r->lowlevdir.begin(); it != r->lowlevdir.end(); it++) {
		_dump(it->second, it->first, prefix+"  ");
	}
}
void dicomdir::dump() { _dump(root_dirrec); };

} // namespace dicom -----------------------------------------------------
