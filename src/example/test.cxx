
#include <map>
#include <list>
#include <string>
#include <dicom.h>
#include "errormsg.h"

using namespace dicom;

int main()
{
	try {
		//dicomdir dr("..\\..\\..\\sample\\DICOMDIR");
		dicomdir dr;
		int ret;
		ret = dr.add_dicomfile("..\\..\\..\\sample\\img001.dcm", "sample\\img001.dcm");
		printf("RET = %d\n", ret);
		ret = dr.add_dicomfile("..\\..\\..\\sample\\24bpp1.dcm", "sample\\24bpp1.dcm");
		printf("RET = %d\n", ret);
		ret =  dr.add_dicomfile("..\\..\\..\\sample\\24bpp2.dcm", "sample\\24bpp2.dcm");
		printf("RET = %d\n", ret);

		dr.save_to_file("test.dcm");
	} catch (char *err) {
		puts(err);
	}

	char buf[128];
	strcpy(buf, "123.4567890AB");

	puts(gen_uid(NULL).c_str());
	puts(gen_uid(NULL).c_str());
	puts(gen_uid(NULL).c_str());
	puts(gen_uid(NULL).c_str());
	puts(gen_uid(NULL).c_str());

//	dicomdir dr;
//
//	dicomfile df("../../../sample/img001.dcm");
//	dr.add_dicomfile(&df, "img001.dcm");


//	dr.dump();

//	puts("--------------");
//	strlist_t *l;
//	l = get_tags_for_dicomdir("STdUDY");
//	if (l) {
//		strlist_t::iterator it;
//		for (it = l->begin(); it != l->end(); it++) {
//			printf(">> %s\n", *it);
//		}
//	}
//
//	puts("--------------");
//
//	reset_tags_for_dicomdir();
//
//	puts("--------------");

}
