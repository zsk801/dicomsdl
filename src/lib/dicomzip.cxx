/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include "dicom.h"
#include <map>

namespace dicom { //------------------------------------------------------

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
			double a = de_a->to_double();
			double b = de_b->to_double();
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

//char* _pick_a_key(char *k, char *buf, int c)
//{
//	c--;
//	if (!k || !*k)
//		return NULL;
//	for (;*k && c-- ;) {
//		if (*k == ',') {
//			k++;
//			break;
//		}
//		*buf++ = *k++;
//	}
//	*buf = '\0';
//	return k;
//}


DLLEXPORT int compare_dataset(dataset *ds_a, dataset *ds_b, char **keys)
{
	int ret = 0, i = 0;
	while (keys[i]) {
		ret = compare_dataelement
				(ds_a->get_dataelement(keys[i]),
				 ds_b->get_dataelement(keys[i]));
		if (ret)
			return ret;
		i++;
	}
	return ret;
}


DLLEXPORT dataset* pick_dataelements(dicomfile *df, char **keys)
{
	dataset *ds = new dataset();

	int i = 0;
	while (keys[i]) {
		dataelement *de_a, *de_b;
		de_a = df->get_dataelement(keys[i]);
		if (de_a->is_valid()) {
			de_b = ds->add_dataelement(keys[i]);
			if (de_b->is_valid())
				de_b->from_data((const char *)(de_a->ptr), de_a->len);
		}
		i++;
	}

	return ds;
}

class c_compare_dataset {
public:
	char **keys;
	c_compare_dataset(char **keys_): keys(keys_) { };

	// return true if ds_a < ds_b
	bool operator()(dataset* ds_a, dataset* ds_b) const
	{
		return compare_dataset(ds_a, ds_b, keys)<0;
	}
};


typedef std::map<dataset*, std::string, c_compare_dataset> m_dataset_str_t;

// mapping dataset (that holds instance info) with filenames
struct map_instance_t {
	m_dataset_str_t *m;
	char **inst_keys;
	c_compare_dataset *i_cmp;

	map_instance_t(char **_inst_keys) : inst_keys(_inst_keys) {
		printf("new dsmap_instance_t() <%p>\n", this);
		i_cmp = new c_compare_dataset(inst_keys);
		m = new m_dataset_str_t(*i_cmp);
	}

	~map_instance_t() {
		printf("~dsmap_instance_t() <%p>\n", this);

		m_dataset_str_t::iterator it;
		for (it = m->begin(); it != m->end(); it++) {
			delete it->first; // delete key
		}

		delete m;
		delete i_cmp;
	}

	void add(dicomfile *dfobj, std::string fn)
	{
		dataset *ds;
		ds = pick_dataelements(dfobj, inst_keys);

		std::pair<m_dataset_str_t::iterator, bool> ret;
		ret = m->insert(std::pair<dataset*, std::string>(ds, fn));

		printf("ADDED %s\n", fn.c_str());
		if (ret.second == false)
			delete ds;
	}

};

typedef std::map<dataset*, map_instance_t*, c_compare_dataset>
	m_instance_list_t;

// mapping dataset (that holds series info) with map_instance_t
struct map_series_t {
	m_instance_list_t *m; // list of instance that a series has
	char **ser_keys;
	char **inst_keys;
	c_compare_dataset *s_cmp;

	map_series_t(char **_ser_keys, char **_inst_keys)
		: ser_keys(_ser_keys), inst_keys(_inst_keys) {
		printf("new dsmap_series_t() <%p>\n", this);
		s_cmp = new c_compare_dataset(ser_keys);
		m = new m_instance_list_t(*s_cmp);
	}

	~map_series_t() {
		printf("~dsmap_series_t() <%p>\n", this);

		m_instance_list_t::iterator it;
		for (it = m->begin(); it != m->end(); it++) {
			delete it->second; // delete value
			delete it->first; // delete key
		}

		delete m;
		delete s_cmp;
	}

	void add(dicomfile *dfobj, std::string fn)
	{
		dataset *ds;
		m_instance_list_t::iterator it;

		ds = pick_dataelements(dfobj, ser_keys);
		it = m->find(ds);
		if (it == m->end()) {
			(*m)[ds] = new map_instance_t(inst_keys);
			(*m)[ds]->add(dfobj, fn);
		} else {
			it->second->add(dfobj, fn);
			delete ds;
		}
	}
};


typedef std::map<dataset*, map_series_t*, c_compare_dataset>
	m_series_list_t;

// mapping dataset (that holds study info) with map_series_t
struct map_study_t {
	m_series_list_t *m; // list of series that a study has
	char **study_keys;
	char **ser_keys;
	char **inst_keys;
	c_compare_dataset *s_cmp;

	map_study_t(char **_study_keys, char **_ser_keys, char **_inst_keys)
		: study_keys(_study_keys),
		  ser_keys(_ser_keys),
		  inst_keys(_inst_keys) {
		printf("new dsmap_study_t() <%p>\n", this);
		s_cmp = new c_compare_dataset(study_keys);
		m = new m_series_list_t(*s_cmp);
	}

	~map_study_t() {
		printf("~dsmap_study_t() <%p>\n", this);

		m_series_list_t::iterator it;
		for (it = m->begin(); it != m->end(); it++) {
			delete it->first; // delete key
			delete it->second; // delete dsmap_series_t
		}

		delete m;
		delete s_cmp;
	}

	void add(dicomfile *dfobj, std::string fn)
	{
		dataset *ds;
		m_series_list_t::iterator it;

		ds = pick_dataelements(dfobj, study_keys);
		it = m->find(ds);
		if (it == m->end()) {
			(*m)[ds] = new map_series_t(ser_keys, inst_keys);
			(*m)[ds]->add(dfobj, fn);
		} else {
			it->second->add(dfobj, fn);
			delete ds;
		}
	}
};

DLLEXPORT void test_func(char **fnlist, char **study_keys, char **ser_keys, char **inst_keys)
{
	map_study_t studyset(study_keys, ser_keys, inst_keys);

	int i = 0;
	while (fnlist[i]) {
		dicomfile *df;
		df = open_dicomfile(fnlist[i]);
		studyset.add(df, fnlist[i]);
		close_dicomfile(df);
		i++;
	}
}


} // namespace dicom -----------------------------------------------------
