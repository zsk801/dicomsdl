/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dicom.h"

namespace dicom { //------------------------------------------------------

struct _element_registry_struct_ {
    tagtype tag;
    const char *tag_str;
    const char *name;
    const char *keyword;
    vrtype vr;
    const char *vr_str;
    const char *vm;
    int if_retired;
};

struct _element_keyword_struct_ {
    tagtype tag;
    const char *keyword;
};

struct _uid_registry_struct_ {
    const char *uidvalue;
    uidtype uid;
    const char *uid_name;
    const char *uid_type;
    const char *uid_part;
};

#include "dicomdict.inc.cxx"


// element tag -> vr or name ---------------------------------------------

const int _sizeof_element_registry_struct_ =
		sizeof(element_registry)/sizeof(_element_registry_struct_);

int _compare_element_tag(
		struct _element_registry_struct_ *a,
		struct _element_registry_struct_ *b )
{
	if (a->tag > b->tag)		return 1;
	else if (a->tag == b->tag)	return 0;
	else						return -1;
}

int _compare_element_tag_xx(
		struct _element_registry_struct_ *a,
		struct _element_registry_struct_ *b )
{
	const char *at, *bt;
	at = a->tag_str;
	bt = b->tag_str;

	while (*at) {
		if (*bt ==  'x' || *at == *bt) {
			at++;
			bt++;
		} else {
			if (*at > *bt) return 1;
			if (*at < *bt) return -1;
		}
	}
	return 0;
}

struct _element_registry_struct_* _find_tag(tagtype tag)
{
	struct _element_registry_struct_ key, *result;

	// find from tag

	key.tag = tag;
	result = (struct _element_registry_struct_ *)
		bsearch(&key,
				element_registry,
				_sizeof_element_registry_struct_,
				sizeof(_element_registry_struct_),
		(int (*)(const void*, const void*))_compare_element_tag );

	if (result)
		return result;

	// check item such as (1000,xxx0)

	char tagstr[16];
	sprintf(tagstr, "(%04x,%04x)", group_in_tag(tag), element_in_tag(tag));
	key.tag_str = tagstr;

	result = (struct _element_registry_struct_ *)
		bsearch(&key,
				element_registry,
				_sizeof_element_registry_struct_,
				sizeof(_element_registry_struct_),
		(int (*)(const void*, const void*))_compare_element_tag_xx );

	if (result)
		return result;

	// can't find in registry

	return NULL;
}

DLLEXPORT vrtype get_tag_vr (tagtype tag)
{
	if (element_in_tag(tag) == 0) return VR_UL;

	struct _element_registry_struct_* result = _find_tag(tag);
	if (result)	return result->vr;

	return VR_NULL;
}

DLLEXPORT const char* get_tag_name (tagtype tag)
{
	struct _element_registry_struct_* result = _find_tag(tag);
	if (result) return result->name;

	if (element_in_tag(tag) == 0)
		return "Group Length";
	if (group_in_tag(tag) & 1)
		return "Private Data Elements";
	return "Unknown Data Elements";
}

DLLEXPORT const char* get_tag_keyword (tagtype tag)
{
	struct _element_registry_struct_* result = _find_tag(tag);
	if (result) return result->keyword;

	if (element_in_tag(tag) == 0)
		return "(Group Length)";
	if (group_in_tag(tag) & 1)
		return "(Private Data Elements)";
	return "(Unknown Data Elements)";
}

// keyword -> tag --------------------------------------------------------

int _compare_element_keyword(
		struct _element_keyword_struct_ *a,
		struct _element_keyword_struct_ *b )
{
	return strcmp(a->keyword, b->keyword);
}

const int _sizeof_element_keyword_struct_ =
		sizeof(element_keyword)/sizeof(_element_keyword_struct_);

DLLEXPORT tagtype find_tag(const char *keyword)
{
	struct _element_keyword_struct_ key, *result;

	key.keyword = keyword;
	result = (struct _element_keyword_struct_ *)
		bsearch(&key,
				element_keyword,
				_sizeof_element_keyword_struct_,
				sizeof(_element_keyword_struct_),
		(int (*)(const void*, const void*))_compare_element_keyword );

	if (result)
		return result->tag;

	return 0xFFFFFFFF;
}


// convert between uid, uid value and uid name ---------------------------

int _compare_uidvalue(
		struct _uid_registry_struct_ *a,
		struct _uid_registry_struct_ *b  )
{
	if (!a->uidvalue) return 1;
	if (!b->uidvalue) return -1;
	return strcmp(a->uidvalue, b->uidvalue);
}

const int _sizeof_uid_registry_ =
		sizeof(uid_registry)/sizeof(_uid_registry_struct_);

struct _uid_registry_struct_ *_find_uid(const char *uidvalue)
{
	if (!uidvalue || !(*uidvalue))
		return (_uid_registry_struct_ *)uid_registry;

	struct _uid_registry_struct_ key, *result;
	key.uidvalue = uidvalue;
	result = (struct _uid_registry_struct_ *)
		bsearch(&key, uid_registry, _sizeof_uid_registry_,
		sizeof(struct _uid_registry_struct_),
		(int (*)(const void*, const void*))_compare_uidvalue );
	if (result) return result;

	return (_uid_registry_struct_ *)uid_registry;
}

DLLEXPORT uidtype uidvalue_to_uid (const char *uidvalue)
{
	return _find_uid(uidvalue)->uid;
}

DLLEXPORT const char *uid_to_uidvalue (uidtype uid)
{
	return (const char *)uidvalue_registry[uid*2];
}

DLLEXPORT const char *uidvalue_to_uidname (const char *uidvalue)
{
	return _find_uid(uidvalue)->uid_name;
}

DLLEXPORT const char *uid_to_uidname (uidtype uid)
{
	return (const char *)uidvalue_registry[uid*2+1];
}


// vr repr string --------------------------------------------------------

DLLEXPORT const char *get_vr_repr (vrtype vr)
{
	switch (vr) {
	case VR_NULL:	return "NULL";
	case VR_AE:	return "AE";
	case VR_AS:	return "AS";
	case VR_AT:	return "AT";
	case VR_CS:	return "CS";
	case VR_DA:	return "DA";
	case VR_DS:	return "DS";
	case VR_DT:	return "DT";
	case VR_FD:	return "FD";
	case VR_FL:	return "FL";
	case VR_IS:	return "IS";
	case VR_LO:	return "LO";
	case VR_LT:	return "LT";
	case VR_OB:	return "OB";
	case VR_OF:	return "OF";
	case VR_OW:	return "OW";
	case VR_PN:	return "PN";
	case VR_SH:	return "SH";
	case VR_SL:	return "SL";
	case VR_SQ:	return "SQ";
	case VR_SS:	return "SS";
	case VR_ST:	return "ST";
	case VR_TM:	return "TM";
	case VR_UI:	return "UI";
	case VR_UL:	return "UL";
	case VR_UN:	return "UN";
	case VR_US:	return "US";
	case VR_UT:	return "UT";
	case VR_UK:	return "UK";
	case VR_PIXSEQ:	return "PIXSEQ";
	case VR_OFFSET: return "UL";
	default:
		return "NULL";
	}
}

} // namespace dicom -----------------------------------------------------
