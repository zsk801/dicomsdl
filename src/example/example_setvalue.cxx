#include "dicom.h"

int main()
{
	try {
		dicom::dicomfile df;
//		dicom::dataelement *e;

		df.get_dataelement("00100020")->set_value("PAT ID");
		df.get_dataelement("00100010")->set_value("PAT NAME");
		df.add_dataelement("00110111", dicom::VR_US)->set_value(123);
		df.add_dataelement("00110111", dicom::VR_US)->set_value(123);
		df.add_dataelement("00110111", dicom::VR_US)->set_value(123);
//		df.add_dataelement("00110111", dicom::VR_US)->set_value(123);
//
//		e = df.add_dataelement(0x00020002, dicom::VR_UI);
//		e->from_string(dicom::uid_to_uidvalue(dicom::UID_POSITRON_EMISSION_TOMOGRAPHY_IMAGE_STORAGE));
//		e = df.add_dataelement(0x00020010); // omit vr arg if tag can be found in dictionary
//		e->from_string(dicom::uid_to_uidvalue(dicom::UID_EXPLICIT_VR_LITTLE_ENDIAN));
//		e->from_string(dicom::uid_to_uidvalue(dicom::UID_EXPLICIT_VR_BIG_ENDIAN));

//		df.add_dataelement("00110111", dicom::VR_US)->set_value(123);
		int values[] = {16,2,77,29};
//		df.add_dataelement("00110012", dicom::VR_IS)->set_value(values, 4);
//		df.add_dataelement("00090010.0.00110012", dicom::VR_IS)->set_value(values, 4);
		//df.add_dataelement("00090010.0.00110012", dicom::VR_IS)->set_value(values, 4);
//		df.add_dataelement("00090010.0.00110012", dicom::VR_IS)->set_value(values, 4);
//		df.add_dataelement("00090010.0.00110012", dicom::VR_IS)->set_value(values, 4);
//		df.add_dataelement("00090010.1.00110111", dicom::VR_US)->set_value(123);
//		df.add_dataelement("00090010.1.00110111", dicom::VR_US)->set_value(123);
//		df.add_dataelement("00090010.1.00110111", dicom::VR_US)->set_value(123);
//		df.add_dataelement("00090010.1.00110111", dicom::VR_US)->set_value(123);
////
//		e = df.add_dataelement(0x00110014, dicom::VR_OB); // omit vr arg if tag can be found in dictionary
//		e->from_string(dicom::uid_to_uidvalue(dicom::UID_EXPLICIT_VR_LITTLE_ENDIAN));

////		de = dfo.add_element('00020010', dicom.VR_UI)
////		de.from_string(dicom.ts_uid_to_uidvalue(dicom.UID_EXPLICIT_VR_LITTLE_ENDIAN))
////		dfo.add_element('00090010.0.00100010', dicom.VR_US).set_value(123)
////		dfo.add_element('00090010.0.00100012', dicom.VR_US).set_value(123)
////		dfo.add_element('00090010.0.00100013', dicom.VR_US).set_value([123,123])
////		dfo.add_element('00090010.1.00100013', dicom.VR_US).set_value([0x12, 0x23])
////		dfo.add_element('00090010.1.00100013', dicom.VR_US).set_value([0x34])
////		dfo.add_element('00090011.1.00100013', dicom.VR_US).set_value([0x45])
////		dfo.add_element('000a0011', dicom.VR_IS).set_value([0x45])
//
//
//		puts(df.dump_string().c_str());
//
//		puts("--------------");
//
//		char *val; int len;
//		df.write(&val, &len, dicom::DICOMFILE_WRITE_WITHOUT_METAINFO);
//		{
//			FILE *fout = fopen("c:\\temp\\out.dcm", "wb");
//			fwrite(val, len, 1, fout);
//			fclose(fout);
//		}
//		free(val);

	} catch (char *err) {
		printf("%s\n", err);
	}

	return 0;
}
