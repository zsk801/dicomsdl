#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------------------

#define uint16	unsigned short

#define MAKETAG(gggg,eeee)	(((gggg)<< 16) + (eeee))
#define GROUPINTAG(tag)		((uint16)((tag) >> 16))
#define ELEMENTINTAG(tag)	((uint16)((tag) & 0xffff))
#define MAKE_EVEN(x)		((x)+((x)&1))


#define VR_NULL    0x0000

/* VR_PIXSEQ is a private VR used in this program for ENCODED PIXEL DATA
 * i.e. in case of 0x7fe00010, VR_OB, len = -1 */
#define VR_PIXSEQ  0x5058
#define VR_OFFSET  0x4f53

#define VR_AE    0x4145
#define VR_AS    0x4153
#define VR_AT    0x4154
#define VR_CS    0x4353
#define VR_DA    0x4441
#define VR_DS    0x4453
#define VR_DT    0x4454
#define VR_FD    0x4644
#define VR_FL    0x464c
#define VR_IS    0x4953
#define VR_LO    0x4c4f
#define VR_LT    0x4c54
#define VR_OB    0x4f42
#define VR_OF    0x4f46
#define VR_OW    0x4f57
#define VR_PN    0x504e
#define VR_SH    0x5348
#define VR_SL    0x534c
#define VR_SQ    0x5351
#define VR_SS    0x5353
#define VR_ST    0x5354
#define VR_TM    0x544d
#define VR_UI    0x5549
#define VR_UL    0x554c
#define VR_UN    0x554e
#define VR_US    0x5553
#define VR_UT    0x5554

/* VR_UK; strange VR which found in file from RAYPAX,
 * probably contains short string --; */
#define VR_UK    0x554b

#define TAGTYPE unsigned int
#define VRTYPE unsigned short

// -----------------------------------------------------------------------

struct _element_registry_struct_ {
    TAGTYPE tag;
    char *tag_str;
    char *name;
    char *keyword;
    VRTYPE vr;
    char *vr_str;
    char *vm;
    int if_retired;
};

struct _element_keyword_struct_ {
    TAGTYPE tag;
    char *keyword;
};

#include "dicomdict.hxx"
#include "dicomdict.cxx"

// -----------------------------------------------------------------------

/* get the VR of a Data Element with 'tag' */
VRTYPE	ts_get_tag_vr(TAGTYPE tag);

/* get the name of a Data Element with 'tag' */
char* ts_get_tag_name(TAGTYPE tag);

/* find Data Element Tag correspond to 'keyword' */
TAGTYPE	ts_find_tag(char *keyword);

/* get the short descriptive string for 'vr' */
char* ts_get_vr_repr (VRTYPE vr);

/*

#define uid_to_uidvalue
#define uidvalue_to_uidname
#define uidvalue_to_uid
*/
DICOMUID ts_uidvalue_to_uid(char *uidvalue);
char* ts_uid_to_uidvalue(DICOMUID uid);
char* ts_uidvalue_to_uidname(char *uidvalue);

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
	char *at, *bt;
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

struct _element_registry_struct_* _find_tag(TAGTYPE tag)
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
	sprintf(tagstr, "(%04x,%04x)", GROUPINTAG(tag), ELEMENTINTAG(tag));
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


VRTYPE ts_get_tag_vr(TAGTYPE tag)
{
	if (ELEMENTINTAG(tag) == 0) return VR_UL;

	struct _element_registry_struct_* result = _find_tag(tag);
	if (result)	return result->vr;

	return VR_NULL;
}

char* ts_get_tag_name(TAGTYPE tag)
{
	struct _element_registry_struct_* result = _find_tag(tag);
	if (result) return result->name;

	if (ELEMENTINTAG(tag) == 0)
		return "Group Length";
	if (GROUPINTAG(tag) & 1)
		return "Private Data Elements";
	return "Unknown Data Elements";
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

TAGTYPE ts_find_tag(char *keyword)
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

// vr repr string --------------------------------------------------------

char *ts_get_vr_repr (VRTYPE vr)
{
	switch (vr) {
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
	case VR_PIXSEQ:	return "PX";
	case VR_OFFSET: return "UL";
	case VR_NULL:
	default:
		return "NULL";
	}
}

// -----------

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

struct _uid_registry_struct_ *_find_uid(char *uidvalue)
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

DICOMUID ts_uidvalue_to_uid(char *uidvalue)
{
	return _find_uid(uidvalue)->uid;
}

char* ts_uid_to_uidvalue(DICOMUID uid)
{
	return (char *)uidvalue_registry[uid];
}

char* ts_uidvalue_to_uidname(char *uidvalue)
{
	return _find_uid(uidvalue)->uidname;
}


int main()
{
	puts(ts_get_vr_repr(ts_get_tag_vr(0x50ff2600)));
	puts(ts_get_tag_name(ts_find_tag("BrachyAccessoryDeviceSequence")));
	printf("%08X\n", ts_find_tag("BrachyAccessoryDeviceSequence"));
	printf(">>>>>>>%s\n", ts_uidvalue_to_uidname("1.2.840.10008.1.2.4.101"));
	puts(ts_uid_to_uidvalue(UID_UNIFIED_WORKLIST_AND_PROCEDURE_STEP_SERVICE_CLASS));
}
