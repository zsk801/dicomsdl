/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#ifndef __DICOM_H__
#define __DICOM_H__

#include "dicomcfg.h"
#include <map>
#include <set>
#include <vector>
#include <string>

namespace dicom { //------------------------------------------------------

struct dicomfile;
struct dataset;
struct dataelement;
struct sequence;
class dataelement_iterator;

struct dicomdir;

/* -----------------------------------------------------------------------
 * define types
 */

#define DICOMSDL_UIDPREFIX "1.2.826.0.1.3680043.8.417.1"
#define DICOMSDL_IMPLCLASSUID DICOMSDL_UIDPREFIX".1"
#define DICOMSDL_IMPLVERNAME "DICOMSDL 2010SEP"
#define DICOMSDL_FILESETID "DICOMDIR"

typedef uint32 tagtype;
typedef uint16 vrtype;

#define make_tag(gggg,eeee)	(((gggg)<< 16) + (eeee))
#define group_in_tag(tag)	((uint16)((tag) >> 16))
#define element_in_tag(tag)	((uint16)((tag) & 0xffff))
#define make_even(x)		((x)+((x)&1))

const vrtype VR_NULL = 0x0000; /* NULL tag */
const vrtype VR_AE = 0x4541;
const vrtype VR_AS = 0x5341;
const vrtype VR_AT = 0x5441;
const vrtype VR_CS = 0x5343;
const vrtype VR_DA = 0x4144;
const vrtype VR_DS = 0x5344;
const vrtype VR_DT = 0x5444;
const vrtype VR_FD = 0x4446;
const vrtype VR_FL = 0x4c46;
const vrtype VR_IS = 0x5349;
const vrtype VR_LO = 0x4f4c;
const vrtype VR_LT = 0x544c;
const vrtype VR_OB = 0x424f;
const vrtype VR_OF = 0x464f;
const vrtype VR_OW = 0x574f;
const vrtype VR_PN = 0x4e50;
const vrtype VR_SH = 0x4853;
const vrtype VR_SL = 0x4c53;
const vrtype VR_SQ = 0x5153;
const vrtype VR_SS = 0x5353;
const vrtype VR_ST = 0x5453;
const vrtype VR_TM = 0x4d54;
const vrtype VR_UI = 0x4955;
const vrtype VR_UL = 0x4c55;
const vrtype VR_UN = 0x4e55;
const vrtype VR_US = 0x5355;
const vrtype VR_UT = 0x5455;

/* VR_UK; strange VR which found in file from RAYPAX
 * ; probably contains short string --; */
const vrtype VR_UK = 0x4b55;
/* VR_PIXSEQ is a custom VR only used in this program;
 * for ENCODED PIXEL DATA
 * i.e. in case of 0x7fe00010; VR_OB; len = -1 */
const vrtype VR_PIXSEQ = 0x5850;
/* VR_OFFSET is custom VR in this DICOM implementation.
 * An element with VR_OFFSET contains offset of a directory record.
 * Any elements with (0004;1200); (0004;1202); (0004;1400); (0004;1420) will
 * have this VR. */
const vrtype VR_OFFSET = 0x534f;

typedef enum {
	DICOM_OK = 0,
	DICOM_DEFLATED_FILEIMAGE = 1,

	DICOM_ERROR = -1,
	DICOM_MEMORY_ERROR = -2,
	DICOM_FILE_ERROR = -3,
	DICOM_INSTREAM_ERROR = -4,
	DICOM_INFLATE_ERROR = -5,
	DICOM_DEFLATE_ERROR = -6,
	DICOM_IMAGE_ERROR = -7,
	DICOM_ILLEGAL_INDEX_ERROR = -8,
	DICOM_UNSUPPORTED_TRANSFERSYNTAX_ERROR = -9,
	DICOM_ILLEGAL_ARGUMENT_ERROR = -10,
	DICOM_ENCODE_ERROR = -11,
	DICOM_DECODE_ERROR = -12,
	DICOM_WRITE_ERROR = -13,
	DICOM_INTERNAL_ERROR = -99
} errtype;


/** options for loading or saving a dicom file object */
typedef uint32 opttype;
typedef uint32 optarg;

const opttype OPT_DEFAULT						= 0x0000;

/** Load a fore pat of a DICOM file
 *
 * Default, Read the part of a DICOM file, before the given <i>tag</i>.
 * <i>tag</i> value should be given as additional parameters for
 * open_dicomfile. By skipping the latter portion of a file,
 * loading a large numbers of DICOM file will be much faster. Following
 * code will skip all data elements with tags after
 * <code>(0054,0040)</code>.

\verbatim
dicomfile *df =
  open_dicomfile("somefile.dcm", OPT_READ_PARTIAL_FILE, 0x00540400);
\endverbatim
\sa open_dicomfile(), open_dicomfile_from_memory()
 */
const opttype OPT_LOAD_PARTIAL_FILE				= 0x0001;

/** Continue load a dicom file on error
 * <code>open_dicomfile()</code> and
 * <code>open_dicomfile_from_memory()</code> continue reading on error
 * and returns partially loaded dicomfile object, rather than returns
 * <code>NULL</code>. get_error_message() is needed to check
 * if error has been occurred.

\verbatim
dicomfile *df = open_dicomfile(filename, OPT_LOAD_CONTINUE_ON_ERROR);
if (get_error_message()) {
	printf("There was error while reading '%s'; %s",
	filename, get_error_message());
}
\endverbatim
\sa open_dicomfile(), open_dicomfile_from_memory()
 */
const opttype OPT_LOAD_CONTINUE_ON_ERROR		= 0x0002;

/** Do not copy file image
 *
 * Only for <code>open_dicomfile_from_memory()</code>.
 * When <code>open_dicomfile_from_memory()</code> load a dicomfile object
 * from DICOM file image, it copies whole image and keeps it internally.
 * Whenever you access data element's value, accessing function will
 * reference internally kept DICOM file image.
 * When <code>OPT_LOAD_DONOT_COPY_DATA</code> option is given,
 * <code>open_dicomfile_from_memory()</code> will not copy the image. That
 * means if you try access data element's value after destroy original
 * DICOM file image, application will crash. So, if you use an option
 * <code>OPT_LOAD_DONOT_COPY_DATA</code> original file image should be
 * maintained until dicomfile object is deleted.

\verbatim
char *fileimage;
// .. loading fileimage from somewhere
dicomfile *df =
	open_dicomfile_from_memory(fileimage, OPT_LOAD_DONOT_COPY_DATA);
delete fileimage; // deleting original file image
df->get_dataelement("PatientName")->to_string(); // CRASH!!!
\endverbatim
\sa open_dicomfile_from_memory(), open_dicomdir_from_memory()
 */
const opttype OPT_LOAD_DONOT_COPY_DATA			= 0x0004;

const opttype OPT_SAVE_WITHOUT_PREAMBLE			= 0x0100;
const opttype OPT_SAVE_WITHOUT_METAINFO			= 0x0200;
const opttype OPT_SAVE_IMPLICIT_SQ_LENGTH		= 0x0400;
const opttype OPT_SAVE_IMPLICIT_DATASET_LENGTH	= 0x0800;
const opttype OPT_SAVE_CALC_GROUPLENGTH			= 0x1000;
const opttype OPT_SAVE_BASIC_OFFSET_TABLE		= 0x2000;

static const opttype default_load_opt = OPT_DEFAULT
		//| OPT_LOAD_PARTIAL_FILE
		//| OPT_LOAD_CONTINUE_ON_ERROR
		//| OPT_LOAD_DONOT_COPY_DATA
		;

static const opttype default_save_opt = OPT_DEFAULT
		//| OPT_SAVE_WITHOUT_PREAMBLE
		//| OPT_SAVE_WITHOUT_METAINFO
		//| OPT_SAVE_IMPLICIT_SQ_LENGTH
		//| OPT_SAVE_IMPLICIT_DATASET_LENGTH
		//| OPT_SAVE_CALC_GROUPLENGTH
		//| OPT_SAVE_BASIC_OFFSET_TABLE
		;


/* COPY DICOMDICT.HXX INTO HERE vvvv ---------------------------------- */

/* -----------------------------------------------------------------------
 *
 * Converted from Part 7: Message Exchange (2009)
 * Annex E Command Dictionary, E.1 REGISTRY OF DICOM COMMAND ELEMENTS
 *
 */

typedef enum {
	C_STORE_RQ 	= 0x0001,
	C_STORE_RSP 	= 0x8001,
	C_GET_RQ 	= 0x0010,
	C_GET_RSP 	= 0x8010,
	C_FIND_RQ 	= 0x0020,
	C_FIND_RSP 	= 0x8020,
	C_MOVE_RQ 	= 0x0021,
	C_MOVE_RSP 	= 0x8021,
	C_ECHO_RQ 	= 0x0030,
	C_ECHO_RSP 	= 0x8030,
	N_EVENT_REPORT_RQ 	= 0x0100,
	N_EVENT_REPORT_RSP 	= 0x8100,
	N_GET_RQ 	= 0x0110,
	N_GET_RSP 	= 0x8110,
	N_SET_RQ 	= 0x0120,
	N_SET_RSP 	= 0x8120,
	N_ACTION_RQ 	= 0x0130,
	N_ACTION_RSP 	= 0x8130,
	N_CREATE_RQ 	= 0x0140,
	N_CREATE_RSP 	= 0x8140,
	N_DELETE_RQ 	= 0x0150,
	N_DELETE_RSP 	= 0x8150,
	C_CANCEL_RQ 	= 0x0FFF,
} commandtype;


/* -----------------------------------------------------------------------
 *
 * Converted from DICOM Part 6: Data Dictionary (2009)
 *
 * Python codelet that converts uid name into 'uid variable name'.
 *
    uid = uid_name.upper().replace(' & ', 'AND')
    uid = re.sub('['+re.escape(',-@/()')+']', ' ', uid)
    uid = uid.split(':')[0]
    uid = uid.split('[')[0]
    uid = '_'.join(uid.split())
    uid = 'UID_'+uid
    if 'RETIRED' in uid: uid = None
 */

typedef enum {
	UID_UNKNOWN = 0,
	UID_IMPLICIT_VR_LITTLE_ENDIAN,
	UID_EXPLICIT_VR_LITTLE_ENDIAN,
	UID_DEFLATED_EXPLICIT_VR_LITTLE_ENDIAN,
	UID_EXPLICIT_VR_BIG_ENDIAN,
	UID_JPEG_BASELINE_PROCESS_1,
	UID_JPEG_EXTENDED_PROCESS_2AND4,
	UID_JPEG_LOSSLESS_NON_HIERARCHICAL_PROCESS_14,
	UID_JPEG_LOSSLESS_NON_HIERARCHICAL_FIRST_ORDER_PREDICTION_PROCESS_14,
	UID_JPEG_LS_LOSSLESS_IMAGE_COMPRESSION,
	UID_JPEG_LS_LOSSY_NEAR_LOSSLESS_IMAGE_COMPRESSION,
	UID_JPEG_2000_IMAGE_COMPRESSION_LOSSLESS_ONLY,
	UID_JPEG_2000_IMAGE_COMPRESSION,
	UID_JPEG_2000_PART_2_MULTI_COMPONENT_IMAGE_COMPRESSION_LOSSLESS_ONLY,
	UID_JPEG_2000_PART_2_MULTI_COMPONENT_IMAGE_COMPRESSION,
	UID_JPIP_REFERENCED,
	UID_JPIP_REFERENCED_DEFLATE,
	UID_MPEG2_MAIN_PROFILE_MAIN_LEVEL,
	UID_MPEG2_MAIN_PROFILE_HIGH_LEVEL,
	UID_RLE_LOSSLESS,
	UID_RFC_2557_MIME_ENCAPSULATION,
	UID_XML_ENCODING,
	UID_COLOR_PALETTE_STORAGE,
	UID_VERIFICATION_SOP_CLASS,
	UID_MEDIA_STORAGE_DIRECTORY_STORAGE,
	UID_STORAGE_COMMITMENT_PUSH_MODEL_SOP_CLASS,
	UID_PROCEDURAL_EVENT_LOGGING_SOP_CLASS,
	UID_SUBSTANCE_ADMINISTRATION_LOGGING_SOP_CLASS,
	UID_MODALITY_PERFORMED_PROCEDURE_STEP_SOP_CLASS,
	UID_MODALITY_PERFORMED_PROCEDURE_STEP_RETRIEVE_SOP_CLASS,
	UID_MODALITY_PERFORMED_PROCEDURE_STEP_NOTIFICATION_SOP_CLASS,
	UID_BASIC_FILM_SESSION_SOP_CLASS,
	UID_BASIC_FILM_BOX_SOP_CLASS,
	UID_BASIC_GRAYSCALE_IMAGE_BOX_SOP_CLASS,
	UID_BASIC_COLOR_IMAGE_BOX_SOP_CLASS,
	UID_PRINT_JOB_SOP_CLASS,
	UID_BASIC_ANNOTATION_BOX_SOP_CLASS,
	UID_PRINTER_SOP_CLASS,
	UID_PRINTER_CONFIGURATION_RETRIEVAL_SOP_CLASS,
	UID_VOI_LUT_BOX_SOP_CLASS,
	UID_PRESENTATION_LUT_SOP_CLASS,
	UID_MEDIA_CREATION_MANAGEMENT_SOP_CLASS_UID,
	UID_COMPUTED_RADIOGRAPHY_IMAGE_STORAGE,
	UID_DIGITAL_X_RAY_IMAGE_STORAGE_FOR_PRESENTATION,
	UID_DIGITAL_X_RAY_IMAGE_STORAGE_FOR_PROCESSING,
	UID_DIGITAL_MAMMOGRAPHY_X_RAY_IMAGE_STORAGE_FOR_PRESENTATION,
	UID_DIGITAL_MAMMOGRAPHY_X_RAY_IMAGE_STORAGE_FOR_PROCESSING,
	UID_DIGITAL_INTRA_ORAL_X_RAY_IMAGE_STORAGE_FOR_PRESENTATION,
	UID_DIGITAL_INTRA_ORAL_X_RAY_IMAGE_STORAGE_FOR_PROCESSING,
	UID_CT_IMAGE_STORAGE,
	UID_ENHANCED_CT_IMAGE_STORAGE,
	UID_ULTRASOUND_MULTI_FRAME_IMAGE_STORAGE,
	UID_MR_IMAGE_STORAGE,
	UID_ENHANCED_MR_IMAGE_STORAGE,
	UID_MR_SPECTROSCOPY_STORAGE,
	UID_ENHANCED_MR_COLOR_IMAGE_STORAGE,
	UID_ULTRASOUND_IMAGE_STORAGE,
	UID_ENHANCED_US_VOLUME_STORAGE,
	UID_SECONDARY_CAPTURE_IMAGE_STORAGE,
	UID_MULTI_FRAME_SINGLE_BIT_SECONDARY_CAPTURE_IMAGE_STORAGE,
	UID_MULTI_FRAME_GRAYSCALE_BYTE_SECONDARY_CAPTURE_IMAGE_STORAGE,
	UID_MULTI_FRAME_GRAYSCALE_WORD_SECONDARY_CAPTURE_IMAGE_STORAGE,
	UID_MULTI_FRAME_TRUE_COLOR_SECONDARY_CAPTURE_IMAGE_STORAGE,
	UID_12_LEAD_ECG_WAVEFORM_STORAGE,
	UID_GENERAL_ECG_WAVEFORM_STORAGE,
	UID_AMBULATORY_ECG_WAVEFORM_STORAGE,
	UID_HEMODYNAMIC_WAVEFORM_STORAGE,
	UID_CARDIAC_ELECTROPHYSIOLOGY_WAVEFORM_STORAGE,
	UID_BASIC_VOICE_AUDIO_WAVEFORM_STORAGE,
	UID_GENERAL_AUDIO_WAVEFORM_STORAGE,
	UID_ARTERIAL_PULSE_WAVEFORM_STORAGE,
	UID_RESPIRATORY_WAVEFORM_STORAGE,
	UID_GRAYSCALE_SOFTCOPY_PRESENTATION_STATE_STORAGE_SOP_CLASS,
	UID_COLOR_SOFTCOPY_PRESENTATION_STATE_STORAGE_SOP_CLASS,
	UID_PSEUDO_COLOR_SOFTCOPY_PRESENTATION_STATE_STORAGE_SOP_CLASS,
	UID_BLENDING_SOFTCOPY_PRESENTATION_STATE_STORAGE_SOP_CLASS,
	UID_XA_XRF_GRAYSCALE_SOFTCOPY_PRESENTATION_STATE_STORAGE,
	UID_X_RAY_ANGIOGRAPHIC_IMAGE_STORAGE,
	UID_ENHANCED_XA_IMAGE_STORAGE,
	UID_X_RAY_RADIOFLUOROSCOPIC_IMAGE_STORAGE,
	UID_ENHANCED_XRF_IMAGE_STORAGE,
	UID_X_RAY_3D_ANGIOGRAPHIC_IMAGE_STORAGE,
	UID_X_RAY_3D_CRANIOFACIAL_IMAGE_STORAGE,
	UID_BREAST_TOMOSYNTHESIS_IMAGE_STORAGE,
	UID_NUCLEAR_MEDICINE_IMAGE_STORAGE,
	UID_RAW_DATA_STORAGE,
	UID_SPATIAL_REGISTRATION_STORAGE,
	UID_SPATIAL_FIDUCIALS_STORAGE,
	UID_DEFORMABLE_SPATIAL_REGISTRATION_STORAGE,
	UID_SEGMENTATION_STORAGE,
	UID_SURFACE_SEGMENTATION_STORAGE,
	UID_REAL_WORLD_VALUE_MAPPING_STORAGE,
	UID_VL_ENDOSCOPIC_IMAGE_STORAGE,
	UID_VIDEO_ENDOSCOPIC_IMAGE_STORAGE,
	UID_VL_MICROSCOPIC_IMAGE_STORAGE,
	UID_VIDEO_MICROSCOPIC_IMAGE_STORAGE,
	UID_VL_SLIDE_COORDINATES_MICROSCOPIC_IMAGE_STORAGE,
	UID_VL_PHOTOGRAPHIC_IMAGE_STORAGE,
	UID_VIDEO_PHOTOGRAPHIC_IMAGE_STORAGE,
	UID_OPHTHALMIC_PHOTOGRAPHY_8_BIT_IMAGE_STORAGE,
	UID_OPHTHALMIC_PHOTOGRAPHY_16_BIT_IMAGE_STORAGE,
	UID_STEREOMETRIC_RELATIONSHIP_STORAGE,
	UID_OPHTHALMIC_TOMOGRAPHY_IMAGE_STORAGE,
	UID_LENSOMETRY_MEASUREMENTS_STORAGE,
	UID_AUTOREFRACTION_MEASUREMENTS_STORAGE,
	UID_KERATOMETRY_MEASUREMENTS_STORAGE,
	UID_SUBJECTIVE_REFRACTION_MEASUREMENTS_STORAGE,
	UID_VISUAL_ACUITY_MEASUREMENTS,
	UID_SPECTACLE_PRESCRIPTION_REPORTS_STORAGE,
	UID_MACULAR_GRID_THICKNESS_AND_VOLUME_REPORT_STORAGE,
	UID_BASIC_TEXT_SR_STORAGE,
	UID_ENHANCED_SR_STORAGE,
	UID_COMPREHENSIVE_SR_STORAGE,
	UID_PROCEDURE_LOG_STORAGE,
	UID_MAMMOGRAPHY_CAD_SR_STORAGE,
	UID_KEY_OBJECT_SELECTION_DOCUMENT_STORAGE,
	UID_CHEST_CAD_SR_STORAGE,
	UID_X_RAY_RADIATION_DOSE_SR_STORAGE,
	UID_COLON_CAD_SR_STORAGE,
	UID_ENCAPSULATED_PDF_STORAGE,
	UID_ENCAPSULATED_CDA_STORAGE,
	UID_POSITRON_EMISSION_TOMOGRAPHY_IMAGE_STORAGE,
	UID_ENHANCED_PET_IMAGE_STORAGE,
	UID_BASIC_STRUCTURED_DISPLAY_STORAGE,
	UID_RT_IMAGE_STORAGE,
	UID_RT_DOSE_STORAGE,
	UID_RT_STRUCTURE_SET_STORAGE,
	UID_RT_BEAMS_TREATMENT_RECORD_STORAGE,
	UID_RT_PLAN_STORAGE,
	UID_RT_BRACHY_TREATMENT_RECORD_STORAGE,
	UID_RT_TREATMENT_SUMMARY_RECORD_STORAGE,
	UID_RT_ION_PLAN_STORAGE,
	UID_RT_ION_BEAMS_TREATMENT_RECORD_STORAGE,
	UID_PATIENT_ROOT_QUERY_RETRIEVE_INFORMATION_MODEL_FIND,
	UID_PATIENT_ROOT_QUERY_RETRIEVE_INFORMATION_MODEL_MOVE,
	UID_PATIENT_ROOT_QUERY_RETRIEVE_INFORMATION_MODEL_GET,
	UID_STUDY_ROOT_QUERY_RETRIEVE_INFORMATION_MODEL_FIND,
	UID_STUDY_ROOT_QUERY_RETRIEVE_INFORMATION_MODEL_MOVE,
	UID_STUDY_ROOT_QUERY_RETRIEVE_INFORMATION_MODEL_GET,
	UID_COMPOSITE_INSTANCE_ROOT_RETRIEVE_MOVE,
	UID_COMPOSITE_INSTANCE_ROOT_RETRIEVE_GET,
	UID_COMPOSITE_INSTANCE_RETRIEVE_WITHOUT_BULK_DATA_GET,
	UID_MODALITY_WORKLIST_INFORMATION_MODEL_FIND,
	UID_GENERAL_PURPOSE_WORKLIST_INFORMATION_MODEL_FIND,
	UID_GENERAL_PURPOSE_SCHEDULED_PROCEDURE_STEP_SOP_CLASS,
	UID_GENERAL_PURPOSE_PERFORMED_PROCEDURE_STEP_SOP_CLASS,
	UID_INSTANCE_AVAILABILITY_NOTIFICATION_SOP_CLASS,
	UID_RT_BEAMS_DELIVERY_INSTRUCTION_STORAGE_SUPPLEMENT_74_FROZEN_DRAFT,
	UID_RT_CONVENTIONAL_MACHINE_VERIFICATION_SUPPLEMENT_74_FROZEN_DRAFT,
	UID_RT_ION_MACHINE_VERIFICATION_SUPPLEMENT_74_FROZEN_DRAFT,
	UID_UNIFIED_PROCEDURE_STEP_PUSH_SOP_CLASS,
	UID_UNIFIED_PROCEDURE_STEP_WATCH_SOP_CLASS,
	UID_UNIFIED_PROCEDURE_STEP_PULL_SOP_CLASS,
	UID_UNIFIED_PROCEDURE_STEP_EVENT_SOP_CLASS,
	UID_GENERAL_RELEVANT_PATIENT_INFORMATION_QUERY,
	UID_BREAST_IMAGING_RELEVANT_PATIENT_INFORMATION_QUERY,
	UID_CARDIAC_RELEVANT_PATIENT_INFORMATION_QUERY,
	UID_HANGING_PROTOCOL_STORAGE,
	UID_HANGING_PROTOCOL_INFORMATION_MODEL_FIND,
	UID_HANGING_PROTOCOL_INFORMATION_MODEL_MOVE,
	UID_HANGING_PROTOCOL_INFORMATION_MODEL_GET,
	UID_PRODUCT_CHARACTERISTICS_QUERY_SOP_CLASS,
	UID_SUBSTANCE_APPROVAL_QUERY_SOP_CLASS,
	UID_BASIC_GRAYSCALE_PRINT_MANAGEMENT_META_SOP_CLASS,
	UID_BASIC_COLOR_PRINT_MANAGEMENT_META_SOP_CLASS,
	UID_GENERAL_PURPOSE_WORKLIST_MANAGEMENT_META_SOP_CLASS,
	UID_COLOR_PALETTE_INFORMATION_MODEL_FIND,
	UID_COLOR_PALETTE_INFORMATION_MODEL_MOVE,
	UID_COLOR_PALETTE_INFORMATION_MODEL_GET,
	UID_DICOM_APPLICATION_CONTEXT_NAME,
	UID_DICOM_CONTROLLED_TERMINOLOGY,
	UID_DICOM_UID_REGISTRY,
	UID_STORAGE_SERVICE_CLASS,
	UID_UNIFIED_WORKLIST_AND_PROCEDURE_STEP_SERVICE_CLASS
} uidtype;

/* COPY DICOMDICT.HXX INTO HERE ^^^^ ---------------------------------- */


/* -----------------------------------------------------------------------
 * dataset
 */

typedef std::map <tagtype, dataelement *> element_dict_type;

/*! Implementation of DICOM data set */
struct DLLEXPORT dataset {
	element_dict_type edict;		// element dictionary
	uidtype tsuid;		// transfer syntax for this dataset

	// load & save -------------------------------------------------------

	// return DICOM_OK or DICOM_DEFLATED_FILEIMAGE
	// throw error string on error
	int load(
		void *instream, dicomfile* dfobj, uidtype tsuid,
		opttype opt=default_load_opt, optarg arg=0);

	int save_a(char **val_a, int *len_a, opttype opt=default_save_opt);
	int _save(void *ostream, uidtype tsuid, opttype opt);

	// get function ------------------------------------------------------
	dataelement* get_dataelement(tagtype tag);

	/* tagstr accept following forms
	     (gggg,eeee mean group and element number of tag in hex form)
	  "0xggggeeee", "ggggeeee"
	  "ggggeeee.0.ggggeeee", "KeyWord" */
	dataelement* get_dataelement(char *tagstr);

	dataelement& operator[] (tagtype tag)
		{ return *get_dataelement(tag); };
	dataelement& operator[] (char *tagstr)
		{ return *get_dataelement(tagstr); };

	// add function ------------------------------------------------------
	dataelement* add_dataelement(
			tagtype tag, vrtype vr=VR_NULL,
			uint32 len=0, void *ptr=NULL,
			int endian=MACHINE_ENDIANNESS, int own_memory=false);
	dataelement* add_dataelement(char *tagstring, vrtype vr=VR_NULL);

	// del function ------------------------------------------------------
	void remove_dataelement(tagtype tag);
	void remove_all_dataelements();

	~dataset() { remove_all_dataelements(); };

	void dump_string_a(char **val_a, int *len_a, std::string prefix="");
	std::string dump_string(std::string prefix="");

	// image related function --------------------------------------------
	void get_image_info(
			int *width, int *height,
			int *precision, int *signedness,
			int *ncomponents, int *bytes_per_pixel,
			int *nframes);
	void set_image_dimension(
			int width, int height,
			int precision, int signedness, int ncomponents);

	void get_pixeldata_a(char **val_a, int *len_a);
	int copy_pixeldata_to(char *buf, int rowstep, int framestep, int idx=-1);
	int set_pixeldata(uidtype tsuid, char *buf,
		int width, int height, int prec, int sgnd, int ncomps, int nframes,
		int rowstep, int framestep, int quality=0);

	// misc functions ----------------------------------------------------

	int set_filemetainfo(
		uidtype sop_class_uid,
		char* sop_instance_uid,
		uidtype transfer_syntax_uid
	);

	int change_pixelencoding(uidtype transfer_syntax_uid, int quality=0);

	inline int number_of_elements()
		{ return edict.size(); };

	// change dataelement->ptr when instream->data has changed
	void realloc_ptr(long ptrdiff);

private:
	void _dump(std::iostream *os, std::string prefix);
};

/* -----------------------------------------------------------------------
 * dicomfile
 */

/*! Read and parse a DICOM file
 *
 * \param filename DICOM file name
 * \param opt option to read
 * \param arg arguments for an option
 * \return Return a dicom::dicomfile object. Return NULL on failure.
 * \sa open_dicomfile_from_memory(), close_dicomfile(),
 *     OPT_LOAD_CONTINUE_ON_ERROR, OPT_LOAD_PARTIAL_FILE
 */
DLLEXPORT dicomfile* open_dicomfile
			(char *filename,
			 opttype opt=default_load_opt, optarg arg=0);

/*! Read and parse a DICOM file from memory
 *
 * \param data image of a DICOM file
 * \param datasize length of data
 * \param opt option to read
 * \param arg arguments for an option
 * \return Return a dicom::dicomfile object. Return NULL on failure.
 * \sa open_dicomfile(), close_dicomfile()
 *     OPT_LOAD_CONTINUE_ON_ERROR, OPT_LOAD_DONOT_COPY_DATA,
 *     OPT_LOAD_PARTIAL_FILE
 */
DLLEXPORT dicomfile* open_dicomfile_from_memory
			(char *data, int datasize,
			 opttype opt=default_load_opt, optarg arg=0);

/*! Destroy a dicom::dicomfile object.
 *
 * \param df dicom::dicomfile object that is returned from
 * 			 open_dicomfile() or open_dicomfile_from_memory()
 */
DLLEXPORT void close_dicomfile(dicomfile *df);

struct DLLEXPORT dicomfile: public dataset {
	void *stream;

	// read dicom file ---------------------------------------------------

	int load_from_file(char *filename,
			opttype opt=default_load_opt, optarg arg=0);
	int load_from_data
			(char *data, int datasize,
			opttype opt=default_load_opt, optarg arg=0);

	int save_to_file(char *filename, opttype opt=default_save_opt);
	int save_to_memory_a
			(char **val_a, int *len_a, opttype opt=default_save_opt);

	// track the positions of dataset within dicomfile image -------------
	std::map<uint32, dataset*> dataset_pos_list;
	void mark_dataset_offset(uint32 offset, dataset *ds);
	dataset* dataset_at(uint32 offset);

	// ctor & dtor -------------------------------------------------------
	// dicomfile("filename.dcm"); // read from file
	// dicomfile(data, datasize); // read from buffer
	// throw error string on error
	dicomfile(char *data, int datasize=-1);

	dicomfile() { stream = NULL; };
	~dicomfile();

	// return associated filename
	char* get_filename();

private:
	void _read_from_stream(opttype opt, optarg arg);
};

/* -----------------------------------------------------------------------
 * dataelement
 */

struct DLLEXPORT dataelement {
	tagtype	tag;	// DICOM data element's tag
	vrtype	vr;		// DICOM data element's VR
	uint32	len;	// length of DICOM data element's value
	void*	ptr;	// pointer to value
	int		endian;		// LITTLE_ENDIAN or BIG_ENDIAN
	int		own_memory;	// if true, ~dataelement() does free(ptr)

	uint8* get_ptr() { return (uint8 *)ptr; };

	// check validity ----------------------------------------------------

	inline int is_valid()	{ return (vr != VR_NULL); };
	inline operator bool()	{ return (vr != VR_NULL); };

	// get int type value(s) ---------------------------------------------

	int to_int(int defaultvalue=0);
	inline operator int () { return to_int(); };

	void to_int_values_a(int **values, int *nvalues);
	std::vector<int> to_int_values();
	operator std::vector<int> () { return to_int_values(); };

	// get double type value(s) ------------------------------------------

	double to_double(double defaultvalue=0.0);
	inline operator double() { return to_double(); };

	void to_double_values_a(double **values, int *nvalues);
	std::vector <double> to_double_values();
	operator std::vector <double> () { return to_double_values(); };

	// get string type value ---------------------------------------------

	void to_string_a(char **val_a, int *len_a=NULL);
	std::string to_string(const char* defaultvalue="");
	inline operator std::string () { return to_string(); };

	// get raw value ----------------------------------------------------

	void raw_value(char **val, int *len);

	// get a representative string of value --------------------------------

	void repr_string_a(char **val_a, int *len_a, int max_len=80);
	std::string repr_string(int max_len=80);

	// get the uid type value if element's vr is VR_UI -----------------------

	uidtype to_uid();
	inline operator uidtype () { return to_uid(); };

	// set int type value(s) ---------------------------------------------

	void from_int(int value);
	void from_int_values(int *values, int nvalues);

	inline void set_value(int value)
		{ from_int(value); };
	inline void set_value(int *values, int nvalues)
		{ from_int_values(values, nvalues); };
	void set_value(std::vector <int> & v);

	inline int operator=(int value)
		{ from_int(value); return value; };
	inline std::vector <int> & operator=(std::vector <int> & v)
		{ set_value(v); return v; };

	// set double type value(s) ------------------------------------------

	void from_double(double value);
	void from_double_values(double *values, int nvalues);

	inline void set_value(double value)
		{ from_double(value); };
	inline void set_value(double *values, int nvalues)
		{ from_double_values(values, nvalues); };
	void set_value(std::vector <double> & v);

	inline double operator=(double v)
		{ from_double(v); return v; };
	inline std::vector <double> & operator=(std::vector <double> & v)
		{ set_value(v); return v; };

	// direct access function to sequence, recordoffset, ...

	int number_of_datasets();
	dataset* dataset_at(int idx);
	dataset* add_dataset();
	sequence* to_sequence()
		{ if (vr == VR_SQ && ptr) return (sequence *)ptr;
		  else return NULL; };
	inline operator sequence* () { return to_sequence(); };

	int number_of_frames();

	void from_dataset(dataset *);
	dataset* to_dataset();
	inline operator dataset* () { return to_dataset(); };

	// set string type value(s) ------------------------------------------

	void from_string(const char *str);
	void from_string(std::string &s)
		{ from_string((char *)(s.c_str())); };

	inline void set_value(char *str)
		{ from_string(str); };
	inline void set_value(std::string &s)
		{ from_string((char *)(s.c_str())); };

	inline char* operator=(char * v)
		{ from_string(v); return v; };
	inline std::string operator=(std::string &s)
		{ from_string((char *)(s.c_str())); return s; };

	// set raw type value(s) ---------------------------------------------

	void from_data(const char *data, int datasize);

	inline void set_value(const char *data, int datasize)
		{ from_data(data, datasize); };

	// -------------------------------------------------------------------

	void set_endian(int endian=MACHINE_ENDIANNESS);
	int get_vm();

	// -------------------------------------------------------------------
	int alloc_memory(int len); // return if alloc memory successfully
	void free_value();
	~dataelement() { free_value(); };

	void _save(void *ostream, uidtype tsuid, opttype opt);
};

DLLEXPORT dataelement* nullelement();

class DLLEXPORT dataelement_iterator
{
public:
	dataset* ds;
	element_dict_type::iterator it;

	dataelement_iterator(dataset *ds);
	dataelement* next();
};

/* -----------------------------------------------------------------------
 * dataset sequence
 */

struct DLLEXPORT sequence {
	std::vector <dataset *> seq;

	int load(void *instream, dicomfile* dfobj, uidtype tsuid);
	int size() { return (int) seq.size(); };

	dataset* dataset_at(int idx) {
		return (idx < size() ? seq[idx] : NULL);
	}
	dataset* operator[] (int idx)
		{ return dataset_at(idx); };

	dataset* add_dataset();
	void add_dataset(dataset *ds);
	void remove_dataset(int idx, bool delete_dataset=true);
	void remove_dataset(dataset *ds, bool delete_dataset=true);
	void remove_all_datasets(bool delete_dataset=true);

	void _save(void *ostream, uidtype tsuid, opttype opt);

	~sequence() { remove_all_datasets(); };

};


/* -----------------------------------------------------------------------
 * struct DICOMDIR
 */

DLLEXPORT dicomdir* open_dicomdir
				(char *filename, opttype opt=default_load_opt);
DLLEXPORT dicomdir* open_dicomdir_from_memory
				(char *data, int datasize, opttype opt=default_load_opt);
DLLEXPORT void close_dicomdir(dicomdir *df);

// implementing a directory record
// see PS 3.3 - 2009
// F.3 BASIC DIRECTORY INFORMATION OBJECT DEFINITION

struct dirrec_t {
	// directory record type
	char dirrec_type[20];

	// low level directory records
	std::map<std::string, dirrec_t *> lowlevdir;

	// dataset containing keys for this directory record
	dataset *ds;

	// if own_dataset is true, ~dirrec_t() should delete dataset
	bool own_dataset;

	void set_dirrec_type(const char *s);
	dirrec_t();
	~dirrec_t();
};

// implementing a DICOMDIR file

struct DLLEXPORT dicomdir {
	dicomfile *df;
	dirrec_t *root_dirrec;

	int load_from_file
			(char *filename, opttype opt=default_load_opt);
	int load_from_data
			(char *data, int datasize, opttype opt=default_load_opt);

	void save_to_file(char *filename, opttype opt=default_save_opt);
	void save_to_memory_a
			(char **val_a, int *len_a, opttype opt=default_save_opt);

	dicomdir();
	dicomdir(char *data, int datasize=-1);
	~dicomdir();

	int add_dicomfile(dicomfile *df, char *ref_file_id);
	int add_dicomfile(char *filename, char *ref_file_id);

	void build_dicomfile();

	void _dump(dirrec_t* r, std::string key="", std::string prefix="");
	void dump();

protected:
	int add_dicomfile_image_type(dicomfile *df, char *ref_file_id);
	void analyze_directory_records(dirrec_t* base_dir, dataset *ds);
	void make_root_dirrec_own_dataset(dirrec_t* r=NULL,
			std::set<dataset *> *sds = (std::set<dataset *> *)NULL);
};

// functions managing dataelement's tag list to be stored in DICOMDIR file
DLLEXPORT void add_tags_for_dicomdir(char **drtype_taglist);
DLLEXPORT void reset_tags_for_dicomdir();


/* -----------------------------------------------------------------------
 * misc functions
 */

DLLEXPORT vrtype get_tag_vr (tagtype tag);
DLLEXPORT const char* get_tag_name (tagtype tag);
DLLEXPORT const char* get_tag_keyword (tagtype tag);
DLLEXPORT tagtype find_tag(char *keyword);

DLLEXPORT uidtype uidvalue_to_uid (const char *uidvalue);
DLLEXPORT const char *uid_to_uidvalue (uidtype uid);
DLLEXPORT const char *uidvalue_to_uidname (const char *uidvalue);
DLLEXPORT const char *uid_to_uidname (uidtype uid);

DLLEXPORT const char *get_vr_repr (vrtype vr);

// logging debug, warning and error messages

typedef void (*logfunc)(char *msg);
DLLEXPORT void set_debug_logger(logfunc);
DLLEXPORT void set_warning_logger(logfunc);
DLLEXPORT void set_error_logger(logfunc);

DLLEXPORT void set_display_debug_message(int b);
DLLEXPORT void set_display_warning_message(int b);
DLLEXPORT void set_display_debug_message(int b);

// get error message

/*! Return error message
 * \return Return a string contains error message
 *         if previous operation had an error.
 *         Otherwise, return <code>NULL</code>.
 *
 */
DLLEXPORT char *get_error_message();

// select decoder / encoder for decoding / encoding pixel data

DLLEXPORT int use_decoder(uidtype tsuid, char *codec_name);
DLLEXPORT int use_encoder(uidtype tsuid, char *codec_name);

// support functions for zipped DICOM files

DLLEXPORT void test_unzip(char *filename);
DLLEXPORT std::string zipfile_get_list(char *filename);
DLLEXPORT void zipfile_extract_file_a(char *zipfn, char *fn,
		char **val_a, int *len_a);

// uid's length should not exceed 64 bytes
#define MAX_UID_LEN 64

DLLEXPORT std::string gen_uid(char* base_uid=NULL);
DLLEXPORT void set_uid_prefix(char *u=NULL);
DLLEXPORT int is_valid_uid(char *u);

} // namespace dicom -----------------------------------------------------

#endif //  __DICOM_H__
