/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include "dicom.h"

namespace dicom { //------------------------------------------------------

DLLEXPORT void test_unzip(const char *filename)
{

}

// 1 if a>b, 0 if a=b, -1 if a<b
DLLEXPORT int compare_dataelement(dataelement *de_a, dataelement *de_b)
{
	// check if both dataelement is valid
	if (!(*de_a && *de_b)) {
		if (*de_a) return 1;
		if (*de_b) return -1;
		return 0;
	}

	// compare only elements with same VR
	if (de_a->vr != de_b->vr)
		return 0;

	switch (de_a->vr) {
	case VR_SL:
	case VR_SS:
	case VR_UL:
	case VR_US:
	case VR_IS:
	case VR_AT:
		if (de_a->get_vm() == 1 && de_b->get_vm() == 1) {
			int a = de_a->to_int();
			int b = de_b->to_int();
			if (a>b) return 1;
			else if (a==b) return 0;
			else return -1;
		} else {
			int *a, alen, *b, blen, len;
			de_a->to_int_values_a(&a, &alen);
			de_b->to_int_values_a(&b, &blen);
			len = (alen>blen? blen:alen);
			for (int i = 0; i < len; i++) {
				if (a[i] > b[i]) { free(a); free(b); return 1; }
				if (a[i] < b[i]) { free(a); free(b); return -1; }
			}
			free(a);
			free(b);
			if (alen>blen) return 1;  // 1,2,3 > 1,2
			if (alen<blen) return -1; // 1,2 < 1,2,3
			return 0;
		}
		break;

	case VR_FL:
	case VR_FD:
	case VR_DS:
		if (de_a->get_vm() == 1 && de_b->get_vm() == 1) {
			double a = de_a->to_int();
			double b = de_b->to_int();
			if (a>b) return 1;
			else if (a==b) return 0;
			else return -1;
		} else {
			double *a, *b;
			int alen, blen, len;
			de_a->to_double_values_a(&a, &alen);
			de_b->to_double_values_a(&b, &blen);
			len = (alen>blen? blen:alen);
			for (int i = 0; i < len; i++) {
				if (a[i] > b[i]) { free(a); free(b); return 1; }
				if (a[i] < b[i]) { free(a); free(b); return -1; }
			}
			free(a);
			free(b);
			if (alen>blen) return 1;  // 1,2,3 > 1,2
			if (alen<blen) return -1; // 1,2 < 1,2,3
			return 0;
		}
		break;

	case VR_AE:
	case VR_AS:
	case VR_CS:
	case VR_DA:
	case VR_DT:
	case VR_LO:
	case VR_LT:
	case VR_PN:
	case VR_SH:
	case VR_ST:
	case VR_TM:
	case VR_UI:
		int ret, n;
		if (de_a->len > de_b->len)
			n = de_b->len;
		else
			n = de_a->len;
		ret = memcmp(de_a->ptr, de_b->ptr, n);
		if (ret == 0) {
			if (de_a->len > de_b->len)
				ret = 1;
			else if (de_a->len == de_b->len)
				ret = 0;
			else
				ret = -1;
		}
		return ret;

		break;
	default:
		// cannot compare them
		return 0;
		break;
	}
}

char* _pick_a_key(char *k, char *buf, int c)
{
	c--;
	if (!k || !*k)
		return NULL;
	for (;*k && c-- ;) {
		if (*k == ',') {
			k++;
			break;
		}
		*buf++ = *k++;
	}
	*buf = '\0';
	return k;
}
DLLEXPORT int compare_dicomfile(dicomfile *de_a, dicomfile *de_b, char* keys)
{
	int ret = 0;
	char key[128], *p = keys;
	while (1) {
		p = _pick_a_key(p, key, 128);
		if (p) {
			ret = compare_dataelement
					(de_a->get_dataelement(key), de_b->get_dataelement(key));
			if (ret)
				return ret;
		} else
			break;
	}
	return ret;
}

} // namespace dicom -----------------------------------------------------
