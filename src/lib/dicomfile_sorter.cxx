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

// -----------------------------------------------------------------------
// comparing two dataelement or two dataset
//

// 1 if a>b, 0 if a=b, -1 if a<b
int compare_dataelement(dataelement *de_a, dataelement *de_b)
{
	// check if both dataelement is valid

	if (!(*de_a && *de_b)) {
		if (*de_a) return -1; // valid < NULL
		if (*de_b) return 1; // NULL > valid
		return 0;
	}

	// compare only elements with same VR
	if (de_a->vr != de_b->vr)
		return 0;

	switch (de_a->vr) {
	case VR_SL:	case VR_SS:	case VR_UL:	case VR_US:
	case VR_IS:	case VR_AT:
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

	case VR_FL:	case VR_FD:	case VR_DS:
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

	case VR_AE:	case VR_AS:
	case VR_CS:	case VR_DA:	case VR_DT:
	case VR_LO:	case VR_LT:
	case VR_PN:	case VR_SH:	case VR_ST:
	case VR_TM:	case VR_UI:
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

int compare_dataset(
		dataset *ds_a, dataset *ds_b, const char **keys)
{
	int ret = 0;
	while (*keys) {
		ret = compare_dataelement
				(ds_a->get_dataelement(*keys),
				 ds_b->get_dataelement(*keys));
		if (ret)
			return ret;
		keys++;
	}
	return ret;
}

// -----------------------------------------------------------------------
// pick some keys from datafile object
//

DLLEXPORT dataset* pick_dataelements(dicomfile *df, const char **keys)
{
	dataset *ds = new dataset();
	while (*keys) {
		dataelement *de_a, *de_b;
		de_a = df->get_dataelement(*keys);
		if (de_a->is_valid()) {
			de_b = ds->add_dataelement(*keys);
			if (de_b->is_valid())
				de_b->from_data((const char *)(de_a->ptr), de_a->len);
		}
		keys++;
	}
	return ds;
}


// -----------------------------------------------------------------------
// make a descriptive string for a dataset
//

void rtrim(std::string& str) {
	std::string::size_type pos = str.size();
	while (pos > 0 && isspace(str[pos - 1])) pos--;
	str.erase(pos);
}

std::string build_dataset_desc(dataset *ds, const char **keys)
{
	std::list<std::string> li;
	dataelement *de;
	while (*keys) {
		de = ds->get_dataelement(*keys);
		if (de->is_valid()) {
			std::string v = de->to_string();
			rtrim(v); li.push_back(v);
		}
		keys++;
	}

	if (li.size() == 0)
		return std::string("(null)");

	std::string repr = "";

	std::list<std::string>::iterator it;

	it = li.begin();
	repr += (*it++);

	for (; it != li.end(); it++)
		repr += " "+(*it);

	return repr;
}

// -----------------------------------------------------------------------
// helper class for sorting dicom files
//

class c_compare_dataset {
public:
	const char **keys;
	c_compare_dataset(const char **keys_): keys(keys_) { };

	// return true if ds_a < ds_b
	bool operator()(dataset* ds_a, dataset* ds_b) const
	{
		return compare_dataset(ds_a, ds_b, keys)<0;
	}
};

// -----------------------------------------------------------------------
// a series object that holds many instances
//

// mapping dataset (that holds instance info) with filenames
typedef std::map<dataset*, std::string, c_compare_dataset>
	map_inst_str_t;

struct inst_rec_t {
	map_inst_str_t *m;
	map_inst_str_t::iterator curr_it;

	const char **inst_keys;
	c_compare_dataset *i_cmp;

	std::string desc; // description of series

	inst_rec_t(const char **_inst_keys) :
		inst_keys(_inst_keys)
	{
		i_cmp = new c_compare_dataset(inst_keys);
		m = new map_inst_str_t(*i_cmp);
		curr_it = m->end();
	}

	~inst_rec_t()
	{
		map_inst_str_t::iterator it;
		for (it = m->begin(); it != m->end(); it++) {
			delete it->first; // delete key
		}
		delete m;
		delete i_cmp;
	}

	void add(dicomfile *dfobj, std::string fn)
	{
		dataset *ds = pick_dataelements(dfobj, inst_keys);
		map_inst_str_t::iterator it = m->find(ds);
		if (it == m->end()) {
			(*m)[ds] = fn;
		} else {
			printf("PASS %s\n", fn.c_str());
			delete ds;
		}
	}

	size_t size() { return m->size(); };

	void rewind() { curr_it = m->end(); }

	const char* get_next()
	{
		if (size() == 0)
			return NULL;

		if (curr_it == m->end())
			curr_it = m->begin();
		else
			curr_it++;

		if (curr_it == m->end())
			return NULL;

		return curr_it->second.c_str();
	}
};

// -----------------------------------------------------------------------
// a study object that holds many series
//

// mapping dataset (that holds series info) with inst_rec_t
typedef std::map<dataset*, inst_rec_t*, c_compare_dataset>
	map_series_inst_t;

struct series_rec_t {
	map_series_inst_t *m; // list of instance that a series has
	map_series_inst_t::iterator curr_it;

	const char **ser_keys;
	const char **ser_desc_keys;
	const char **inst_keys;
	c_compare_dataset *s_cmp;

	std::string desc; // description of study

	series_rec_t(const char **_ser_keys,
			const char **_ser_desc_keys,
			const char **_inst_keys)
		: ser_keys(_ser_keys),
		  ser_desc_keys(_ser_desc_keys),
		  inst_keys(_inst_keys)
	{
		s_cmp = new c_compare_dataset(ser_keys);
		m = new map_series_inst_t(*s_cmp);
		curr_it = m->end();
	}

	~series_rec_t()
	{
		map_series_inst_t::iterator it;
		for (it = m->begin(); it != m->end(); it++) {
			delete it->second; // delete value
			delete it->first; // delete key
		}
		delete m;
		delete s_cmp;
	}

	void add(dicomfile *dfobj, std::string fn)
	{
		dataset *ds = pick_dataelements(dfobj, ser_keys);
		map_series_inst_t::iterator it = m->find(ds);
		if (it == m->end()) {
			(*m)[ds] = new inst_rec_t(inst_keys);
			(*m)[ds]->add(dfobj, fn);
		} else {
			it->second->add(dfobj, fn);
			delete ds;
		}
	}

	size_t size() { return m->size(); };

	void rewind() { curr_it = m->end(); }

	const char* get_next()
	{
		if (size() == 0)
			return NULL;

		if (curr_it == m->end())
			curr_it = m->begin();
		else
			curr_it++;

		if (curr_it == m->end())
			return NULL;

		curr_it->second->rewind();
		curr_it->second->desc =
				build_dataset_desc(curr_it->first, ser_desc_keys);
		return curr_it->second->desc.c_str();
	}
};

// -----------------------------------------------------------------------
// a list of study
//

// mapping dataset (that holds study info) with series_rec_t
typedef std::map<dataset*, series_rec_t*, c_compare_dataset>
	map_study_series_t;

struct study_rec_t {
	map_study_series_t *m;
	map_study_series_t::iterator curr_it;

	const char **study_keys;
	const char **study_desc_keys;
	const char **ser_keys;
	const char **ser_desc_keys;
	const char **inst_keys;
	c_compare_dataset *s_cmp;

	study_rec_t(const char **_study_keys,
			const char **_study_desc_keys,
			const char **_ser_keys,
			const char **_ser_desc_keys,
			const char **_inst_keys)
		: study_keys(_study_keys),
		  study_desc_keys(_study_desc_keys),
		  ser_keys(_ser_keys),
		  ser_desc_keys(_ser_desc_keys),
		  inst_keys(_inst_keys)
	{
		s_cmp = new c_compare_dataset(study_keys);
		m = new map_study_series_t(*s_cmp);
		curr_it = m->end();
	}

	~study_rec_t()
	{
		map_study_series_t::iterator it;
		for (it = m->begin(); it != m->end(); it++) {
			delete it->first; // delete key
			delete it->second; // delete dsmap_series_t
		}
		delete m;
		delete s_cmp;
	}

	void add(dicomfile *dfobj, std::string fn)
	{
		if (!dfobj) return;

		dataset *ds = pick_dataelements(dfobj, study_keys);
		map_study_series_t::iterator it = m->find(ds);
		if (it == m->end()) {
			(*m)[ds] = new series_rec_t(ser_keys, ser_desc_keys, inst_keys);
			(*m)[ds]->add(dfobj, fn);
		} else {
			it->second->add(dfobj, fn);
			delete ds;
		}
	}

	size_t size() { return m->size(); };

	void rewind() { curr_it = m->end(); }

	const char* get_next()
	{
		if (size() == 0)
			return NULL;

		if (curr_it == m->end())
			curr_it = m->begin();
		else
			curr_it++;

		if (curr_it == m->end())
			return NULL;

		curr_it->second->rewind();
		curr_it->second->desc =
				build_dataset_desc(curr_it->first, study_desc_keys);
		return curr_it->second->desc.c_str();
	}

	const char* get_next_study()
	{
		return get_next();
	}

	const char* get_next_series()
	{
		if (curr_it == m->end())
			return NULL;
		return curr_it->second->get_next();
	}

	const char* get_next_instance()
	{
		if (curr_it == m->end())
			return NULL;
		if (curr_it->second->curr_it == curr_it->second->m->end())
			return NULL;
		return curr_it->second->curr_it->second->get_next();
	}

	size_t number_of_series_in_curr_study()
	{
		if (curr_it == m->end())
			return 0;
		return curr_it->second->size();
	}

	size_t number_of_images_in_curr_series()
	{
		if (curr_it == m->end())
			return NULL;
		if (curr_it->second->curr_it == curr_it->second->m->end())
			return NULL;
		return curr_it->second->curr_it->second->size();
	}
};

// -----------------------------------------------------------------------
// wrappers for users
//

dicomfile_sorter::dicomfile_sorter(const char **study_keys,
		const char **study_desc_keys,
		const char **ser_keys,
		const char **ser_desc_keys,
		const char **inst_keys)
{
	study_rec_t *rec;
	rec = new study_rec_t(study_keys, study_desc_keys,
			ser_keys, ser_desc_keys, inst_keys);

	m_rec = (void *)rec;
};

void dicomfile_sorter::add(dicomfile *dfobj, const char *fn)
{
	((study_rec_t *)m_rec)->add(dfobj, std::string(fn));
};

dicomfile_sorter::~dicomfile_sorter()
{
	delete ((study_rec_t *)m_rec);
}

void dicomfile_sorter::rewind()
{
	((study_rec_t *)m_rec)->rewind();
}

const char* dicomfile_sorter::get_next_study()
{
	return ((study_rec_t *)m_rec)->get_next_study();
}

const char* dicomfile_sorter::get_next_series()
{
	return ((study_rec_t *)m_rec)->get_next_series();
}

const char* dicomfile_sorter::get_next_instance()
{
	return ((study_rec_t *)m_rec)->get_next_instance();
}

size_t dicomfile_sorter::number_of_studies()
{
	return ((study_rec_t *)m_rec)->size();
}

size_t dicomfile_sorter::number_of_series_in_curr_study()
{
	return ((study_rec_t *)m_rec)->number_of_series_in_curr_study();
}

size_t dicomfile_sorter::number_of_images_in_curr_series()
{
	return ((study_rec_t *)m_rec)->number_of_images_in_curr_series();
}


} // namespace dicom -----------------------------------------------------
