#include "dicom.h"
#include <string>

int main()
{
	dicom::dicomfile *df;
	df = dicom::open_dicomfile("img001.dcm");
	if (df) {
		// get some informations dicom file
		std::string studydesc =
				df->get_dataelement("StudyDescription")->to_string();
		// shorter form
		std::string studytime = (*df)["StudyDate"];

		printf("Study Description = %s\n", studydesc.c_str());
		printf("Study Time = %s\n", studytime.c_str());

		// get images' informations in DICOM file

		int width, height;
		int precision, signedness, ncomponents, bytes_per_pixel;
		int nframes;
		df->get_image_info(&width, &height,
			&precision, &signedness, &ncomponents, &bytes_per_pixel,
			&nframes);

		// get pixeldata

		char *pixeldata;
		int pixeldata_length;
		df->get_pixeldata_a(&pixeldata, &pixeldata_length);

		if (pixeldata) {
			// do something ...

			free(pixeldata);
		}

		delete df;
	} else {
		printf("Error message: %s\n", dicom::get_error_message());
	}
}
