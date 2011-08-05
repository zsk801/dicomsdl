/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string>
#include <sstream>
#include <vector>
#include "dicom.h"
#include "memutil.h"
#include "recordoffset.h"
#include "pixelsequence.h"
#include "outstream.h"
#include "errormsg.h"

namespace dicom { //------------------------------------------------------

void dataelement::set_endian(int _endian)
{
	if (endian == _endian) return;

	endian = _endian;
	switch (vr) {
		case VR_AT:	case VR_OW:	case VR_SS:	case VR_US:
			swap16(ptr, len); break;
		case VR_FL:	case VR_OF:	case VR_SL:	case VR_UL:
			swap32(ptr, len);	break;
		case VR_FD:
			swap64(ptr, len); break;
		default:
			break;
	}
}

// get int type value(s) -------------------------------------------------

int dataelement::to_int(int defaultvalue)
{
	if (!is_valid() || len == 0) return defaultvalue;
	set_endian();

	switch (vr) {
		case VR_SL:	return ptr_as_int32(ptr);
		case VR_SS: return ptr_as_int16(ptr);
		case VR_UL: return ptr_as_uint32(ptr);
		case VR_US: return ptr_as_uint16(ptr);
		case VR_AT: return (ptr_as_uint16_p(ptr)[0] << 16) +
						   (ptr_as_uint16_p(ptr)[1]);
		case VR_IS:
			char buf[256];
			memcpy(buf, ptr, (len<255 ? len : 255));
			buf[len<255 ? len : 255] = '\0';
			return atoi(buf);
		case VR_FL:	case VR_FD:	case VR_DS:
			return (long) floor(to_double((double)defaultvalue)+0.5);
		//case VR_OFFSET:///////////////////////////////////////////////////////////////////////
		//	return AS_OFFSET(de)->offset;
		default:
			return defaultvalue;
	}
}

void dataelement::to_int_values_a(int **values, int *nvalues)
{
	if (len == 0) {
		*values = NULL;
		*nvalues = 0;
		return;
	}

	set_endian();

	int *_values;
	int _nvalues;

	_nvalues = get_vm();
	_values  = (int *)malloc(_nvalues * sizeof(long));
	if (!_values) {
		build_error_message("in dataelement::to_int_values_a(): "
				"cannot alloc memory for int array");
		*values = NULL;
		*nvalues = 0;
		return;
	}

	int i;

	switch(vr) {
		case VR_SL:
			for (i = 0; i < _nvalues; i++)
				_values[i] = (int)(ptr_as_int32_p(ptr)[i]);
			break;
		case VR_SS:
			for (i = 0; i < _nvalues; i++)
				_values[i] = (int)(ptr_as_int16_p(ptr)[i]);
			break;
		case VR_UL:
			for (i = 0; i < _nvalues; i++)
				_values[i] = (int)(ptr_as_uint32_p(ptr)[i]);
			break;
		case VR_US:
			for (i = 0; i < _nvalues; i++)
				_values[i] = (int)(ptr_as_uint16_p(ptr)[i]);
			break;
		case VR_AT:
			for (i = 0; i < _nvalues; i++)
				_values[i] = (int)((ptr_as_uint16_p(ptr)[i*2] << 16) +
								   (ptr_as_uint16_p(ptr)[i*2+1]));
			break;
		case VR_IS:
			{
				char *p, *nextp;
				p = (char *)ptr;
				for (i = 0; i < _nvalues; i++) {
					_values[i] = strtol(p, &nextp, 10);
					p = nextp+1;
					if (p - (char *)ptr >= (int)len) break;
				}
			}
			break;
		case VR_FL:	case VR_FD:	case VR_DS:
			{
				double* dvalues;
				int ndvalues;
				to_double_values_a(&dvalues, &ndvalues);
				for (i = 0; i < ndvalues; i++)
					_values[i] = (int)(floor(dvalues[i]+0.5));
				free(dvalues);
			}
			break;
		default:
			if (_values) free(_values);
			_values = NULL;
			_nvalues = 0;
			break;
	}
	*values = _values;
	*nvalues = _nvalues;
}


std::vector<int> dataelement::to_int_values()
{
	std::vector <int> v;
	int *p, *values, nvalues;
	to_int_values_a(&values, &nvalues);
	if (values) {
		p = values;
		while (nvalues--) v.push_back(*p++);
		free(values);
	}
	return v;
}

// get double type value(s) ------------------------------------------

double dataelement::to_double(double defaultvalue)
{
	if (!is_valid() || len == 0) return defaultvalue;
	set_endian();

	switch (vr) {
		case VR_FL:	return ptr_as_float32(ptr);
		case VR_FD:	return ptr_as_float64(ptr);
		case VR_DS:
			char buf[256];
			memcpy(buf, ptr, (len<255?len:255));
			buf[(len<255?len:255)] = '\0';
			return atof(buf);
			break;

		case VR_SL:	case VR_SS:	case VR_UL:	case VR_US:
		case VR_IS:
			return (double) to_int((long)defaultvalue);
			break;

		default:
			return defaultvalue;
	}
}

void dataelement::to_double_values_a(double **values, int *nvalues)
{
	if (len == 0) {
		*values = NULL;
		*nvalues = 0;
		return;
	}
	set_endian();

	double *_values;
	int _nvalues;

	_nvalues = get_vm();
	_values = (double *)malloc(_nvalues * sizeof(double));
	if (!_values) {
		build_error_message("in dataelement::to_double_values_a(): "
				"cannot alloc memory for double array");
		*values = NULL;
		*nvalues = 0;
		return;
	}

	int i;

	switch(vr) {
		case VR_FL:
			for (i = 0; i < _nvalues; i++)
				_values[i] = ptr_as_float32_p(ptr)[i];
			break;
		case VR_FD:
			for (i = 0; i < _nvalues; i++)
				_values[i] = ptr_as_float64_p(ptr)[i];
			break;
		case VR_DS:
			{
				char *p, *nextp;
				p = (char *)ptr;
				for (i = 0; i < _nvalues; i++) {
					_values[i] = strtod(p, &nextp);
					p = nextp+1;
					if (p - (char *)ptr >= (int)len) break;
				}
			}
			break;
		case VR_SL:		case VR_SS:		case VR_UL:		case VR_US:
		case VR_IS:
			{
				int *ivalues;
				int nivalues;
				to_int_values_a(&ivalues, &nivalues);
				for (i = 0; i < nivalues; i++)
					_values[i] = (double)(ivalues[i]);
				free(ivalues);
			}
			break;
		default:
			free(_values);
			_values = NULL;
			_nvalues = 0;
			break;
	}
	*values = _values;
	*nvalues = _nvalues;
}

std::vector <double> dataelement::to_double_values()
{
	std::vector <double> v;
	double *p, *values;
	int nvalues;
	to_double_values_a(&values, &nvalues);
	if (values) {
		p = values;
		while (nvalues--) v.push_back(*p++);
		free(values);
	}
	return v;
}

// get string type value -------------------------------------------------

void dataelement::to_string_a(char **val_a, int *len_a)
{
	// 'strz' should be free after used.

	char *p = NULL;
	int n = 0;

	switch (vr) {
	case VR_AE:	case VR_AS:	case VR_CS:	case VR_DA:
	case VR_DS:	case VR_DT:	case VR_IS:	case VR_LO:
	case VR_LT:	case VR_PN:	case VR_SH:	case VR_ST:	case VR_UK:
	case VR_TM:	case VR_UI:
		if (len) {
			n = len;
			p = (char *)malloc(n+1);
			if (!p) goto L_MEMERROR;

			memcpy(p, ptr, n);
			if (p[n-1] == '\0' || isspace((unsigned char)(p[n-1]))) {
				p[n-1] = '\0';
				n -= 1;
			} else
				p[n] = '\0';
		}
		break;

	case VR_SQ:
	case VR_PIXSEQ:
		break;

	case VR_OFFSET:
		// (0004,1200), (0004,1202), (0004,1400), (0004,1420)
		p = (char *)malloc(128);
		if (!p) goto L_MEMERROR;
		n = sprintf(p, "{%08x}", ((recordoffset *)ptr)->offset);
		break;

	case VR_SL:			case VR_SS:
		if (len) {
			std::stringstream ss;
			int* values;
			int nvalues;

			to_int_values_a(&values, &nvalues);
			ss << values[0];
			if (nvalues > 1)
				for (int i = 1; i < nvalues; i++)
					ss << "\\" << values[i];
			free(values);

			n = (int)ss.tellp();
			p = (char *)malloc(n+1);
			if (!p) goto L_MEMERROR;
			ss.read(p, n);
			p[n] = '\0';
		}
		break;

	case VR_UL:			case VR_US:
		if (len) {
			std::stringstream ss;
			unsigned int* values;
			int nvalues;

			to_int_values_a((int **)&values, &nvalues);
			ss << values[0];
			if (nvalues > 1)
				for (int i = 1; i < nvalues; i++)
					ss << "\\" << values[i];
			free(values);

			n = (int)ss.tellp();
			p = (char *)malloc(n+1);
			if (!p) goto L_MEMERROR;
			ss.read(p, n);
			p[n] = '\0';
		}
		break;
	case VR_FL:			case VR_FD:
		if (len) {
			std::stringstream ss;
			double* values;
			int nvalues;

			to_double_values_a(&values, &nvalues);
			ss << values[0];
			if (nvalues > 1)
				for (int i = 1; i < nvalues; i++)
					ss << "\\" << values[i];
			free(values);

			n = (int)ss.tellp();
			p = (char *)malloc(n+1);
			if (!p) goto L_MEMERROR;
			ss.read(p, n);
			p[n] = '\0';
		}
		break;
	case VR_AT:
		if (len) {
			std::stringstream ss;
			unsigned int* values;
			int nvalues;
			char buf[16];

			to_int_values_a((int **)&values, &nvalues);
			int i = 0;
			while (1) {
				sprintf(buf, "(%04x,%04x)",
						values[i] >> 16, values[i]&0xffff);
				ss << buf;
				if (++i >= nvalues) break;
				ss << "\\";
			}
			free(values);

			n = (int)ss.tellp();
			p = (char *)malloc(n+1);
			if (!p) goto L_MEMERROR;
			ss.read(p, n);
			p[n] = '\0';
		}
		break;

	case VR_OB:	case VR_UN:	case VR_UT:
	case VR_OW:
	case VR_OF:
	default:
		if (len) {
			n = len;
			p = (char *)malloc(n+1);
			if (!p) goto L_MEMERROR;

			memcpy(p, ptr, n);
			p[n] = '\0';
		}
		break;
	}

	*val_a = p;
	if (len_a) *len_a = n;
	return;

L_MEMERROR:
	build_error_message("in dataelement::to_string_a(): "
			"cannot alloc memory for string array");
	*val_a = NULL;
	*len_a = 0;
	return;
}

std::string dataelement::to_string(const char* defaultvalue)
{
	char *strz;
	int len;
	to_string_a(&strz, &len);
	if (strz) {
		std::string v(strz, len);
		free(strz);
		return v;
	}
	else {
		// just return a empty string
		return std::string(defaultvalue);
	}
}

// get raw value ----------------------------------------------------

void dataelement::raw_value(char **val, int *len)
{
	static const char *pixseqdesc = "[   PIXEL SEQUENCE   ]";
	static const char *seqdesc = "[   SEQUENCE   ]";
	set_endian();

	switch (vr) {
		case VR_AE:	case VR_AS:	case VR_CS:	case VR_DA:
		case VR_DS:	case VR_DT:	case VR_IS:	case VR_LO:
		case VR_LT:	case VR_PN:	case VR_SH:	case VR_ST: case VR_UK:
		case VR_TM:	case VR_UI:
			*val = (char *)ptr;
			*len = (this->len > 0
					&& ((*val)[this->len-1] == 0
					|| isspace(((unsigned char *)(*val))[this->len-1]))
						? this->len-1 : this->len );
			break;

		case VR_SQ:
			*val = (char *)seqdesc;
			*len = (int)strlen(seqdesc);
			break;

		case VR_PIXSEQ:
			*val = (char *)pixseqdesc;
			*len = (int)strlen(pixseqdesc);
			break;

		case VR_SL:			case VR_SS:
		case VR_UL:			case VR_US:
		case VR_FL:			case VR_FD:
		case VR_AT:

		case VR_OB:	case VR_OF:	case VR_OW:
		case VR_UN:	case VR_UT:
		case VR_NULL: // *_ptr = NULL, *_len = 0

		default:
			*val = (char *)ptr;
			*len = this->len;
			break;
	}
}

// get dump string of dicomfile object -----------------------------------

void __to_printable_string
	(char *str, int len, std::stringstream &ss, int max_len)
{
	char dummy[16];
	if (max_len && max_len < len)
		len = max_len;
	while (len--) {
		if (*str == '\"')
			ss << "\\\"";
		else if (*str == '\\')
			ss << "\\\\";
		else if (isprint((unsigned char)(*str)))
			ss << *str;
		else {
			sprintf(dummy, "\\x%02x", (unsigned char)*str);
			ss << dummy;
		}
		str++;
	}
}

void dataelement::repr_string_a(char **val_a, int *len_a, int max_len)
{
	static const char *nulldesc = "(null)";
	char *str; int len;
	char dummy[128];
	int _len_a;

	std::stringstream ss;

	if (max_len < 0) max_len = 0;
	switch (vr) {
		case VR_AE:	case VR_AS:	case VR_CS:	case VR_DA:
		case VR_DS:	case VR_DT:	case VR_IS:	case VR_LO:
		case VR_LT:	case VR_PN:	case VR_SH:	case VR_ST:	case VR_UK:
		case VR_TM:	case VR_UI:
			to_string_a(&str, &len);
			if (str) {
				ss << "\"";
				__to_printable_string(str, len, ss, max_len);
				free(str);
				ss << "\"";
			} else
				ss << nulldesc;
			break;

		case VR_SQ:
			sprintf(dummy, "[ DATASET SEQUNCE WITH %d SET(S) ]",
					number_of_datasets());
			ss << dummy;
			break;

		case VR_PIXSEQ:
			sprintf(dummy, "[ ENCODED PIXEL SEQUENCE WITH %d FRAME(S) ]",
					number_of_frames());
			ss << dummy;
			break;

		case VR_SL:			case VR_SS:
		case VR_UL:			case VR_US:
		case VR_FL:			case VR_FD:
		case VR_AT:
		case VR_OFFSET:
			to_string_a(&str, &len);
			if (str) {
				ss << str;
				free(str);
				break;
			} else
				ss << nulldesc;
			break;

		case VR_OB:	case VR_UN:	case VR_UT:
		case VR_OW:
		case VR_OF:
		default:
			if (ptr && this->len) {
				ss << "\"";
				__to_printable_string(
						(char *)(ptr), this->len, ss, max_len);
				ss << "\"";
			} else
				ss << nulldesc;
			break;
	}

	_len_a = (int)ss.tellp();
	if (max_len == 0) max_len = _len_a;
	if (_len_a <= 0) {
		_len_a = 0;
		*val_a = NULL;
	} else if (_len_a <= max_len) {
		*val_a = (char *)malloc(_len_a+1);
		ss.read(*val_a, _len_a);
		(*val_a)[_len_a] = '\0';
	} else {
		_len_a = max_len;
		*val_a = (char *)malloc(_len_a+1);
		ss.read(*val_a, _len_a);
		int ndots = 3;
		while (ndots-- > 0) // tail dot
			if (_len_a - ndots >= 0)
				(*val_a)[_len_a - ndots] = '.';
		(*val_a)[_len_a] = '\0';
	}
	if (len_a) *len_a = _len_a;
}

std::string dataelement::repr_string(int max_len)
{
	char *val;
	int len;
	repr_string_a(&val, &len, max_len);
	std::string v(val, len);
	if (val) free(val);
	return v;
}

uidtype dataelement::to_uid()
{
	if (vr == VR_UI) {
		char *uv;
		to_string_a(&uv, NULL);
		if (uv) {
			uidtype u = uidvalue_to_uid(uv);
			free(uv);
			return u;
		}
	}
	return UID_UNKNOWN;
}

void dataelement::from_int(int value)
{
	set_endian();

	switch (vr) {
	case VR_SL:
		free_value();
		len = sizeof(int32);		ptr = malloc(len);
		ptr_as_int32(ptr) = (int32)value;
		own_memory = true;
		break;
	case VR_SS:
		free_value();
		len = sizeof(int16);		ptr = malloc(len);
		ptr_as_int16(ptr) = (int16)value;
		own_memory = true;
		break;
	case VR_UL:
		free_value();
		len = sizeof(uint32);		ptr = malloc(len);
		ptr_as_int32(ptr) = (uint32)value;
		own_memory = true;
		break;
	case VR_US:
		free_value();
		len = sizeof(uint16);		ptr = malloc(len);
		ptr_as_uint16(ptr) = (uint16)value;
		own_memory = true;
		break;
	case VR_AT:
		free_value();
		len = sizeof(uint32);		ptr = malloc(len);
		ptr_as_uint16_p(ptr)[0] =  ((uint32)value >> 16);
		ptr_as_uint16_p(ptr)[1] =  ((uint32)value & 0xffff);
		own_memory = true;
		break;
	case VR_IS:
		free_value();
		char buf[256];
		len = snprintf(buf, 255, "%d", value);
		if (len & 0x01)  // make even length
			{ buf[len] = ' '; len++; }
		ptr = (void *)malloc(len);
		memcpy(ptr, buf, len);
		own_memory = true;
		break;
	case VR_FL:	case VR_FD:	case VR_DS:
		from_double((double)value);
		break;
	default:
		// -- PRINT WARNING MESSAGE ////////////////////////////////////////////////////////
		printf("WARNING: cannot set 'int' value to the element "
			   "with tag=(%04x,%04x), VR='%s'.\n",
			   group_in_tag(tag), element_in_tag(tag),
			   get_vr_repr(vr));
		// --
	}
}

void dataelement::from_int_values(int *values, int nvalues)
{
	int i;

	set_endian();
	switch (vr) {
	case VR_SL:
		free_value();
		if (nvalues && values) {
			len = nvalues * sizeof(int32);
			ptr = (void *)malloc(len);
			for (i = 0; i < nvalues; i++)
				ptr_as_int32_p(ptr)[i] = (int32)(values[i]);
			own_memory = true;
		}
		break;
	case VR_SS:
		free_value();
		if (nvalues && values) {
			len = nvalues * sizeof(int16);
			ptr = (void *)malloc(len);
			for (i = 0; i < nvalues; i++)
				ptr_as_int16_p(ptr)[i] = (int16)(values[i]);
			own_memory = true;
		}
		break;
	case VR_UL:
		free_value();
		if (nvalues && values) {
			len = nvalues * sizeof(uint32);
			ptr = (void *)malloc(len);
			for (i = 0; i < nvalues; i++)
				ptr_as_uint32_p(ptr)[i] = (uint32)(values[i]);
			own_memory = true;
		}
		break;
	case VR_US:
		if (nvalues && values) {
			len = nvalues * sizeof(uint16);
			ptr = (void *)malloc(len);
			for (i = 0; i < nvalues; i++)
				ptr_as_int16_p(ptr)[i] = (int16)(values[i]);
			own_memory = true;
		}
		break;
	case VR_AT:
		free_value();
		if (nvalues && values) {
			len = nvalues * sizeof(uint32);
			ptr = (void *)malloc(len);
			for (i = 0; i < nvalues; i++) {
				ptr_as_uint16_p(ptr)[i*2] =
						 ((uint32)values[i] >> 16);
				ptr_as_uint16_p(ptr)[i*2+1] =
						((uint32)values[i] & 0xffff);
			}
			own_memory = true;
		}
		break;
	case VR_IS:
		free_value();
		if (nvalues && values) {
			char *buf; buf = new char [16*nvalues];
			len = 0;
			for (i = 0; i < nvalues; i++)
				len +=
					snprintf(buf+len,
							  16*nvalues-len,
							  "%d\\", values[i]);
			if (len > 0) len --; // remove training '\'
			if (len & 0x01)  // make even length
				{ buf[len] = ' '; len++; }
			ptr = (void *)malloc(len);
			memcpy(ptr, buf, len);
			delete [] buf;
			own_memory = true;
		}
		break;

	case VR_FL:	case VR_FD:	case VR_DS:
		free_value();
		if (nvalues && values) {
			double *dvalues;
			dvalues = new double[nvalues];
			for (i = 0; i < nvalues; i++)
				dvalues[i] = (double)values[i];
			from_double_values(dvalues, nvalues);
			delete [] dvalues;
		}
		break;

	default:
		// -- PRINT WARNING MESSAGE
		printf("WARNING: cannot set 'int' values to the element "
			   "with tag=(%04x,%04x), VR='%s'.\n",
			   group_in_tag(tag), element_in_tag(tag),
			   get_vr_repr(vr));
		// --
	}

}

void dataelement::set_value(std::vector <int> & v)
{
	int nvalues = v.size();
	int *values = new int[nvalues];
	for (int i = 0; i < nvalues; i++)
		values[i] = v[i];
	from_int_values(values, nvalues);
	delete[] values;
}

void dataelement::from_double(double value)
{
	set_endian();
	switch (vr) {
	case VR_FL:
		free_value();
		len = sizeof(float32);		ptr = malloc(len);
		ptr_as_float32(ptr) = (float32)value;
		own_memory = true;
		break;
	case VR_FD:
		free_value();
		len = sizeof(float64);		ptr = malloc(len);
		ptr_as_float64(ptr) = (float64)value;
		own_memory = true;
		break;
	case VR_DS:
		free_value();
		char buf[256];
		len = snprintf(buf, 255, "%.10g", value);
		if (len & 0x01)  // make even length
			{ buf[len] = ' '; len++; }
		ptr = (void *)malloc(len);
		memcpy(ptr, buf, len);
		own_memory = true;
		break;
	case VR_SL:	case VR_SS:	case VR_UL:	case VR_US:
	case VR_IS:	case VR_AT:
		from_int((int)floor(value+0.5));
		break;
	default:
		// -- PRINT WARNING MESSAGE
		printf("WARNING: cannot set 'double' value to the element "
			   "with tag=(%04x,%04x), VR='%s'.\n",
			   group_in_tag(tag), element_in_tag(tag),
			   get_vr_repr(vr));
		// --
	}
}

void dataelement::from_double_values(double *values, int nvalues)
{
	int i;
	set_endian();
	switch (vr) {
	case VR_FL:
		free_value();
		if (nvalues && values) {
			len = nvalues * sizeof(float32);
			ptr = (void *)malloc(len);
			for (i = 0; i < nvalues; i++)
				ptr_as_float32_p(ptr)[i] = (float32)(values[i]);
			own_memory = true;
		}
		break;
	case VR_FD:
		free_value();
		if (nvalues && values) {
			len = nvalues * sizeof(float64);
			ptr = (void *)malloc(len);
			for (i = 0; i < nvalues; i++)
				ptr_as_float64_p(ptr)[i] = (float64)(values[i]);
			own_memory = true;
		}
		break;
	case VR_DS:
		free_value();
		if (nvalues && values) {
			char *buf; buf = new char [20*nvalues];
			len = 0;
			for (i = 0; i < nvalues; i++)
				len +=
					snprintf(buf+len,
							  16*nvalues-len,
							  "%.10g\\", values[i]);
			if (len > 0) len --; // remove training '\'
			if (len & 0x01)  // make even length
				{ buf[len] = ' '; len++; }
			ptr = (void *)malloc(len);
			memcpy(ptr, buf, len);
			delete [] buf;
			own_memory = true;
		}
		break;
	case VR_SL:		case VR_SS:		case VR_UL:		case VR_US:
	case VR_IS:		case VR_AT:
		{
			free_value();
			if (nvalues && values) {
				int *ivalues;
				ivalues = new int[nvalues];
				for (i = 0; i < nvalues; i++)
					ivalues[i] = (int)floor(values[i]+0.5);
				from_int_values(ivalues, nvalues);
				delete [] ivalues;
			}
		}
		break;
	default:
		// -- PRINT WARNING MESSAGE
		printf("WARNING: cannot set 'double' values to the element "
			   "with tag=(%04x,%04x), VR='%s'.\n",
			   group_in_tag(tag), element_in_tag(tag),
			   get_vr_repr(vr));
		// --
	}
}

void dataelement::set_value(std::vector <double> & v)
{
	int nvalues = v.size();
	double *values = new double[nvalues];
	for (int i = 0; i < nvalues; i++)
		values[i] = v[i];
	from_double_values(values, nvalues);
	delete[] values;
}

void dataelement::from_string(const char *str)
{
	if (vr == VR_SQ || vr == VR_OFFSET || vr == VR_PIXSEQ)
		return;
	free_value();
	set_endian();
	if (str) {
		size_t strsiz = strlen(str);
		from_data(str, strsiz);
	}
}

void dataelement::from_data(const char *data, int datasize)
{
	if (vr == VR_SQ || vr == VR_OFFSET || vr == VR_PIXSEQ)
		return;

	free_value();
	set_endian();
	if (data && datasize) {
		len = datasize;
		if (len & 0x01) len++; // make length even
		ptr = malloc(len);

		if (ptr) {
			((char *)(ptr))[len-1] =
				(vr == VR_OB || vr == VR_UN || vr == VR_UT
						? ' ':'\x00');
			memcpy(ptr, data, datasize);
			own_memory = true;
		} else {
			len = 0;
			own_memory = false;
			build_error_message("in dataelement::from_data(): "
				"cannot alloc %d bytes for ptr", len);
		}
	}
}

int dataelement::number_of_datasets()
{
	return vr==VR_SQ ? ((sequence *)ptr)->size() : 0;
};

int dataelement::number_of_frames()
{
	return vr==VR_PIXSEQ? ((pixelsequence *)ptr)->number_of_frames() : 0;
}

dataset* dataelement::dataset_at(int idx)
{
	return vr==VR_SQ ? ((sequence *)ptr)->dataset_at(idx) : NULL;
}

dataset* dataelement::add_dataset()
{
	return vr==VR_SQ ? ((sequence *)ptr)->add_dataset() : NULL;
}


void dataelement::from_dataset(dataset *ds)
{
	if (vr == VR_OFFSET)
		((recordoffset *)ptr)->from_dataset(ds);
}

dataset* dataelement::to_dataset()
{
	return vr==VR_OFFSET ? ((recordoffset *)ptr)->to_dataset() : NULL;
}

int dataelement::alloc_memory(int _len)
{
	if (vr != VR_SQ && vr != VR_PIXSEQ && vr != VR_OFFSET) {
		free_value();
		len = make_even(_len);
		if (len) {
			ptr = malloc(len);
			if (!ptr) {
				own_memory = false;
				build_error_message("in dataelement::alloc_memory(int): "
						"can't alloc %d bytes", _len);
				return DICOM_MEMORY_ERROR;
			}
			own_memory = true;
		} else {
			ptr = NULL;
			own_memory = false;
		}
	}
	return DICOM_OK;
}

void dataelement::free_value()
{
	if (own_memory) {
		switch (vr) {
		case VR_SQ:
			delete (sequence *)ptr;
			break;
		case VR_PIXSEQ:
			delete (pixelsequence *)ptr;
			break;
		case VR_OFFSET:
			delete (recordoffset *)ptr;
			break;

		default:
			if (own_memory && ptr)
				free (ptr);
			break;
		}
	}
	ptr = NULL;
	len = 0;
	own_memory = false;
}

int dataelement::get_vm()
{
	if (len == 0) return 0;
	switch (vr) {
		case VR_FD:
			return len / 8;
		case VR_AT:		case VR_FL:		case VR_OF:
		case VR_UL:		case VR_SL:
			return len / 4;
		case VR_US:		case VR_SS:		case VR_OW:
			return len / 2;

		// PS 3.5-2009, 6.4 VALUE MULTIPLICITY (VM) AND DELIMITATION
		case VR_AE:		case VR_AS:
		case VR_CS:		case VR_DA:
		case VR_DS:		case VR_DT:
		case VR_IS:		case VR_LO:
		case VR_PN:		case VR_SH:		case VR_UK:
		case VR_TM:		case VR_UI:
			{
				int dels = 0;
				for (uint32 i = 0; i < len; i++)
					if (((char *)ptr)[i] == '\\') dels ++;
				return dels + 1;
			}
			break;

		// in case of VR_SQ, vm() returns number of datasets in sequence
		case VR_SQ:
			return number_of_datasets();
		case VR_PIXSEQ:
			return number_of_frames();
		case VR_OFFSET:

		case VR_OB:		case VR_UN:
		case VR_ST:		case VR_UT:
		case VR_LT:		default:
			return 1;
	}

}

static uint64 _zeros_[] = {0,0,0,0,0,0,0,0};
DLLEXPORT dataelement* nullelement()
{
	return (dataelement *)_zeros_;
}

dataelement_iterator::dataelement_iterator(dataset *ds)
{
	this->ds = ds;
	if (ds)
		it = ds->edict.begin();
}

dataelement* dataelement_iterator::next() {
	if (ds && it != ds->edict.end()) {
		dataelement* e;
		e = it->second;
		it++;
		return e;
	} else {
		return NULL;
	}
}

void dataelement::_save(void *ostream, uidtype tsuid, opttype opt)
{
	outstream *os = (outstream *)ostream;

	int endian =
		(tsuid==UID_EXPLICIT_VR_BIG_ENDIAN ? BIG_ENDIAN:LITTLE_ENDIAN);
	int explicit_vr = (tsuid != UID_IMPLICIT_VR_LITTLE_ENDIAN);


	if (group_in_tag(tag) == 0x0002) {
		endian = LITTLE_ENDIAN;
		explicit_vr = 1;
	}

	set_endian(endian);

	switch(vr) {
	case VR_SQ:
		{
			os->write16u(group_in_tag(tag), endian);
			os->write16u(element_in_tag(tag), endian);

			len = -1;

			if (explicit_vr) {
				os->write16u_le(vr); // explicit VR
				os->write16u_le(0x0000);
				os->write32u(len, endian);
			} else {
				os->write32u_le(len); // UID_IMPLICIT_VR_LITTLE_ENDIAN
			}

			if (!(opt & OPT_SAVE_IMPLICIT_SQ_LENGTH))
			{
				os->reserve_bytes_for_length_value(
						(long)this, 4, endian, -4);
				os->mark_start_pos((long)this);
			}

			// write datasets in a sequence
			((sequence *)ptr)->_save(ostream, tsuid, opt);

			if (!(opt & OPT_SAVE_IMPLICIT_SQ_LENGTH)) {
				os->mark_end_pos((long)this);
			}
		}
		break;
	case VR_PIXSEQ:
		{
			// Table A.4-1, 2
			os->write16u(0x7fe0, endian);
			os->write16u(0x0010, endian);
			os->write16u_le(VR_OB);
			os->write16u_le(0x0000);
			os->write32u(0xffffffff, endian);
			((pixelsequence *)ptr)->_save(os, opt);
		}
		break;
	case VR_OB: case VR_OW: case VR_OF: case VR_UT: case VR_UN:
		{
			// Table 7.1-1 EXPLICIT VR OF OB, OW, OF, SQ, UT OR UN
			// tag(4) + vr(2) + 0000h(2) + len(4) + val(len)
			// Table 7.1-3 Data element with implicit VR
			// tag(4) + len(4) + val(len)

			os->write16u(group_in_tag(tag), endian);
			os->write16u(element_in_tag(tag), endian);

			if (explicit_vr) {
				os->write16u_le(vr);
				os->write16u_le(0x0000);
				os->write32u(len, endian);
			} else {
				os->write32u_le(len); // UID_IMPLICIT_VR_LITTLE_ENDIAN
			}

			os->write((uint8 *)ptr, len);
		}
		break;
	case VR_OFFSET:
	{
		// Table 7.1-2 EXPLICIT VR OTHER THAN AS SHOWN IN TABLE 7.1-1
		// tag(4) + vr(2) + len(2) + val(len)
		// Table 7.1-3 Data element with implicit VR
		// tag(4) + len(4) + val(len)

		os->write16u(group_in_tag(tag), endian);
		os->write16u(element_in_tag(tag), endian);

		if (explicit_vr) {
			os->write16u_le(VR_UL);
			os->write16u(4, endian);
		} else {
			os->write32u_le(4); // UID_IMPLICIT_VR_LITTLE_ENDIAN
		}

		os->reserve_bytes_for_offset_value(
				(long)(((recordoffset *)ptr)->to_dataset()),
				4, endian);
	}
	break;
	default:
		{
			// Table 7.1-2 EXPLICIT VR OTHER THAN AS SHOWN IN TABLE 7.1-1
			// tag(4) + vr(2) + len(2) + val(len)
			// Table 7.1-3 Data element with implicit VR
			// tag(4) + len(4) + val(len)

			os->write16u(group_in_tag(tag), endian);
			os->write16u(element_in_tag(tag), endian);

			if (explicit_vr) {
				os->write16u_le(vr);
				os->write16u(len, endian);
			} else {
				os->write32u_le(len); // UID_IMPLICIT_VR_LITTLE_ENDIAN
			}

			/////////////////////////////////////////////////////////// LEN == ODD ?
			os->write((uint8 *)ptr, len);
		}
		break;
	}

}

} // namespace dicom -----------------------------------------------------
