#include <dicom.h>
#include <list>
#include <string>
#include <fstream>

int main()
{

	std::list<std::string> fnlist;
	std::list<std::string>::iterator it;

	std::fstream fin("/Volumes/StorageOSX/Lab/tmp/fnlist.txt", std::fstream::in);

	if (!fin.good()) {
		fin.close();
		return -1;
	}

	std::string l;

	const char *study_keys[] = {
			"00100020", // PatientID
			"00080020", // StudyDate
			"00081030", // StudyTime
			"0020000d", // StudyInstanceUID
			NULL
	};
	const char *study_desc_keys[] = {
			"00080020", // StudyDate
			"00081030", // StudyDescription
			NULL
	};
	const char *ser_keys[] = {
			"00200011", // SeriesNumber
			"00080031", // SeriesTime
			"0020000e", // SeriesInstanceUID
			"0008103e", // SeriesDescription
			NULL
	};
	const char *ser_desc_keys[] = {
			"00200011", // SeriesNumber
			"0008103e", // SeriesDescription
			NULL
	};
	const char *inst_keys[] = {
			"00200032", // ImagePositionPatient
			"00200013", // InstanceNumber
			"00080018", // SOPInstanceUID
			NULL
	};

	dicom::dicomfile_sorter dfsorter(study_keys, study_desc_keys,
			ser_keys, ser_desc_keys, inst_keys);

	while (1) {
		std::getline(fin, l);
		if (fin.eof()) break;
		dicom::dicomfile *df = dicom::open_dicomfile(l.c_str());
		dfsorter.add(df, l.c_str());
		close_dicomfile(df);
	}

	fin.close();


	printf("NUMBER OF STUDIES = %d\n", (int)dfsorter.number_of_studies());

	const char *study_desc, *series_desc, *fn;
	while (study_desc = dfsorter.get_next_study()) {
		printf("[%s]\n", study_desc);
		printf("\t%d series\n", dfsorter.number_of_series_in_curr_study());
		while (series_desc = dfsorter.get_next_series()) {
			printf("\t[%s]\n", series_desc);
			printf("\t\t%d images\n", dfsorter.number_of_images_in_curr_series());
			while (fn = dfsorter.get_next_instance()) {
				//printf("\t\t%s\n", fn);
			}
		}
	}

	return 0;
}

/*
 *
 *
 *
 */
//
//#include <map>
//#include <list>
//#include <string>
//#include <dicom.h>
//#include "errormsg.h"
//
//using namespace dicom;
//
//int main()
//{
//	try {
//		//dicomdir dr("..\\..\\..\\sample\\DICOMDIR");
//		dicomdir dr;
//		int ret;
//		ret = dr.add_dicomfile("..\\..\\..\\sample\\img001.dcm", "sample\\img001.dcm");
//		printf("RET = %d\n", ret);
//		ret = dr.add_dicomfile("..\\..\\..\\sample\\24bpp1.dcm", "sample\\24bpp1.dcm");
//		printf("RET = %d\n", ret);
//		ret =  dr.add_dicomfile("..\\..\\..\\sample\\24bpp2.dcm", "sample\\24bpp2.dcm");
//		printf("RET = %d\n", ret);
//
//		dr.save_to_file("test.dcm");
//	} catch (char *err) {
//		puts(err);
//	}
//
//	char buf[128];
//	strcpy(buf, "123.4567890AB");
//
//	puts(gen_uid(NULL).c_str());
//	puts(gen_uid(NULL).c_str());
//	puts(gen_uid(NULL).c_str());
//	puts(gen_uid(NULL).c_str());
//	puts(gen_uid(NULL).c_str());
//
////	dicomdir dr;
////
////	dicomfile df("../../../sample/img001.dcm");
////	dr.add_dicomfile(&df, "img001.dcm");
//
//
////	dr.dump();
//
////	puts("--------------");
////	strlist_t *l;
////	l = get_tags_for_dicomdir("STdUDY");
////	if (l) {
////		strlist_t::iterator it;
////		for (it = l->begin(); it != l->end(); it++) {
////			printf(">> %s\n", *it);
////		}
////	}
////
////	puts("--------------");
////
////	reset_tags_for_dicomdir();
////
////	puts("--------------");
//
//}
