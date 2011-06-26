/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dicom.h"


void example_01_longform(dicom::dicomfile *df)
{
	printf("Example 01 Long Form\n");

	dicom::dataelement *de;
	std::string value;

	// Get a data element that holds study date (0008,0020).
	de = df->get_dataelement(0x00080020);

	// check existence of the data element.
	if (de->is_valid()) {
	        // retrieve value
	        value = de->to_string();
	} else {
	        // set value for 'non-exist' data element.
	        value = std::string("N/A");
	}

	printf("  Value of 0x00080020 = '%s'\n", value.c_str());

}

void example_01_shortform(dicom::dicomfile *df)
{
	printf("Example 01 Short Form\n");

	std::string value;

	// to_string() will return "N/A" string if get_dataelement() returns
	//   'invalid' or 'non-exist' dataelement.
	value = df->get_dataelement(0x00080020)->to_string("N/A");
	printf("  Value of (0008,0020) = '%s'\n", value.c_str());

	// get value 'non-exist' dataelement without default value
	value = df->get_dataelement(0x0008FFFF)->to_string();
	printf("  Value of (0008,FFFF) = '%s'\n", value.c_str());

	// get value 'non-exist' dataelement with default value
	value = df->get_dataelement(0x0008FFFF)->to_string("N/A");
	printf("  Value of (0008,FFFF) = '%s'\n", value.c_str());
}

/* get a value in 'int' form */
void example_02_get_int(dicom::dicomfile *df)
{
	printf("Example 02 Get Integer Values\n");

	int number_of_slices;

	number_of_slices = df->get_dataelement(0x00540081)->to_int(0);
	printf("  Int Value of (0054,0081) = %d\n", number_of_slices);

	// another equivalent form
	number_of_slices = (*df)[0x00540081].to_int(0);
	printf("  Int Value of (0054,0081) = %d\n", number_of_slices);

	// yet another equivalent form
	number_of_slices = (*df)[0x00540081];
	printf("  Int Value of (0054,0081) = %d\n", number_of_slices);
}

/* get a value in 'double' form */
void example_03_get_double(dicom::dicomfile *df)
{
	printf("Example 03 Get Double Values\n");

	double slice_thickeness;

	slice_thickeness = df->get_dataelement(0x00180050)->to_double(0.0);
	printf("  Double Value of (0018,0050) = %lf\n", slice_thickeness);

	slice_thickeness = (*df)[0x00180050].to_double(0.0);
	printf("  Double Value of (0018,0050) = %lf\n", slice_thickeness);

	slice_thickeness = (*df)[0x00180050];
	printf("  Double Value of (0018,0050) = %lf\n", slice_thickeness);
}


/* get multiple 'double' values */

void example_04_get_double_values(dicom::dicomfile *df)
{
	printf("Example 04 Get Double Values\n");

	double *image_position;
	int n;
	(*df)["ImagePositionPatient"].to_double_values_a(&image_position, &n);
	if (image_position != NULL) {
			printf("  Image Patient Position");
			for (int i = 0; i < n; i++)
					printf(" %lf", image_position[i]);
			printf("\n");
			free(image_position); // user SHOULD free() memories for array!
	}
}


void example_04_get_double_values_simpler(dicom::dicomfile *df)
{
	// much simpler codes using std::vector
	std::vector<double> image_position;
	unsigned int i;

	image_position =df->get_dataelement("ImagePositionPatient")->to_double_values();
	// or
	image_position =(*df)["ImagePositionPatient"].to_double_values();
	// or
	image_position =(*df)["ImagePositionPatient"];

	if (image_position.size()) {
		printf("  Image Patient Position");
		for (i = 0; i < image_position.size(); i++)
				printf(" %lf", image_position[i]);
		printf("\n");
	}
}

/* get string value */
void example_05_get_string(dicom::dicomfile *df)
{
	printf("Example 05 Get String Values\n");

	char *patient_name;
	(*df)["PatientName"].to_string_a(&patient_name);
	if (patient_name) {
	        printf("  Patient name = %s\n", patient_name);
	        free(patient_name); // user SHOULD free() memories for array!
	} else {
	        printf("  Patient name is not available\n");
	}
}

/* get string value */
void example_05_get_string_simpler(dicom::dicomfile *df)
{
	// much simpler codes using std::string
	std::string patient_name;

	patient_name = df->get_dataelement("PatientName")->to_string("N/A");
	printf("  Patient name = %s\n", patient_name.c_str());
	// or
	patient_name = (*df)["PatientName"].to_string("N/A");
	printf("  Patient name = %s\n", patient_name.c_str());
	// or
	patient_name = (std::string)(*df)["PatientName"];
	printf("  Patient name = %s\n", patient_name.c_str());
}

/* get binary data */
void example_06_get_binary_data(dicom::dicomfile *df)
{
	printf("Example 06 Get Binary Data\n");

	char *pixeldata;
	int pixeldata_len;
	df->get_dataelement(0x7fe00010)->raw_value(&pixeldata, &pixeldata_len);

	if (pixeldata != NULL) {
		printf("    Length of pixel data is %d bytes.\n", pixeldata_len);

		// DO NOT free() returned pointer to value!!!
	} else {
		printf("    Pixel data is not available.\n");
	}
}


int main()
{
	dicom::dicomfile *df;

	df = dicom::open_dicomfile("img001.dcm");
	if (!df) {
	        printf("%s\n", dicom::get_error_message());
	    goto __dicom_open_error__; // process error
	}

	example_01_longform(df);
	example_01_shortform(df);
	example_02_get_int(df);
	example_03_get_double(df);
	example_04_get_double_values(df);
	example_04_get_double_values_simpler(df);
	example_05_get_string(df);
	example_05_get_string_simpler(df);
	example_06_get_binary_data(df);

	dicom::close_dicomfile(df);

__dicom_open_error__:
	return 0;
}
