'''
/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */
'''

import sys
import dicom

def example_01_longform(df):
	print "Example 01 Long Form"

	# Get a data element that holds study date (0008,0020).
	de = df.get_dataelement(0x00080020)

	# check existence of the data element.
	if de.is_valid():
		# retrieve value
	    value = de.to_string();
	else:
		# set value for 'non-exist' data element.
		value = "N/A"

	print "  Value of 0x00080020 = '{0}'".format(value)


def example_01_shortform(df):
	print "Example 01 Short Form"

	# to_string() will return "N/A" string if get_dataelement() returns
	#   'invalid' or 'non-exist' dataelement.
	value = df.get_dataelement(0x00080020).to_string("N/A")
	print "  Value of 0x00080020 = '{0}'".format(value)

	# get value 'non-exist' dataelement without default value
	value = df.get_dataelement(0x0008FFFF).to_string();
	print "  Value of 0x00080020 = '{0}'".format(value)

	# get value 'non-exist' dataelement with default value
	value = df.get_dataelement(0x0008FFFF).to_string("N/A");
	print "  Value of 0x00080020 = '{0}'".format(value)

# get a value in 'int' form
def example_02_get_int(df):
	print "Example 02 Get Integer Values"

	number_of_slices = df.get_dataelement(0x00540081).to_int(0)
	print "  Int Value of (0054,0081) = {0}".format(number_of_slices)

	# another equivalent form
	number_of_slices = df.get_dataelement(0x00540081).get_value(0)
	print "  Int Value of (0054,0081) = {0}".format(number_of_slices)

	# yet another equivalent form
	number_of_slices = df[0x00540081]
	print "  Int Value of (0054,0081) = {0}".format(number_of_slices)


# get a value in 'double' form
def example_03_get_double(df):
	print "Example 03 Get Double Values"

	slice_thickeness = df.get_dataelement(0x00180050).to_double(0.0)
	# or
	slice_thickeness = df.get_dataelement(0x00180050).get_value()
	# or
	slice_thickeness = df.get_dataelement(0x00180050).get_value(0.0)
	# or
	slice_thickeness = df[0x00180050]
	print "  Double Value of (0018,0050) = {0}".format(slice_thickeness)


# get multiple 'double' values
def example_04_get_double_values(df):
	print "Example 04 Get Double Values"

	image_position = df.get_dataelement("ImagePositionPatient").to_double_values()
	# or
	image_position = df.get_dataelement("ImagePositionPatient").get_value()
	# or
	image_position = df["ImagePositionPatient"]
	
	print "  Image Patient Position", image_position

# get string value
def example_05_get_string(df):
	print "Example 05 Get String Values"

	patient_name = df.get_dataelement("PatientName").to_string("N/A")
	# or
	patient_name = df.get_dataelement("PatientName").get_value("N/A")
	print "  Patient name = {0}".format(patient_name)

	patient_name = df.get_dataelement("PatientName").to_string()
	# or
	patient_name = df.get_dataelement("PatientName").get_value()
	# or
	patient_name = df["PatientName"]
	if patient_name:
		print "  Patient name = {0}".format(patient_name)
	else:
		print "  Patient name is not available"
	
# get binary dat 
def example_06_get_binary_data(df):
	print "Example 06 Get Binary Data"

	pixeldata = df.get_dataelement(0x7fe00010).raw_value()
	if pixeldata:
		print "    Length of pixel data is {0} bytes.".format(len(pixeldata))
	else:
		print "    Pixel data is not available.\n"

if __name__=="__main__":
	df = dicom.open_dicomfile("img001.dcm")
	if not df:
		print dicom.get_error_message()
		sys.exit(-1)		

	example_01_longform(df)
	example_01_shortform(df)
	example_02_get_int(df)
	example_03_get_double(df)
	example_04_get_double_values(df)
	example_05_get_string(df)
	example_06_get_binary_data(df)

	del df # if you need to destroy dicomfile object explicitly 
